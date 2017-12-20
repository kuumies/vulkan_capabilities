/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::Controller class
 * -------------------------------------------------------------------------- */
 
#include "vk_capabilities_controller.h"

/* -------------------------------------------------------------------------- */

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <future>

/* -------------------------------------------------------------------------- */

#include "vk/vk_buffer.h"
#include "vk/vk_command.h"
#include "vk/vk_descriptor_set.h"
#include "vk/vk_helper.h"
#include "vk/vk_instance.h"
#include "vk/vk_logical_device.h"
#include "vk/vk_mesh.h"
#include "vk/vk_pipeline.h"
#include "vk/vk_render_pass.h"
#include "vk/vk_renderer.h"
#include "vk/vk_shader_module.h"
#include "vk/vk_stringify.h"
#include "vk/vk_surface_properties.h"
#include "vk/vk_surface_widget.h"
#include "vk/vk_sync.h"
#include "vk/vk_swapchain.h"
#include "vk_capabilities_data_creator.h"
#include "vk_capabilities_main_window.h"
#include "common/scene.h"

namespace kuu
{
namespace vk_capabilities
{

/* -------------------------------------------------------------------------- */

struct Controller::Impl
{
    ~Impl()
    {
        if (renderer)
            renderer->destroy();
        surfaceWidget->destroySurface();
    }

    // Main window
    std::unique_ptr<MainWindow> mainWindow;

    // Vulkan objects
    std::shared_ptr<vk::Instance> instance;
    std::shared_ptr<vk::SurfaceWidget> surfaceWidget;
    std::vector<vk::SurfaceProperties> surfaceProperties;

    // Capabilities data created from vulkan objects.
    std::shared_ptr<Data> capabilitiesData;

    // Test data.
    std::shared_ptr<vk::Renderer> renderer;
    std::shared_ptr<Scene> scene;
};

/* -------------------------------------------------------------------------- */

Controller::Controller()
    : impl(std::make_shared<Impl>())
{}

/* -------------------------------------------------------------------------- */

void Controller::start()
{
    impl->mainWindow = std::unique_ptr<MainWindow>(new MainWindow());
    impl->mainWindow->setEnabled(false);
    impl->mainWindow->show();
    impl->mainWindow->showProgress();
    connect(impl->mainWindow.get(), &MainWindow::runDeviceTest,
            this, &Controller::runDeviceTest);

    std::future<void> vulkanInstanceTask = std::async([&]()
    {
        const vk::InstanceInfo instanceInfo;

        std::vector<std::string> extensions;
        if (instanceInfo.isExtensionSupported("VK_KHR_get_physical_device_properties2"))
            extensions.push_back("VK_KHR_get_physical_device_properties2");
        if (instanceInfo.isExtensionSupported("VK_KHR_display"))
            extensions.push_back("VK_KHR_display");
        if (instanceInfo.isExtensionSupported("VK_KHR_surface"))
        {
            extensions.push_back("VK_KHR_surface");
#ifdef _WIN32
            if (instanceInfo.isExtensionSupported("VK_KHR_win32_surface"))
                extensions.push_back("VK_KHR_win32_surface");
#endif
        }

        impl->instance = std::make_shared<vk::Instance>();
        impl->instance->setApplicationName("V-Caps");
        impl->instance->setEngineName("V-CAPS-ENGINE");
        impl->instance->setExtensionNames(extensions);
        impl->instance->setValidateEnabled(true);
        impl->instance->create();
    });

    while (vulkanInstanceTask.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    impl->surfaceWidget = std::make_shared<vk::SurfaceWidget>();
    impl->surfaceWidget->resize(720, 576);
    QApplication::processEvents();
    impl->surfaceWidget->setInstance(impl->instance->handle());
    impl->surfaceWidget->createSurface();

    connect(impl->surfaceWidget.get(), &vk::SurfaceWidget::interval,
            this, &Controller::onSurfaceInterval);

    connect(impl->surfaceWidget.get(), &vk::SurfaceWidget::resized,
            this, &Controller::onSurfaceResized);

    connect(impl->surfaceWidget.get(), &vk::SurfaceWidget::wheel,
            this, &Controller::onSurfaceWheel);

    connect(impl->surfaceWidget.get(), &vk::SurfaceWidget::mouseMove,
            this, &Controller::onSurfaceMouseMove);

    connect(impl->surfaceWidget.get(), &vk::SurfaceWidget::key,
            this, &Controller::onSurfaceKey);

    std::future<void> uiDataTask = std::async([&]()
    {
        auto devices = impl->instance->physicalDevices();
        for (vk::PhysicalDevice& device : devices)
        {
            impl->surfaceProperties.push_back(
                vk::SurfaceProperties(
                    impl->instance->handle(),
                    device.handle(),
                    impl->surfaceWidget->handle()));
        }
        impl->capabilitiesData = DataCreator(*impl->instance, *impl->surfaceWidget, impl->surfaceProperties).data();
        impl->mainWindow->setDataAsync(impl->capabilitiesData);
    });

    while (uiDataTask.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);


    impl->mainWindow->update();
    impl->mainWindow->hideProgress();
    if (impl->capabilitiesData->hasVulkan &&
        impl->capabilitiesData->physicalDeviceData.size())
    {
        impl->mainWindow->setEnabled(true);
    }
}

/* -------------------------------------------------------------------------- */

void Controller::runDeviceTest(int deviceIndex)
{
    VkInstance instance             = impl->instance->handle();
    VkPhysicalDevice physicalDevice = impl->instance->physicalDevice(deviceIndex).handle();
    VkSurfaceKHR surface            = impl->surfaceWidget->handle();

    vk::SurfaceWidget* w = impl->surfaceWidget.get();
    w->startTimer(16, Qt::PreciseTimer);
    w->setWindowTitle("Device test");
    w->showMaximized();

    if (impl->renderer)
        return;

    VkExtent2D widgetExtent;
    widgetExtent.width  = uint32_t(w->width());
    widgetExtent.height = uint32_t(w->height());

    std::vector<std::string> maps =
    {
        "textures/blocksrough_ambientocclusion.png",
        "textures/blocksrough_basecolor.png",
        "textures/blocksrough_height.png",
        "textures/blocksrough_metallic.png",
        "textures/blocksrough_normal.png",
        "textures/blocksrough_roughness.png"
    };

    impl->scene = std::make_shared<Scene>();
    impl->scene->name = "pbr_maps_scene";
    //impl->scene->light.dir = glm::vec4(1.0, -0.5, 1.0, 0.0);
    impl->scene->light.dir =
        glm::normalize(glm::vec4(0.0,  0.0,  0.0, 0.0) -
                       glm::vec4(0.0, 10.0, 10.0, 0.0));

    float quadRadius = 1.0f;
#if 0
    float mapOffset = quadRadius * 2.0f;
    std::vector<glm::vec3> positions =
    {
        glm::vec3(-mapOffset * 1.5-0.1f, -2.0 * quadRadius, 2.0f),
        glm::vec3(-mapOffset * 0.5-0.1f, -2.0 * quadRadius, 2.0f),
        glm::vec3(-mapOffset * 1.5-0.1f, -0.0 * quadRadius, 2.0f),
        glm::vec3(-mapOffset * 0.5-0.1f, -0.0 * quadRadius, 2.0f),
        glm::vec3(-mapOffset * 1.5-0.1f,  2.0 * quadRadius, 2.0f),
        glm::vec3(-mapOffset * 0.5-0.1f,  2.0 * quadRadius, 2.0f)
    };

    std::vector<Vertex> quadVertices =
    {
        {  { quadRadius, -quadRadius, 0.0f}, { 1.0, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  { quadRadius,  quadRadius, 0.0f}, { 1.0, 1.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  {-quadRadius,  quadRadius, 0.0f}, { 0.0, 1.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  {-quadRadius, -quadRadius, 0.0f}, { 0.0, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } }
    };

    std::vector<unsigned int> quadIndices =
    {
        0, 1, 2,
        3, 0, 2
    };

    for (int i = 0; i < 6; ++i)
    {
        Model quad;
        quad.material->diffuse.map = maps[i];
        quad.mesh->vertices = quadVertices;
        quad.mesh->indices = quadIndices;
        quad.worldTransform = glm::translate(glm::mat4(1.0f), positions[i]);

        impl->scene->models.push_back(quad);
    }
#endif
    float sphereRadius = 2.0f;
    std::vector<Vertex> sphereVertices;
    std::vector<unsigned int> sphereIndices;

    int vertexCount = 0;

    auto addVertex = [&](const Vertex& v)
    {
        sphereVertices.push_back(v);
        sphereIndices.push_back(vertexCount);
        vertexCount++;
    };

    auto addTriangle = [&](
            const Vertex& a,
            const Vertex& b,
            const Vertex& c)
    {
        addVertex(a);
        addVertex(b);
        addVertex(c);
    };

    auto addQuad = [&](
            const Vertex& a,
            const Vertex& b,
            const Vertex& c,
            const Vertex& d)
    {
        addTriangle(a, d, c);
        addTriangle(c, b, a);
    };

    auto genSphere = [&](float radius, int ringCount, int sectorCount)
    {
        float ringStep   = 1.0f / float(ringCount   - 1);
        float sectorStep = 1.0f / float(sectorCount - 1);

        std::vector<Vertex> vertices;
        for(int r = 0; r < ringCount; ++r)
            for( int s = 0; s < sectorCount; ++s)
            {
                float y = sin(-M_PI_2 + M_PI * r * ringStep);
                float x = cos(2 * M_PI * s * sectorStep) * sin(M_PI * r * ringStep);
                float z = sin(2 * M_PI * s * sectorStep) * sin(M_PI * r * ringStep);

                glm::vec3 p = glm::vec3(x, y, z) * radius;
                glm::vec2 tc = glm::vec2(s * sectorStep, r * ringStep);
                glm::vec3 n = glm::normalize(p - glm::vec3(0.0f));
                glm::vec3 t;
                glm::vec3 bt;

                vertices.push_back(
                { p, tc, n, t, bt });
            }

        for(int r = 0; r < ringCount - 1; r++)
        {
            for(int s = 0; s < sectorCount - 1; s++)
            {
                int ia = (r+0) * sectorCount + (s+0);
                int ib = (r+0) * sectorCount + (s+1);
                int ic = (r+1) * sectorCount + (s+1);
                int id = (r+1) * sectorCount + (s+0);

                addQuad(vertices[id],
                        vertices[ic],
                        vertices[ib],
                        vertices[ia]);
            }
        }
    };

//    std::vector<std::string> maps2 =
//    {
//        "",
//        "textures/rustediron-streaks_basecolor.png",
//        "",
//        "textures/rustediron-streaks_metallic.png",
//        "textures/rustediron-streaks_normal.png",
//        "textures/rustediron-streaks_roughness.png"
//    };
    std::vector<std::string> maps2 =
    {
        "textures/cratered-rock-ao.png",
        "textures/cratered-rock-albedo.png",
        "textures/cratered-rock-height.png",
        "textures/cratered-rock-metalness.png",
        "textures/cratered-rock-normal.png",
        "textures/cratered-rock-roughness.png"
    };

    genSphere(sphereRadius, 32, 32);

    std::shared_ptr<Model> pbrSphere = std::make_shared<Model>();
    pbrSphere->material->type = Material::Type::Pbr;
    pbrSphere->material->pbr.ambientOcclusionMap = maps2[0];
    pbrSphere->material->pbr.baseColorMap        = maps2[1];
    pbrSphere->material->pbr.heightMap           = maps2[2];
    pbrSphere->material->pbr.metallicMap         = maps2[3];
    pbrSphere->material->pbr.normalMap           = maps2[4];
    pbrSphere->material->pbr.roughnessMap        = maps2[5];

    quadRadius = 3.0f;
    pbrSphere->mesh->vertices =
    {
        {  { quadRadius, -quadRadius, 0.0f}, { 1.0, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  { quadRadius,  quadRadius, 0.0f}, { 1.0, 1.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  {-quadRadius,  quadRadius, 0.0f}, { 0.0, 1.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  {-quadRadius, -quadRadius, 0.0f}, { 0.0, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } }
    };

    pbrSphere->mesh->vertices = sphereVertices;
    pbrSphere->mesh->indices  = sphereIndices;
    pbrSphere->mesh->generateTangents();
    pbrSphere->worldTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, -2.0f, 0.0f));

    impl->scene->models.push_back(pbrSphere);

    std::vector<std::string> maps3 =
    {
        "",
        "textures/rustediron2_basecolor.png",
        "",
        "textures/rustediron2_metallic.png",
        "textures/rustediron2_normal.png",
        "textures/rustediron2_roughness.png"
    };

    std::shared_ptr<Model> pbrSphere2 = std::make_shared<Model>();
    pbrSphere2->material->pbr.ambientOcclusionMap = maps3[0];
    pbrSphere2->material->pbr.baseColorMap        = maps3[1];
    pbrSphere2->material->pbr.heightMap           = maps3[2];
    pbrSphere2->material->pbr.metallicMap         = maps3[3];
    pbrSphere2->material->pbr.normalMap           = maps3[4];
    pbrSphere2->material->pbr.roughnessMap        = maps3[5];
    pbrSphere2->mesh = pbrSphere->mesh;
    pbrSphere2->worldTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -2.0f, 0.0f));
    impl->scene->models.push_back(pbrSphere2);

    std::vector<std::string> maps4 =
    {
        "textures/bamboo-wood-semigloss-ao.png",
        "textures/bamboo-wood-semigloss-albedo.png",
        "",
        "textures/bamboo-wood-semigloss-metal.png",
        "textures/bamboo-wood-semigloss-normal.png",
        "textures/bamboo-wood-semigloss-roughness.png"
    };


    std::shared_ptr<Model> pbrSphere3 = std::make_shared<Model>();
    pbrSphere3->material->pbr = Material::Pbr();
    pbrSphere3->material->pbr.ambientOcclusionMap = maps4[0];
    pbrSphere3->material->pbr.baseColorMap        = maps4[1];
    pbrSphere3->material->pbr.heightMap           = maps4[2];
    pbrSphere3->material->pbr.metallicMap         = maps4[3];
    pbrSphere3->material->pbr.normalMap           = maps4[4];
    pbrSphere3->material->pbr.roughnessMap        = maps4[5];
    pbrSphere3->mesh = pbrSphere->mesh;
    pbrSphere3->worldTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 2.0f, 0.0f));
    impl->scene->models.push_back(pbrSphere3);

    std::vector<std::string> maps5 =
    {
        "textures/oakfloor_AO.png",
        "textures/oakfloor_basecolor.png",
        "textures/oakfloor_Height.png",
        "",
        "textures/oakfloor_normal.png",
        "textures/oakfloor_roughness.png"
    };

    std::shared_ptr<Model> pbrSphere4 = std::make_shared<Model>();
    pbrSphere4->material->pbr.ambientOcclusionMap = maps5[0];
    pbrSphere4->material->pbr.baseColorMap        = maps5[1];
    pbrSphere4->material->pbr.heightMap           = maps5[2];
    pbrSphere4->material->pbr.metallicMap         = maps5[3];
    pbrSphere4->material->pbr.normalMap           = maps5[4];
    pbrSphere4->material->pbr.roughnessMap        = maps5[5];
    pbrSphere4->mesh = pbrSphere->mesh;
    pbrSphere4->worldTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 0.0f));
    impl->scene->models.push_back(pbrSphere4);

    std::shared_ptr<Model> pbrSphere5 = std::make_shared<Model>();
    pbrSphere5->material->pbr = Material::Pbr();
    pbrSphere5->material->pbr.ao = 1.0f;
    pbrSphere5->material->pbr.metallic = 0.8f;
    pbrSphere5->material->pbr.roughness = 0.2f;
    pbrSphere5->material->pbr.albedo = glm::vec3(1.0);
    pbrSphere5->mesh = pbrSphere->mesh;
    pbrSphere5->worldTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 6.0f, 0.0f));
    impl->scene->models.push_back(pbrSphere5);

    std::shared_ptr<Model> pbrSphere6 = std::make_shared<Model>();
    pbrSphere6->material->pbr = Material::Pbr();
    pbrSphere6->material->pbr.ao = 1.0f;
    pbrSphere6->material->pbr.metallic = 0.0f;
    pbrSphere6->material->pbr.roughness = 0.8f;
    pbrSphere6->material->pbr.albedo = glm::vec3(0.0f, 0.0f, 1.0f);
    pbrSphere6->mesh = pbrSphere->mesh;
    pbrSphere6->worldTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 6.0f, 0.0f));
    impl->scene->models.push_back(pbrSphere6);

#if 0
    Model pbrQuad;
    pbrQuad.material->type = Material::Type::Pbr;
    pbrQuad.material->pbr.ambientOcclusion = maps[0];
    pbrQuad.material->pbr.baseColor        = maps[1];
    pbrQuad.material->pbr.height           = maps[2];
    pbrQuad.material->pbr.metallic         = maps[3];
    pbrQuad.material->pbr.normal           = maps[4];
    pbrQuad.material->pbr.roughness        = maps[5];

    quadRadius = 3.0f;
    pbrQuad.mesh->vertices =
    {
        {  { quadRadius, -quadRadius, 0.0f}, { 1.0, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  { quadRadius,  quadRadius, 0.0f}, { 1.0, 1.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  {-quadRadius,  quadRadius, 0.0f}, { 0.0, 1.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  {-quadRadius, -quadRadius, 0.0f}, { 0.0, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } }
    };
    pbrQuad.mesh->indices  = quadIndices;
    pbrQuad.mesh->generateTangents();
    pbrQuad.worldTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(3.1f, -3.0f, 2.0f)) *
        glm::mat4_cast(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
    impl->scene->models.push_back(pbrQuad);
#endif

#if 1
    //--------------------------------------------------------------------------
    // Box mesh in NDC space.

    float width  = 15.0f;
    float height = 1.0f;
    float depth  = 15.0f;
    float bw = width  / 2.0f;
    float bh = height / 2.0f;
    float bd = depth  / 2.0f;

    // Create the vertex list
    std::vector<Vertex> vertices =
    {
        // -------------------------------------------------------
        // Back
        { { -bw, -bh, -bd },  { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
        { {  bw,  bh, -bd },  { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
        { {  bw, -bh, -bd },  { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
        { {  bw,  bh, -bd },  { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
        { { -bw, -bh, -bd },  { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
        { { -bw,  bh, -bd },  { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },

        // -------------------------------------------------------
        // Front
        { { -bw, -bh,  bd },  { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        { {  bw, -bh,  bd },  { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        { {  bw,  bh,  bd },  { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
        { {  bw,  bh,  bd },  { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
        { { -bw,  bh,  bd },  { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
        { { -bw, -bh,  bd },  { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

        // -------------------------------------------------------
        // Left
        { { -bw,  bh,  bd },  { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
        { { -bw,  bh, -bd },  { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
        { { -bw, -bh, -bd },  { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
        { { -bw, -bh, -bd },  { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
        { { -bw, -bh,  bd },  { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
        { { -bw,  bh,  bd },  { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },

        // -------------------------------------------------------
        // Right
        { {  bw,  bh,  bd },  { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { {  bw, -bh, -bd },  { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
        { {  bw,  bh, -bd },  { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
        { {  bw, -bh, -bd },  { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
        { {  bw,  bh,  bd },  { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { {  bw, -bh,  bd },  { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },

        // -------------------------------------------------------
        // Bottom
        { { -bw, -bh, -bd },  { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
        { {  bw, -bh, -bd },  { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
        { {  bw, -bh,  bd },  { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
        { {  bw, -bh,  bd },  { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
        { { -bw, -bh,  bd },  { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
        { { -bw, -bh, -bd },  { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },

        // -------------------------------------------------------
        // Top
        { { -bw,  bh, -bd },  { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  bw,  bh,  bd },  { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  bw,  bh, -bd },  { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  bw,  bh,  bd },  { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { { -bw,  bh, -bd },  { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
        { { -bw,  bh,  bd },  { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    };

    std::shared_ptr<kuu::Mesh> m = std::make_shared<kuu::Mesh>();
    for (const Vertex& v : vertices)
        m->addVertex(v);
    m->generateTangents();

    std::vector<float> vertexVector;
    for (const Vertex& v : m->vertices)
    {
        vertexVector.push_back(v.pos.x);
        vertexVector.push_back(v.pos.y);
        vertexVector.push_back(v.pos.z);
        vertexVector.push_back(v.texCoord.x);
        vertexVector.push_back(v.texCoord.y);
    }

    std::shared_ptr<Model> pbrBox = std::make_shared<Model>();
    pbrBox->material->type = Material::Type::Pbr;
    pbrBox->material->pbr.ambientOcclusionMap = maps[0];
    pbrBox->material->pbr.baseColorMap        = maps[1];
    pbrBox->material->pbr.heightMap           = maps[2];
    pbrBox->material->pbr.metallicMap         = maps[3];
    pbrBox->material->pbr.normalMap           = maps[4];
    pbrBox->material->pbr.roughnessMap        = maps[5];
    pbrBox->mesh = m;
    pbrBox->worldTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.2f, 0.0f));
    impl->scene->models.push_back(pbrBox);

#endif

    VkExtent2D extent;
    extent.width  = uint32_t(impl->surfaceWidget->width());
    extent.height = uint32_t(impl->surfaceWidget->height());
    const float aspect = extent.width / float(extent.height);
    impl->scene->viewport = glm::vec4(0, 0, width, height);
    impl->scene->camera.aspectRatio = aspect;
    impl->scene->camera.farPlane = 50.0f;
    impl->scene->camera.pos.y = 1.5f;
    impl->scene->camera.tPos.y = 1.5f;
    impl->scene->camera.pos.z += 4.0f;
    impl->scene->camera.tPos.z += 4.0f;

    impl->renderer = std::make_shared<vk::Renderer>(instance, physicalDevice, surface, widgetExtent, impl->scene);
    impl->renderer->create();
}

void Controller::onSurfaceInterval()
{
    if (impl->scene)
        impl->scene->camera.update();
    if (impl->renderer && impl->renderer->isValid())
        impl->renderer->renderFrame();
}

void Controller::onSurfaceResized()
{
    VkExtent2D extent;
    extent.width  = uint32_t(impl->surfaceWidget->width());
    extent.height = uint32_t(impl->surfaceWidget->height());

    if (impl->scene)
    {
        impl->scene->viewport = glm::vec4(0, 0, extent.width, extent.height);
        const float aspect = extent.width / float(extent.height);
        impl->scene->camera.aspectRatio = aspect;
    }

    if (impl->renderer)
        impl->renderer->resized(extent);
}

void Controller::onSurfaceWheel(int delta)
{
    const float amount = 1.0f;
    if (impl->scene)
    {
        glm::vec3 moveDir;
        moveDir.z = (delta > 0 ? -amount : amount);
        moveDir = impl->scene->camera.rotation() * moveDir;

        impl->scene->camera.tPos += moveDir;
    }
}

void Controller::onSurfaceMouseMove(const QPoint& offset, int buttons, int modifiers)
{
    if (impl->scene)
    {
        if (buttons & Qt::LeftButton)
        {
            const float scale = 0.01f;

            glm::vec3 moveDir;
            moveDir.x -= float(offset.x()) * scale;
            moveDir.y += float(offset.y()) * scale;

            moveDir = impl->scene->camera.rotation() * moveDir;
            impl->scene->camera.tPos += moveDir;
        }
        if (buttons & Qt::RightButton)
        {
            if (modifiers & Qt::ControlModifier)
            {
                impl->scene->camera.tRoll *= glm::angleAxis(-glm::radians(float(offset.x()) * 0.1f), glm::vec3(0.0f, 0.0f, 1.0f));
            }
            else
            {
                impl->scene->camera.tYaw   *= glm::angleAxis(-glm::radians(float(offset.x()) * 0.1f), glm::vec3(0.0f, 1.0f, 0.0f));
                impl->scene->camera.tPitch *= glm::angleAxis(-glm::radians(float(offset.y()) * 0.1f), glm::vec3(1.0f, 0.0f, 0.0f));
            }
        }
    }
}

void Controller::onSurfaceKey(int key, int modifiers, bool down)
{
    if (down)
    {
        float amount = 0.5f;
        glm::vec3 moveDir;
        switch(key)
        {
            case Qt::Key_W: moveDir.z = -amount; break;
            case Qt::Key_S: moveDir.z =  amount; break;
            case Qt::Key_A: moveDir.x = -amount; break;
            case Qt::Key_D: moveDir.x =  amount; break;
        }

        moveDir = impl->scene->camera.rotation() * moveDir;
        impl->scene->camera.move = moveDir;
    }
    else
    {
        impl->scene->camera.move = glm::vec3();
    }
}

} // namespace vk_capabilities
} // namespace kuu
