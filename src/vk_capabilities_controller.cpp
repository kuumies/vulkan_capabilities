/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::Controller class
 * -------------------------------------------------------------------------- */
 
#include "vk_capabilities_controller.h"

/* -------------------------------------------------------------------------- */

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
    w->show();

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

    float quadRadius = 1.0f;
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

    impl->scene = std::make_shared<Scene>();
    impl->scene->name = "pbr_maps_scene";
    for (int i = 0; i < 6; ++i)
    {
        Model quad;
        quad.material.diffuse.map = maps[i];
        quad.mesh.vertices = quadVertices;
        quad.mesh.indices = quadIndices;
        quad.worldTransform = glm::translate(glm::mat4(1.0f), positions[i]);

        impl->scene->models.push_back(quad);
    }

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

    genSphere(sphereRadius, 32, 32);

    Model pbrSphere;
    pbrSphere.material.type = Material::Type::Pbr;
    pbrSphere.material.pbr.ambientOcclusion = maps[0];
    pbrSphere.material.pbr.baseColor        = maps[1];
    pbrSphere.material.pbr.height           = maps[2];
    pbrSphere.material.pbr.metallic         = maps[3];
    pbrSphere.material.pbr.normal           = maps[4];
    pbrSphere.material.pbr.roughness        = maps[5];

    quadRadius = 3.0f;
    pbrSphere.mesh.vertices =
    {
        {  { quadRadius, -quadRadius, 0.0f}, { 1.0, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  { quadRadius,  quadRadius, 0.0f}, { 1.0, 1.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  {-quadRadius,  quadRadius, 0.0f}, { 0.0, 1.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  {-quadRadius, -quadRadius, 0.0f}, { 0.0, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } }
    };
    pbrSphere.mesh.indices  = quadIndices;
    pbrSphere.mesh.vertices = sphereVertices;
    pbrSphere.mesh.indices  = sphereIndices;

    pbrSphere.mesh.generateTangents();
    pbrSphere.worldTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(3.1f, 0.0f, 2.0f));
    impl->scene->models.push_back(pbrSphere);

#if 0
    Model pbrQuad;
    pbrQuad.material.type = Material::Type::Pbr;
    pbrQuad.material.pbr.ambientOcclusion = maps[0];
    pbrQuad.material.pbr.baseColor        = maps[1];
    pbrQuad.material.pbr.height           = maps[2];
    pbrQuad.material.pbr.metallic         = maps[3];
    pbrQuad.material.pbr.normal           = maps[4];
    pbrQuad.material.pbr.roughness        = maps[5];

    quadRadius = 3.0f;
    pbrQuad.mesh.vertices =
    {
        {  { quadRadius, -quadRadius, 0.0f}, { 1.0, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  { quadRadius,  quadRadius, 0.0f}, { 1.0, 1.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  {-quadRadius,  quadRadius, 0.0f}, { 0.0, 1.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } },
        {  {-quadRadius, -quadRadius, 0.0f}, { 0.0, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} , { 0.0f, 0.0f, 0.0f } }
    };
    pbrQuad.mesh.indices  = quadIndices;
    pbrQuad.mesh.generateTangents();
    pbrQuad.worldTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(3.1f, -3.0f, 2.0f)) *
        glm::mat4_cast(glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
    impl->scene->models.push_back(pbrQuad);
#endif

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
    if (impl->renderer)
        impl->renderer->resized(extent);
}

void Controller::onSurfaceWheel(int delta)
{
    const float amount = 1.0f;
    if (impl->scene)
        impl->scene->camera.tPos.z += (delta > 0 ? -amount : amount);
}

void Controller::onSurfaceMouseMove(const QPoint& offset, int buttons, int modifiers)
{
    if (impl->scene)
    {
        if (buttons & Qt::LeftButton)
        {
            const float scale = 0.01f;
            impl->scene->camera.tPos.x -= float(offset.x()) * scale;
            impl->scene->camera.tPos.y += float(offset.y()) * scale;
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
        glm::vec3 moveDir;
        switch(key)
        {
            case Qt::Key_W: moveDir.z = -1.0f; break;
            case Qt::Key_S: moveDir.z =  1.0f; break;
            case Qt::Key_A: moveDir.x = -1.0f; break;
            case Qt::Key_D: moveDir.x =  1.0f; break;
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
