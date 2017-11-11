/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::RenderPass class
 * ---------------------------------------------------------------- */

#include "vk_render_pass.h"

/* ---------------------------------------------------------------- */

#include <iostream>

/* ---------------------------------------------------------------- */

#include "vk_surface.h"
#include "vk_device.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct RenderPass::Data
{
    Data(const Device& device,
         const Surface& surface,
         const Parameters& params)
        : device(device)
        , params(params)
    {
        /* ------------------------------------------------------------ *
           Render pass

            * framebuffer attachments
            * framebuffer content handling

         * ------------------------------------------------------------ */

        // Create the description of the colorbuffer attachment. The
        // colorbuffer is an image in the swap chain.
        //      * no multisampling
        //      * clear the content before rendering
        //      * keep the contetn after rendering
        //      * do not care stencil operations
        //      * do not create what is the pixel layout before rendering
        //        as it is going to be cleared
        //      * set pixel layout to be presentation format after
        //        rendering as its going to be display on the screen
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format         = surface.format().format;
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Attachment descriptions
        const VkAttachmentDescription attachmentDescriptions[] =
        { colorAttachment };

        // Graphics attachment reference for subpass which is the color
        // attachment above. Use the optimal layout for color attachment
        // as thats what it is.
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Subpass that renders into colorbuffer attachment.
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments    = &colorAttachmentRef;

        // Add a subpass dependecy to prevent the image layout transition
        // being run too early. Make render pass to wait by waiting for
        // the color attachment output stage.
        VkSubpassDependency dependency = {};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // Fill the render pass info.
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments    = attachmentDescriptions;
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        // Create the render pass.
        VkResult result = vkCreateRenderPass(
            device.handle(),
            &renderPassInfo,
            nullptr,
            &renderPass);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create vulkan render pass"
                      << std::endl;

            return;
        }

        /* ------------------------------------------------------------ *
           Framebuffers of swap chain images for render pass.
         * ------------------------------------------------------------ */

        // Get the swap chain image views.
        std::vector<VkImageView> swapChainImageViews = params.swapChain.imageViews();

        // Each swap chain image view has its own framebuffer.
        swapChainFramebuffers.resize(swapChainImageViews.size());

        // Create the framebuffers
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            const VkImageView attachments[] = { swapChainImageViews[i] };

            // Create the framebuffer info.
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments    = attachments;
            framebufferInfo.width           = params.viewportWidth;
            framebufferInfo.height          = params.viewportHeight;
            framebufferInfo.layers          = 1;

            // Create the framebuffer.
            result = vkCreateFramebuffer(
                device.handle(),
                &framebufferInfo,
                nullptr,
                &swapChainFramebuffers[i]);
            if (result != VK_SUCCESS)
            {
                std::cerr << __FUNCTION__
                          << ": failed to create vulkan framebuffer"
                          << std::endl;

                return;
            }
        }

        valid = true;
    }

    ~Data()
    {
        for (uint32_t i = 0; i < swapChainFramebuffers.size(); i++)
            vkDestroyFramebuffer(device.handle(), swapChainFramebuffers[i], nullptr);
        vkDestroyRenderPass(device.handle(), renderPass, nullptr);
    }

    Device device;
    VkRenderPass renderPass;
    bool valid = false;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    const Parameters params;
};

/* ---------------------------------------------------------------- */

RenderPass::RenderPass(const Device& device,
                       const Surface& surface,
                       const Parameters& params)
    : d(std::make_shared<Data>(device, surface, params))
{}

/* ---------------------------------------------------------------- */

bool RenderPass::isValid() const
{ return d->valid; }

/* ---------------------------------------------------------------- */

VkRenderPass RenderPass::handle() const
{ return d->renderPass; }

void RenderPass::begin(int i, VkCommandBuffer buffer, VkClearValue clearColor)
{
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = d->renderPass;
    renderPassInfo.framebuffer       = d->swapChainFramebuffers[i];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = VkExtent2D { d->params.viewportWidth, d->params.viewportHeight };
    renderPassInfo.clearValueCount   = 1;
    renderPassInfo.pClearValues      = &clearColor;

    // Begin the render pass.
    vkCmdBeginRenderPass(
        buffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end(VkCommandBuffer buffer)
{
    vkCmdEndRenderPass(buffer);
}

} // namespace vk
} // namespace kuu
