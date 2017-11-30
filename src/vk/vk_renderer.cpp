/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Renderer class.
 * -------------------------------------------------------------------------- */

#include "vk_renderer.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "vk_buffer.h"
#include "vk_command.h"
#include "vk_descriptor_set.h"
#include "vk_helper.h"
#include "vk_logical_device.h"
#include "vk_mesh.h"
#include "vk_pipeline.h"
#include "vk_render_pass.h"
#include "vk_shader_module.h"
#include "vk_stringify.h"
#include "vk_surface_properties.h"
#include "vk_sync.h"
#include "vk_swapchain.h"

namespace kuu
{
namespace vk
{

namespace
{

std::vector<VkQueueFamilyProperties> getQueueFamilies(
    const VkPhysicalDevice& physicalDevice)
{
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,            // [in]  physical device handle
        &queueFamilyPropertyCount, // [out] queue family property count
        NULL);                     // [in]  properties, NULL to get count

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,                // [in]  physical device handle
        &queueFamilyPropertyCount,     // [in]  queue family property count
        queueFamilyProperties.data()); // [out] queue family properties

    return queueFamilyProperties;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct Matrices
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

/* -------------------------------------------------------------------------- */

struct Renderer::Impl
{
    Impl(const VkInstance& instance,
         const VkPhysicalDevice& physicalDevice,
         const VkSurfaceKHR& surface,
         const VkExtent2D& extent)
        : instance(instance)
        , physicalDevice(physicalDevice)
        , surface(surface)
        , surfaceInfo(instance, physicalDevice, surface)
        , extent(extent)
    {}

    ~Impl()
    {
        destroy();
    }

    bool create()
    {
        if (!setupSurface())
            return false;

        if (!createLogicalDevice())
            return false;

        if (!createRenderPass())
            return false;

        if (!createSwapchain())
            return false;

        if (!createShaders())
            return false;

        if (!createMesh())
            return false;

        if (!createUniformBuffer())
            return false;

        if (!createDescriptorSets())
            return false;

        if (!createPipeline())
            return false;

        if (!createCommandBuffers())
            return false;

        if (!createSync())
            return false;

        return true;
    }

    bool setupSurface()
    {
        surfaceFormat = vk::helper::findSwapchainSurfaceFormat(surfaceInfo.surfaceFormats);
        if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
            return false;

        presentMode = vk::helper::findSwapchainPresentMode(surfaceInfo.presentModes);
        extent      = vk::helper::findSwapchainImageExtent(surfaceInfo.surfaceCapabilities, extent);
        return true;
    }

    bool createLogicalDevice()
    {
        // Get the queue family indices for graphics and presentation queues
        auto queueFamilies = getQueueFamilies(physicalDevice);
        const int graphics     = helper::findQueueFamilyIndex(
            VK_QUEUE_GRAPHICS_BIT,
            queueFamilies);

        if (graphics == -1)
            return false;

        const int presentation = helper::findPresentationQueueFamilyIndex(
            physicalDevice,
            surface,
            queueFamilies,
            { graphics });

        if (presentation == -1)
            return false;

        graphicsFamilyIndex     = graphics;
        presentationFamilyIndex = presentation;

        device = std::make_shared<LogicalDevice>(physicalDevice);
        device->setExtensions( { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
        device->addQueueFamily(graphicsFamilyIndex,     1, 1.0f);
        device->addQueueFamily(presentationFamilyIndex, 1, 1.0f);
        return device->create();
    }

    bool createRenderPass()
    {
        // Colorbuffer attachment description
        VkAttachmentDescription colorAttachment;
        colorAttachment.flags          = 0;
        colorAttachment.format         = surfaceFormat.format;
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Depth/stencil buffer attachment description
        VkAttachmentDescription depthStencilAttachment;
        depthStencilAttachment.flags          = 0;
        depthStencilAttachment.format         = VK_FORMAT_D32_SFLOAT_S8_UINT;
        depthStencilAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthStencilAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthStencilAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Reference to first attachment (color)
        VkAttachmentReference colorAttachmentRef;
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Reference to second attachment (depth & stencil)
        VkAttachmentReference depthStencilAttachmentRef;
        depthStencilAttachmentRef.attachment = 1;
        depthStencilAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Subpass
        VkSubpassDescription subpass;
        subpass.flags                   = 0;
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount    = 0;
        subpass.pInputAttachments       = NULL;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pResolveAttachments     = NULL;
        subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments    = NULL;

        // Subpass dependency
        VkSubpassDependency dependency;
        dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass      = 0;
        dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask   = 0;
        dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                     VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = 0;

        renderPass = std::make_shared<RenderPass>(device->handle());
        renderPass->addAttachmentDescription(colorAttachment);
        renderPass->addAttachmentDescription(depthStencilAttachment);
        renderPass->addSubpassDescription(subpass);
        renderPass->addSubpassDependency(dependency);
        return renderPass->create();

        // Attachment usage dependency. Only this subpass is using the attachment...
    //    srcSubpass – Index of a first (previous) subpass or VK_SUBPASS_EXTERNAL if we want to indicate dependency between subpass and operations outside of a render pass.
    //    dstSubpass – Index of a second (later) subpass (or VK_SUBPASS_EXTERNAL).
    //    srcStageMask – Pipeline stage during which a given attachment was used before (in a src subpass).
    //    dstStageMask – Pipeline stage during which a given attachment will be used later (in a dst subpass).
    //    srcAccessMask – Types of memory operations that occurred in a src subpass or before a render pass.
    //    dstAccessMask – Types of memory operations that occurred in a dst subpass or after a render pass.
    //    dependencyFlags – Flag describing the type (region) of dependency.

    }

    bool createSwapchain()
    {
        swapchainImageCount = vk::helper::findSwapchainImageCount(surfaceInfo.surfaceCapabilities);

        swapchain = std::make_shared<vk::Swapchain>(
            physicalDevice,
            device->handle(),
            surface,
            renderPass->handle());

        swapchain->setSurfaceFormat(surfaceFormat);
        swapchain->setPresentMode(presentMode);
        swapchain->setImageExtent(extent);
        swapchain->setImageCount(swapchainImageCount);
        swapchain->setPreTransform(surfaceInfo.surfaceCapabilities.currentTransform);
        swapchain->setQueueIndicies( { graphicsFamilyIndex, presentationFamilyIndex } );
        swapchain->setCreateDepthStencilBuffer(true);
        return swapchain->create();
    }

    bool createShaders()
    {
        vshModule = std::make_shared<ShaderModule>(device->handle(), "shaders/test.vert.spv");
        vshModule->setStageName("main");
        vshModule->setStage(VK_SHADER_STAGE_VERTEX_BIT);
        if (!vshModule->create())
            return false;

        fshModule = std::make_shared<ShaderModule>(device->handle(), "shaders/test.frag.spv");
        fshModule->setStageName("main");
        fshModule->setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
        if (!fshModule->create())
            return false;

        return true;
    }

    bool createMesh()
    {
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

        indexCount = uint32_t(indices.size());

        mesh = std::make_shared<Mesh>(physicalDevice, device->handle());
        mesh->setVertices(vertices);
        mesh->setIndices(indices);
        mesh->addVertexAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
        mesh->addVertexAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, 3 * sizeof(float));
        mesh->setVertexBindingDescription(0, 6 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX);
        return mesh->create();
    }

    bool createUniformBuffer()
    {
        const float aspect = extent.width / float(extent.height);
        matrices.model      = glm::mat4(1.0f);
        matrices.view       = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
        matrices.projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 150.0f);
        matrices.projection[1][1] *= -1;

        uniformBuffer = std::make_shared<Buffer>(physicalDevice, device->handle());
        uniformBuffer->setSize(sizeof(matrices));
        uniformBuffer->setUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        uniformBuffer->setMemoryProperties(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (!uniformBuffer->create())
            return false;

        void* uniformDst = uniformBuffer->map();
        memcpy(uniformDst, &matrices, size_t(uniformBuffer->size()));
        uniformBuffer->unmap();
        return true;
    }

    bool createDescriptorSets()
    {
        descriptorPool = std::make_shared<DescriptorPool>(device->handle());
        descriptorPool->addTypeSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
        if (!descriptorPool->create())
            return false;

        descriptorSets = std::make_shared<DescriptorSets>(device->handle(), descriptorPool->handle());
        descriptorSets->addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        if (!descriptorSets->create())
            return false;
        descriptorSets->writeUniformBuffer( { { uniformBuffer->handle(), 0, uniformBuffer->size() } } );
        return true;
    }

    bool createPipeline()
    {
        VkPipelineColorBlendAttachmentState colorBlend = {};
        colorBlend.blendEnable    = VK_FALSE;
        colorBlend.colorWriteMask = 0xf;

        float blendConstants[4] = { 0, 0, 0, 0 };

        pipeline = std::make_shared<Pipeline>(device->handle());
        pipeline->addShaderStage(vshModule->createInfo());
        pipeline->addShaderStage(fshModule->createInfo());
        pipeline->setVertexInputState(
            { mesh->vertexBindingDescription() },
              mesh->vertexAttributeDescriptions() );
        pipeline->setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
        pipeline->setViewportState(
            { { 0, 0, float(extent.width), float(extent.height), 0, 1 } },
            { { { 0, 0 }, extent }  } );
        pipeline->setRasterizerState(
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_NONE,
            VK_FRONT_FACE_COUNTER_CLOCKWISE);
        pipeline->setMultisampleState(VK_FALSE, VK_SAMPLE_COUNT_1_BIT);
        pipeline->setDepthStencilState(VK_TRUE);
        pipeline->setColorBlendingState(
                VK_FALSE,
                VK_LOGIC_OP_CLEAR,
                { colorBlend },
                blendConstants);

        const std::vector<VkDescriptorSetLayout> descriptorSetLayouts =
        { descriptorSets->layoutHandle() };
        const std::vector<VkPushConstantRange> pushConstantRanges;
        pipeline->setPipelineLayout(descriptorSetLayouts, pushConstantRanges);
        pipeline->setDynamicState( { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } );
        pipeline->setRenderPass(renderPass->handle());
        return pipeline->create();
    }

    bool createCommandBuffers()
    {
        commandPool = std::make_shared<CommandPool>(device->handle());
        commandPool->setQueueFamilyIndex(graphicsFamilyIndex);
        if (!commandPool->create())
            return false;

        commandBuffers =
            commandPool->allocate(
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                swapchainImageCount);

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
            renderPassInfo.renderPass      = renderPass->handle();
            renderPassInfo.framebuffer     = swapchain->framebuffer(uint32_t(i));
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

            VkDescriptorSet descriptorHandle = descriptorSets->handle();
            vkCmdBindDescriptorSets(
                commandBuffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline->pipelineLayoutHandle(), 0, 1,
                &descriptorHandle, 0, NULL);

            vkCmdBindPipeline(
                commandBuffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline->handle());

            const VkBuffer vertexBuffer = mesh->vertexBufferHandle();
            const VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(
                commandBuffers[i], 0, 1,
                &vertexBuffer,
                offsets);

            const VkBuffer indexBuffer = mesh->indexBufferHandle();
            vkCmdBindIndexBuffer(
                commandBuffers[i],
                indexBuffer,
                0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(
                commandBuffers[i],
                indexCount,
                1, 0, 0, 1);

            vkCmdEndRenderPass(commandBuffers[i]);

            const VkResult result = vkEndCommandBuffer(commandBuffers[i]);
            if (result != VK_SUCCESS)
            {
                std::cerr << __FUNCTION__
                          << ": render commands failed as "
                          << vk::stringify::resultDesc(result)
                          << std::endl;
                return false;
            }
        }

        return true;
    }

    bool createSync()
    {
        renderingFinished = std::make_shared<Semaphore>(device->handle());
        if (!renderingFinished->create())
            return false;

        imageAvailable = std::make_shared<Semaphore>(device->handle());
        if (!imageAvailable->create())
            return false;

        return true;
    }

    bool renderFrame()
    {
        const float aspect = extent.width / float(extent.height);
        matrices.view       = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
        matrices.projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 150.0f);
        matrices.projection[1][1] *= -1;
        q *= glm::angleAxis(glm::radians(0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        matrices.model = glm::mat4_cast(q);

        void* uniformDst = uniformBuffer->map();
        memcpy(uniformDst, &matrices, size_t(uniformBuffer->size()));
        uniformBuffer->unmap();

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            device->handle(),
            swapchain->handle(),
            std::numeric_limits<uint64_t>::max(),
            imageAvailable->handle(),
            VK_NULL_HANDLE,
            &imageIndex);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": next image acquire from swapchain failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;
            return false;
        }

        VkSemaphore waitSemaphores[]   = { imageAvailable->handle() };
        VkSemaphore signalSemaphores[] = { renderingFinished->handle() };
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
        vkGetDeviceQueue(device->handle(), graphicsFamilyIndex, 0, &graphicsQueue);

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
            return false;
        }

        VkSwapchainKHR swapChains[] = { swapchain->handle() };

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
        vkGetDeviceQueue(device->handle(), presentationFamilyIndex, 0, &presentQueue);
        vkQueuePresentKHR(presentQueue, &presentInfo);

        return true;
    }

    bool resized(const VkExtent2D& extent)
    {
        vkDeviceWaitIdle(device->handle());
        this->extent = extent;

        commandPool.reset();
        commandBuffers.clear();
        pipeline.reset();
        renderPass.reset();
        swapchain.reset();

        if (!createRenderPass())
            return false;
        if (!createSwapchain())
            return false;
        if (!createPipeline())
            return false;
        if (!createCommandBuffers())
            return false;

        return true;
    }

    void destroy()
    {
        pipeline->destroy();
        descriptorPool->destroy();
        descriptorSets->destroy();
        uniformBuffer->destroy();
        mesh->destroy();
        fshModule->destroy();
        vshModule->destroy();
        swapchain->destroy();
        renderPass->destroy();
        device->destroy();
    }

    // From user.
    VkInstance instance             = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR surface            = VK_NULL_HANDLE;

    // Surface
    SurfaceProperties surfaceInfo;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;

    // Queue family indices.
    uint32_t graphicsFamilyIndex;
    uint32_t presentationFamilyIndex;

    // Device.
    std::shared_ptr<LogicalDevice> device;

    // Render pass.
    std::shared_ptr<RenderPass> renderPass;

    // Swapchain
    std::shared_ptr<Swapchain> swapchain;
    uint32_t swapchainImageCount;

    // Shaders
    std::shared_ptr<ShaderModule> vshModule;
    std::shared_ptr<ShaderModule> fshModule;

    // Mesh
    std::shared_ptr<Mesh> mesh;
    uint32_t indexCount;

    // Uniform buffer
    std::shared_ptr<Buffer> uniformBuffer;

    // Descriptor sets
    std::shared_ptr<DescriptorPool> descriptorPool;
    std::shared_ptr<DescriptorSets> descriptorSets;

    // Pipeline
    std::shared_ptr<Pipeline> pipeline;

    // Commands
    std::shared_ptr<CommandPool> commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    // Sync
    std::shared_ptr<Semaphore> renderingFinished;
    std::shared_ptr<Semaphore> imageAvailable;

    // Runtime
    Matrices matrices;
    glm::quat q;
};

/* -------------------------------------------------------------------------- */

Renderer::Renderer(const VkInstance& instance,
                   const VkPhysicalDevice& physicalDevice,
                   const VkSurfaceKHR& surface,
                   const VkExtent2D& extent)
    : impl(std::make_shared<Impl>(instance, physicalDevice, surface, extent))
{}

bool Renderer::create()
{
    if (!isValid())
        return impl->create();
    return isValid();
}

void Renderer::destroy()
{
    if (isValid())
        impl->destroy();
}

bool Renderer::isValid() const
{ return impl->commandBuffers.size() > 0; }

bool Renderer::resized(const VkExtent2D& extent)
{ return impl->resized(extent); }

bool Renderer::renderFrame()
{
    return impl->renderFrame();
}


} // namespace vk
} // namespace kuu
