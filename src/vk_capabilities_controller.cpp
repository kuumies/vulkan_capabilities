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

    impl->surfaceWidget = std::unique_ptr<vk::SurfaceWidget>(new vk::SurfaceWidget());
    impl->surfaceWidget->resize(720, 576);
    QApplication::processEvents();
    impl->surfaceWidget->setInstance(impl->instance->handle());
    impl->surfaceWidget->createSurface();

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
    impl->surfaceWidget->show();

    vk::PhysicalDevice physicalDevice =
        impl->instance->physicalDevice(deviceIndex);

    VkInstance i = impl->instance->handle();
    VkPhysicalDevice pd = physicalDevice.handle();
    VkSurfaceKHR surface = impl->surfaceWidget->handle();

    const VkExtent2D widgetExtent = { uint32_t(impl->surfaceWidget->width()), uint32_t(impl->surfaceWidget->height()) };
    vk::Renderer renderer(i, pd, surface, widgetExtent);
    if (!renderer.create())
        return;

    for (;;)
        renderer.renderFrame();

    const vk::PhysicalDeviceInfo info = physicalDevice.info();

    const int graphics     = vk::helper::findQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, info.queueFamilies);
    const int presentation = vk::helper::findPresentationQueueFamilyIndex(
        pd,
        surface,
        info.queueFamilies,
        { graphics });

    vk::LogicalDevice logicalDevice(physicalDevice.handle());
    logicalDevice.setExtensions( { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    logicalDevice.addQueueFamily(graphics,     1, 1.0f);
    logicalDevice.addQueueFamily(presentation, 1, 1.0f);
    logicalDevice.create();
    if (!logicalDevice.isValid())
        return;

    //VkPhysicalDevice pd = physicalDevice.handle();
    VkDevice ld         = logicalDevice.handle();

    const vk::SurfaceProperties surfaceInfo = impl->surfaceProperties[deviceIndex];
    const VkSurfaceFormatKHR surfaceFormat = vk::helper::findSwapchainSurfaceFormat(surfaceInfo.surfaceFormats);
    const VkPresentModeKHR presentMode     = vk::helper::findSwapchainPresentMode(surfaceInfo.presentModes);
    //const VkExtent2D extent                = vk::helper::findSwapchainImageExtent(surfaceInfo.surfaceCapabilities, widgetExtent);
    VkExtent2D extent = widgetExtent;
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

    impl->renderPass = std::make_shared<vk::RenderPass>(ld);
    impl->renderPass->addAttachmentDescription(colorAttachment);
    impl->renderPass->addAttachmentDescription(depthStencilAttachment);
    impl->renderPass->addSubpassDescription(subpass);
    impl->renderPass->addSubpassDependency(dependency);
    impl->renderPass->create();
    if (!impl->renderPass->isValid())
        return;

    impl->swapchain = std::make_shared<vk::Swapchain>(pd,
                                                      ld,
                                                      impl->surfaceWidget->handle(),
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

    const float size = 2.0f;
    const std::vector<float> vertices =
    {
         size, -size,  0.0f, 1.0f, 0.0f, 0.0f,
         size,  size,  0.0f, 0.0f, 1.0f, 0.0f,
        -size,  size,  0.0f, 0.0f, 0.0f, 1.0f,
        -size, -size,  0.0f, 1.0f, 1.0f, 1.0f,
    };

    const std::vector<uint32_t> indices =
    {
        0, 1, 2,
        3, 0, 2
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
    matrices.model      = glm::mat4(1.0f);
    matrices.view       = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
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

    VkPipelineColorBlendAttachmentState colorBlend = {};
    colorBlend.blendEnable    = VK_FALSE;
    colorBlend.colorWriteMask = 0xf;

    float blendConstants[4] = { 0, 0, 0, 0 };

    vk::Pipeline pipeline(ld);
    pipeline.addShaderStage(vshModule.createInfo());
    pipeline.addShaderStage(fshModule.createInfo());
    pipeline.setVertexInputState(
        { mesh.vertexBindingDescription() },
        mesh.vertexAttributeDescriptions() );
    pipeline.setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
    pipeline.setViewportState(
        { { 0, 0, float(extent.width), float(extent.height), 0, 1 } },
        { { { 0, 0 }, extent }  } );
    pipeline.setRasterizerState(
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pipeline.setMultisampleState(VK_FALSE, VK_SAMPLE_COUNT_1_BIT);
    pipeline.setDepthStencilState(VK_TRUE);
    pipeline.setColorBlendingState(
            VK_FALSE,
            VK_LOGIC_OP_CLEAR,
            { colorBlend },
            blendConstants);

    const std::vector<VkDescriptorSetLayout> descriptorSetLayouts =
    { descriptorSet.layoutHandle() };
    const std::vector<VkPushConstantRange> pushConstantRanges;
    pipeline.setPipelineLayout(descriptorSetLayouts, pushConstantRanges);
    pipeline.setDynamicState( { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } );
    pipeline.setRenderPass(impl->renderPass->handle());
    pipeline.create();
    if (!pipeline.isValid())
        return;

    vk::CommandPool commandPool(ld);
    commandPool.setQueueFamilyIndex(graphics);
    commandPool.create();
    if (!commandPool.isValid())
        return;

    std::vector<VkCommandBuffer> commandBuffers = commandPool.allocate(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        imageCount);

    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext            = NULL;
        beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = NULL;

        vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color        = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo;
        renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext           = NULL;
        renderPassInfo.renderPass      = impl->renderPass->handle();
        renderPassInfo.framebuffer     = impl->swapchain->framebuffer(uint32_t(i));
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = uint32_t(clearValues.size());
        renderPassInfo.pClearValues    = clearValues.data();

        vkCmdBeginRenderPass(
            commandBuffers[i],
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport;
        viewport.x        = 0;
        viewport.y        = 0;
        viewport.width    = float(extent.width);
        viewport.height   = float(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

        VkRect2D scissor;
        scissor.offset = {};
        scissor.extent = extent;
        vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

        VkDescriptorSet descriptorHandle = descriptorSet.handle();
        vkCmdBindDescriptorSets(
            commandBuffers[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline.pipelineLayoutHandle(), 0, 1,
            &descriptorHandle, 0, NULL);

        vkCmdBindPipeline(
            commandBuffers[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline.handle());

        const VkBuffer vertexBuffer = mesh.vertexBufferHandle();
        const VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(
            commandBuffers[i], 0, 1,
            &vertexBuffer,
            offsets);

        const VkBuffer indexBuffer = mesh.indexBufferHandle();
        vkCmdBindIndexBuffer(
            commandBuffers[i],
            indexBuffer,
            0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(
            commandBuffers[i],
            uint32_t(indices.size()),
            1, 0, 0, 1);

        vkCmdEndRenderPass(commandBuffers[i]);

        const VkResult result = vkEndCommandBuffer(commandBuffers[i]);
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": render commands failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;
        }
    }

    vk::Semaphore renderingFinished(ld);
    renderingFinished.create();
    if (!renderingFinished.isValid())
        return;

    vk::Semaphore imageAvailable(ld);
    imageAvailable.create();
    if (!imageAvailable.isValid())
        return;

    glm::quat q;

    for (;;)
    {
        q *= glm::angleAxis(glm::radians(0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        matrices.model = glm::mat4_cast(q);

        void* uniformDst = uniformBuffer.map();
        memcpy(uniformDst, &matrices, size_t(uniformBuffer.size()));
        uniformBuffer.unmap();

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            ld,
            impl->swapchain->handle(),
            std::numeric_limits<uint64_t>::max(),
            imageAvailable.handle(),
            VK_NULL_HANDLE,
            &imageIndex);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": next image acquire from swapchain failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;
        }

        VkSemaphore waitSemaphores[]   = { imageAvailable.handle() };
        VkSemaphore signalSemaphores[] = { renderingFinished.handle() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo;
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = NULL;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        VkQueue graphicsQueue;
        vkGetDeviceQueue(ld, uint32_t(graphics), 0, &graphicsQueue);

        result =
            vkQueueSubmit(
                graphicsQueue,
                1,
                &submitInfo,
                VK_NULL_HANDLE);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": graphics queue submit failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;
        }

        VkSwapchainKHR swapChains[] = { impl->swapchain->handle() };

        VkPresentInfoKHR presentInfo;
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext              = NULL;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = swapChains;
        presentInfo.pImageIndices      = &imageIndex;
        presentInfo.pResults           = NULL;

        VkQueue presentQueue;
        vkGetDeviceQueue(ld, uint32_t(presentation), 0, &presentQueue);
        vkQueuePresentKHR(presentQueue, &presentInfo);
    }

    vkDeviceWaitIdle(ld);
}

} // namespace vk_capabilities
} // namespace kuu
