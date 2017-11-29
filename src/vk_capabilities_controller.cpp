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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <future>

/* -------------------------------------------------------------------------- */

#include "vk/vk_buffer.h"
#include "vk/vk_helper.h"
#include "vk/vk_instance.h"
#include "vk/vk_mesh.h"
#include "vk/vk_pipeline.h"
#include "vk/vk_render_pass.h"
#include "vk/vk_shader_module.h"
#include "vk/vk_surface_properties.h"
#include "vk/vk_surface_widget.h"
#include "vk/vk_swapchain.h"
#include "vk/vk_descriptor_set.h"
#include "vk_capabilities_data_creator.h"
#include "vk_capabilities_main_window.h"

namespace kuu
{
namespace vk_capabilities
{

/* -------------------------------------------------------------------------- */

struct Controller::Impl
{
    // Main window
    std::unique_ptr<MainWindow> mainWindow;

    // Vulkan objects
    std::shared_ptr<vk::Instance> instance;
    std::unique_ptr<vk::SurfaceWidget> surfaceWidget;
    std::vector<vk::SurfaceProperties> surfaceProperties;

    // Capabilities data created from vulkan objects.
    std::shared_ptr<Data> capabilitiesData;

    // Test data.
    std::shared_ptr<vk::RenderPass> renderPass;
    std::shared_ptr<vk::Swapchain> swapchain;
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

    impl->surfaceWidget = std::unique_ptr<vk::SurfaceWidget>(new vk::SurfaceWidget(impl->instance->handle()));
    impl->surfaceWidget->resize(720, 576);

    std::future<void> uiDataTask = std::async([&]()
    {
        auto devices = impl->instance->physicalDevices();
        for (vk::PhysicalDevice& device : devices)
        {
            impl->surfaceProperties.push_back(
                vk::SurfaceProperties(
                    impl->instance->handle(),
                    device.physicalDeviceHandle(),
                    impl->surfaceWidget->surface()));
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
    vk::PhysicalDevice physicalDevice =
        impl->instance->physicalDevice(deviceIndex);

    const vk::PhysicalDeviceInfo info = physicalDevice.info();

    const int graphics     = vk::helper::findQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, info.queueFamilies);
    const int presentation = vk::helper::findPresentationQueueFamilyIndex(
        physicalDevice.physicalDeviceHandle(),
        impl->surfaceWidget->surface(),
        info.queueFamilies,
        { graphics });

    physicalDevice.setExtensions( { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    physicalDevice.addQueueFamily(graphics,     1, 1.0f);
    physicalDevice.addQueueFamily(presentation, 1, 1.0f);
    physicalDevice.create();
    if (!physicalDevice.isValid())
        return;

    VkPhysicalDevice pd = physicalDevice.physicalDeviceHandle();
    VkDevice ld         = physicalDevice.logicalDeviceHandle();

    const VkExtent2D widgetExtent = { uint32_t(impl->surfaceWidget->width()), uint32_t(impl->surfaceWidget->height()) };
    const vk::SurfaceProperties surfaceInfo = impl->surfaceProperties[deviceIndex];
    const VkSurfaceFormatKHR surfaceFormat = vk::helper::findSwapchainSurfaceFormat(surfaceInfo.surfaceFormats);
    const VkPresentModeKHR presentMode     = vk::helper::findSwapchainPresentMode(surfaceInfo.presentModes);
    const VkExtent2D extent                = vk::helper::findSwapchainImageExtent(surfaceInfo.surfaceCapabilities, widgetExtent);
    const int imageCount                   = vk::helper::findSwapchainImageCount(surfaceInfo.surfaceCapabilities);

    VkAttachmentDescription colorAttachment;
    colorAttachment.flags          = 0;                                 // No aliases
    colorAttachment.format         = surfaceFormat.format;              // Surface format
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;             // No multisampling
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;       // Clear the content on load
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;      // Keep the content stored-
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // We do not care about stencil
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // We do not care about stencil
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;         // We do not care as content is going to be cleared
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;   // Attachment is going to be presented to surface

    VkAttachmentDescription depthStencilAttachment;
    depthStencilAttachment.flags          = 0;                                 // No aliases
    depthStencilAttachment.format         = VK_FORMAT_D32_SFLOAT_S8_UINT;      // Surface format
    depthStencilAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;             // No multisampling
    depthStencilAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;       // Clear the content on load
    depthStencilAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;      // Keep the content stored-
    depthStencilAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // We do not care about stencil
    depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // We do not care about stencil
    depthStencilAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;         // We do not care as content is going to be cleared
    depthStencilAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Attachment is depth/stencil

    VkAttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;                                        // Reference to first
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Used as color attachment

    VkAttachmentReference depthStencilAttachmentRef;
    depthStencilAttachmentRef.attachment = 1;                                                // Reference to second attachment
    depthStencilAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Used as depth/stencil attachment

    VkSubpassDescription subpass;
    subpass.flags                   = 0;                               // No flags
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS; // Graphics pipeline
    subpass.inputAttachmentCount    = 0;                               // No input attachments
    subpass.pInputAttachments       = NULL;
    subpass.colorAttachmentCount    = 1;                               // Color attachment
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pResolveAttachments     = NULL;                            // No multisampling
    subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;      // Depth/stencil attachment
    subpass.preserveAttachmentCount = 0;                               // This subpass is not a passtrought
    subpass.pPreserveAttachments    = NULL;

    // Attachment usage dependency. Only this subpass is using the attachment...
//    srcSubpass – Index of a first (previous) subpass or VK_SUBPASS_EXTERNAL if we want to indicate dependency between subpass and operations outside of a render pass.
//    dstSubpass – Index of a second (later) subpass (or VK_SUBPASS_EXTERNAL).
//    srcStageMask – Pipeline stage during which a given attachment was used before (in a src subpass).
//    dstStageMask – Pipeline stage during which a given attachment will be used later (in a dst subpass).
//    srcAccessMask – Types of memory operations that occurred in a src subpass or before a render pass.
//    dstAccessMask – Types of memory operations that occurred in a dst subpass or after a render pass.
//    dependencyFlags – Flag describing the type (region) of dependency.

    VkSubpassDependency dependency;
    dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;                           // Implicit subpass before
    dependency.dstSubpass      = 0;                                             // This subpass
    dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask   = 0;
    dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    impl->renderPass = std::make_shared<vk::RenderPass>(physicalDevice.logicalDeviceHandle());
    impl->renderPass->setAttachmentDescriptions( { colorAttachment, depthStencilAttachment } );
    impl->renderPass->setSubpassDescriptions( { subpass } );
    impl->renderPass->setSubpassDependencies( { dependency } );
    impl->renderPass->create();
    if (!impl->renderPass->isValid())
        return;

    impl->swapchain = std::make_shared<vk::Swapchain>(impl->surfaceWidget->surface(),
                                                      physicalDevice.physicalDeviceHandle(),
                                                      physicalDevice.logicalDeviceHandle(),
                                                      impl->renderPass->handle());
    impl->swapchain->setSurfaceFormat(surfaceFormat)
                    .setPresentMode(presentMode)
                    .setImageExtent(extent)
                    .setImageCount(imageCount)
                    .setPreTransform(surfaceInfo.surfaceCapabilities.currentTransform)
                    .setQueueIndicies( { uint32_t(graphics), uint32_t(presentation) } )
                    .setCreateDepthStencilBuffer(true);
    impl->swapchain->create();
    if (!impl->swapchain->isValid())
        return;

    vk::ShaderModule vshModule(ld, "shaders/test.vert.spv");
    vshModule.setStageName("main");
    vshModule.setStage(VK_SHADER_STAGE_VERTEX_BIT);
    vshModule.create();
    if (!vshModule.isValid())
        return;

    vk::ShaderModule fshModule(ld, "shaders/test.frag.spv");
    fshModule.setStageName("main");
    fshModule.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
    fshModule.create();
    if (!fshModule.isValid())
        return;


    const std::vector<float> vertices =
    {
         0.0f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f
    };

    const std::vector<uint32_t> indices =
    {
        0, 1, 2
    };

    vk::Mesh mesh(pd, ld);
    mesh.setVertices(vertices);
    mesh.setIndices(indices);
    mesh.addVertexAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
    mesh.addVertexAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, 3 * sizeof(float));
    mesh.setVertexBindingDescription(0, 6 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX);
    mesh.create();
    if (!mesh.isValid())
        return;

    struct Matrices
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    } matrices;
    const float aspect = extent.width / float(extent.height);
    matrices.view       = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
    matrices.projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 150.0f);
    matrices.projection[1][1] *= -1;

    vk::Buffer uniformBuffer(pd, ld);
    uniformBuffer.setSize(sizeof(matrices));
    uniformBuffer.setUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uniformBuffer.setMemoryProperties(
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniformBuffer.create();
    if (!uniformBuffer.isValid())
        return;

    void* uniformDst = uniformBuffer.map();
    memcpy(uniformDst, &matrices, size_t(uniformBuffer.size()));
    uniformBuffer.unmap();

    vk::DescriptorPool descriptorPool(ld);
    descriptorPool.addTypeSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    descriptorPool.create();
    if (!descriptorPool.isValid())
        return;

    vk::DescriptorSets descriptorSet(ld, descriptorPool.handle());
    descriptorSet.addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    descriptorSet.create();
    if (!descriptorSet.isValid())
        return;
    descriptorSet.writeUniformBuffer( { { uniformBuffer.handle(), 0, uniformBuffer.size() } } );

    vk::Pipeline pipeline(ld);
    pipeline.addShaderStage(vshModule.createInfo());
    pipeline.addShaderStage(fshModule.createInfo());
    pipeline.setVertexInputState(
        { mesh.vertexBindingDescription() },
        mesh.vertexAttributeDescriptions() );
    pipeline.setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
}

} // namespace vk_capabilities
} // namespace kuu
