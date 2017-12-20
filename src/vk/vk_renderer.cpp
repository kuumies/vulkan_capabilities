/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Renderer class.
 * -------------------------------------------------------------------------- */

#include "vk_renderer.h"
#include <iostream>
#include <QtGui/QImage>
#include "renderer/vk_atmoshere_renderer.h"
#include "renderer/vk_mesh_manager.h"
#include "renderer/vk_pbr_renderer.h"
#include "renderer/vk_shadow_map_depth.h"
#include "renderer/vk_shadow_map_renderer.h"
#include "renderer/vk_sky_renderer.h"
#include "vk_buffer.h"
#include "vk_command.h"
#include "vk_descriptor_set.h"
#include "vk_helper.h"
#include "vk_image.h"
#include "vk_logical_device.h"
#include "vk_mesh.h"
#include "vk_pipeline.h"
#include "vk_queue.h"
#include "vk_render_pass.h"
#include "vk_shader_module.h"
#include "vk_stringify.h"
#include "vk_surface_properties.h"
#include "vk_sync.h"
#include "vk_swapchain.h"
#include "vk_texture.h"
#include "../common/scene.h"
#include "../common/light.h"

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

struct Renderer::Impl
{
    Impl(const VkInstance& instance,
         const VkPhysicalDevice& physicalDevice,
         const VkSurfaceKHR& surface,
         const VkExtent2D& extent,
         const std::shared_ptr<Scene>& scene)
        : instance(instance)
        , physicalDevice(physicalDevice)
        , surface(surface)
        , surfaceInfo(instance, physicalDevice, surface)
        , extent(extent)
        , scene(scene)
    {}

    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {       
        if (!setupSurface())
            return false;

        if (!createLogicalDevice())
            return false;

        if (!createSwapchain())
            return false;

        if (!createCommandPool())
            return false;

        if (!createRenderPass())
            return false;

        if (!createSubRenderers())
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

        const int transfer = helper::findQueueFamilyIndex(
            VK_QUEUE_TRANSFER_BIT,
            queueFamilies,
            { presentation, graphics });

        if (presentation == -1)
            return false;

        graphicsFamilyIndex     = graphics;
        presentationFamilyIndex = presentation;
        transferFamilyIndex    = transfer;

        device = std::make_shared<LogicalDevice>(physicalDevice);
        device->setExtensions( { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
        device->addQueueFamily(graphicsFamilyIndex,     1, 1.0f);
        device->addQueueFamily(presentationFamilyIndex, 1, 1.0f);
        if (transferFamilyIndex >= 0)
            device->addQueueFamily(transferFamilyIndex, 1, 1.0f);
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

        renderPass = std::make_shared<RenderPass>(physicalDevice, device->handle());
        renderPass->addAttachmentDescription(colorAttachment);
        renderPass->addAttachmentDescription(depthStencilAttachment);
        renderPass->addSubpassDescription(subpass);
        renderPass->addSubpassDependency(dependency);
        renderPass->setSwapchainImageViews(swapchain->imageViews(), extent);
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

        swapchain = std::make_shared<vk::Swapchain>(device->handle(), surface);
        swapchain->setSurfaceFormat(surfaceFormat);
        swapchain->setPresentMode(presentMode);
        swapchain->setImageExtent(extent);
        swapchain->setImageCount(swapchainImageCount);
        swapchain->setPreTransform(surfaceInfo.surfaceCapabilities.currentTransform);
        swapchain->setQueueIndicies( { graphicsFamilyIndex, presentationFamilyIndex } );
        return swapchain->create();
    }

    bool createSubRenderers()
    {
        std::shared_ptr<MeshManager> meshManager =
            std::make_shared<MeshManager>(
                physicalDevice,
                device->handle(),
                graphicsFamilyIndex);

        for (std::shared_ptr<Model> m : scene->models)
            if (m->material->type == Material::Type::Pbr)
                meshManager->addPbrMesh(m->mesh);

        atmosphereRenderer = std::make_shared<AtmosphereRenderer>(
            physicalDevice,
            device->handle(),
            graphicsFamilyIndex);
        atmosphereRenderer->setLightDir(scene->light.dir);
        atmosphereRenderer->render();

        skyRenderer = std::make_shared<SkyRenderer>(
            physicalDevice,
            device->handle(),
            graphicsFamilyIndex,
            extent,
            renderPass->handle(),
            atmosphereRenderer->textureCube());
        skyRenderer->setScene(scene);

        shadowMapRenderer = std::make_shared<ShadowMapRenderer>(
                    physicalDevice,
                    device->handle(),
                    graphicsFamilyIndex,
                    meshManager);
        shadowMapRenderer->setScene(scene);

        shadowMapDepthRenderer = std::make_shared<ShadowMapDepth>(
                    physicalDevice,
                    device->handle(),
                    graphicsFamilyIndex,
                    renderPass->handle());
        shadowMapDepthRenderer->setShadowMap(shadowMapRenderer->texture());

        pbrRenderer = std::make_shared<PbrRenderer>(
            physicalDevice,
            device->handle(),
            graphicsFamilyIndex,
            extent,
            renderPass->handle(),
            atmosphereRenderer->textureCube(),
            meshManager);
        pbrRenderer->setShadowMap(shadowMapRenderer->texture());
        pbrRenderer->setScene(scene);

        return true;
    }

    bool createCommandPool()
    {
        graphicsCommandPool = std::make_shared<CommandPool>(device->handle());
        graphicsCommandPool->setQueueFamilyIndex(graphicsFamilyIndex);
        return graphicsCommandPool->create();
    }

    bool createCommandBuffers()
    {
        commandBuffers =
            graphicsCommandPool->allocateBuffers(
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
            clearValues[0].color        = { 0.1f, 0.1f, 0.1f, 1.0f };
            clearValues[1].depthStencil = { 1.0f, 0 };

            VkRenderPassBeginInfo renderPassInfo;
            renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.pNext             = NULL;
            renderPassInfo.renderPass        = renderPass->handle();
            renderPassInfo.framebuffer       = renderPass->framebuffer(uint32_t(i));
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = extent;
            renderPassInfo.clearValueCount   = uint32_t(clearValues.size());
            renderPassInfo.pClearValues      = clearValues.data();

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

            skyRenderer->recordCommands(commandBuffers[i]);
            pbrRenderer->recordCommands(commandBuffers[i]);
            //shadowMapDepthRenderer->recordCommands(commandBuffers[i]);

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
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence = VK_NULL_HANDLE;
        vkCreateFence(device->handle(),
                      &info,
                      NULL,
                      &fence);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            device->handle(),
            swapchain->handle(),
            std::numeric_limits<uint64_t>::max(),
            imageAvailable->handle(),
            fence,
            &imageIndex);

        vkWaitForFences(device->handle(), 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkDestroyFence(device->handle(), fence, NULL);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": next image acquire from swapchain failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;
            return false;
        }

        skyRenderer->updateUniformBuffers();
        pbrRenderer->updateUniformBuffers();

        shadowMapRenderer->render();

        // Render
        Queue graphicsQueue(device->handle(), graphicsFamilyIndex, 0);
        graphicsQueue.create();
        graphicsQueue.submit(commandBuffers[imageIndex],
                             renderingFinished->handle(),
                             imageAvailable->handle(),
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        // Present
        Queue presentQueue(device->handle(), presentationFamilyIndex, 0);
        presentQueue.create();
        presentQueue.present(swapchain->handle(),
                             renderingFinished->handle(),
                             imageIndex);

        return true;
    }

    bool resized(const VkExtent2D& extent)
    {
        vkDeviceWaitIdle(device->handle());
        this->extent = extent;

        graphicsCommandPool.reset();
        commandBuffers.clear();

        renderPass.reset();
        swapchain.reset();

        if (!createSwapchain())
            return false;
        if (!createRenderPass())
            return false;
        if (!createCommandPool())
            return false;

        pbrRenderer->resized(extent, renderPass->handle());
        skyRenderer->resized(extent, renderPass->handle());

        if (!createCommandBuffers())
            return false;

        return true;
    }

    void destroy()
    {
        if (device->handle() == VK_NULL_HANDLE)
            return;

        vkDeviceWaitIdle(device->handle());

        shadowMapRenderer.reset();
        shadowMapDepthRenderer.reset();
        skyRenderer.reset();
        pbrRenderer.reset();
        atmosphereRenderer.reset();

        graphicsCommandPool->destroy();
        commandBuffers.clear();
        renderingFinished->destroy();
        imageAvailable->destroy();

        swapchain->destroy();
        renderPass->destroy();
        device->destroy();
    }

    bool isValid() const
    {
        return commandBuffers.size() > 0                             &&
               graphicsCommandPool && graphicsCommandPool->isValid() &&
               renderingFinished   && renderingFinished->isValid()   &&
               imageAvailable      && imageAvailable->isValid()      &&
               swapchain           && swapchain->isValid()           &&
               renderPass          && renderPass->isValid()          &&
               device              && device->isValid();
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
    uint32_t transferFamilyIndex;
    uint32_t graphicsFamilyIndex;
    uint32_t presentationFamilyIndex;

    // Device.
    std::shared_ptr<LogicalDevice> device;

    // Render pass.
    std::shared_ptr<RenderPass> renderPass;

    // Swapchain
    std::shared_ptr<Swapchain> swapchain;
    uint32_t swapchainImageCount;

    // Commands
    std::shared_ptr<CommandPool> graphicsCommandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    // Sync
    std::shared_ptr<Semaphore> renderingFinished;
    std::shared_ptr<Semaphore> imageAvailable;

    // Scene
    const std::shared_ptr<Scene> scene;

    // Sub-renderers
    std::shared_ptr<AtmosphereRenderer> atmosphereRenderer;
    std::shared_ptr<PbrRenderer> pbrRenderer;
    std::shared_ptr<SkyRenderer> skyRenderer;
    std::shared_ptr<ShadowMapRenderer> shadowMapRenderer;
    std::shared_ptr<ShadowMapDepth> shadowMapDepthRenderer;
};

/* -------------------------------------------------------------------------- */

Renderer::Renderer(const VkInstance& instance,
                   const VkPhysicalDevice& physicalDevice,
                   const VkSurfaceKHR& surface,
                   const VkExtent2D& extent,
                   const std::shared_ptr<Scene>& scene)
    : impl(std::make_shared<Impl>(instance,
                                  physicalDevice,
                                  surface,
                                  extent,
                                  scene))
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
{ return impl->isValid(); }

bool Renderer::resized(const VkExtent2D& extent)
{ return impl->resized(extent); }

bool Renderer::renderFrame()
{ return impl->renderFrame(); }

} // namespace vk
} // namespace kuu
