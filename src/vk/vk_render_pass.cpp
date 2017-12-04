/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::RenderPass class
 * -------------------------------------------------------------------------- */

#include "vk_render_pass.h"
#include <algorithm>
#include <iostream>
#include "vk_image.h"
#include "vk_stringify.h"

/* -------------------------------------------------------------------------- */

namespace kuu
{
namespace vk
{

struct RenderPass::Impl
{
    Impl(const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice)
        : logicalDevice(logicalDevice)
        , depthStencilImage(physicalDevice, logicalDevice)
    {}

    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {
        VkStructureType type = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        VkRenderPassCreateInfo info;
        info.sType           = type;
        info.pNext           = NULL;
        info.flags           = 0;
        info.attachmentCount = uint32_t(attachmentDescriptions.size());
        info.pAttachments    = attachmentDescriptions.data();
        info.subpassCount    = uint32_t(subpassDescriptions.size());
        info.pSubpasses      = subpassDescriptions.data();
        info.dependencyCount = uint32_t(subpassDependencies.size());
        info.pDependencies   = subpassDependencies.data();

        const VkResult result =
            vkCreateRenderPass(
                logicalDevice,  // [in]  logical device
                &info,          // [in]  create info
                NULL,           // [in]  allocator callback
                &renderPass);   // [out] render pass handle

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": render pass creation failed as "
                      << vk::stringify::result(result)
                      << std::endl;
            return false;
        }

        depthStencilImage.setType(VK_IMAGE_TYPE_2D);
        depthStencilImage.setFormat(VK_FORMAT_D32_SFLOAT_S8_UINT);
        depthStencilImage.setExtent( { imageExtent.width, imageExtent.height, 1 } );
        depthStencilImage.setTiling(VK_IMAGE_TILING_OPTIMAL);
        depthStencilImage.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        depthStencilImage.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
        depthStencilImage.setImageViewAspect(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
        depthStencilImage.setMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        depthStencilImage.create();
        if (!depthStencilImage.isValid())
            return false;

        // Create swapchain framebuffers
        size_t swapchainImageCount = swapchainImageViews.size();
        swapchainFramebuffers.resize(swapchainImageCount);
        for (size_t i = 0; i < swapchainImageCount; ++i)
        {
            std::vector<VkImageView> attachments =
            { swapchainImageViews[i],
              depthStencilImage.imageViewHandle() };

            VkStructureType type = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            VkFramebufferCreateInfo info;
            info.sType           = type;                         // Must be this type.
            info.pNext           = NULL;                         // Must be null
            info.flags           = 0;                            // Must be 0
            info.renderPass      = renderPass;                   // Render pass handle.
            info.attachmentCount = uint32_t(attachments.size()); // Attachment count
            info.pAttachments    = attachments.data();           // Attachments
            info.width           = imageExtent.width;            // Width of framebuffer
            info.height          = imageExtent.height;           // Height of framebuffer
            info.layers          = 1;                            // Single layer

            const VkResult result =
                vkCreateFramebuffer(
                    logicalDevice,
                    &info,
                    NULL,
                    &swapchainFramebuffers[i]);

            if (result != VK_SUCCESS)
            {
                std::cerr << __FUNCTION__
                          << ": swap chain framebuffer creation failed as "
                          << vk::stringify::result(result)
                          << std::endl;
                return false;
            }
        }

        return true;
    }

    void destroy()
    {

        for (size_t i = 0; i < swapchainFramebuffers.size(); i++)
            vkDestroyFramebuffer(
                logicalDevice,            // [in] logical device handle
                swapchainFramebuffers[i], // [in] framebuffer handle
                NULL);                    // [in] allocator
        swapchainFramebuffers.clear();

        vkDestroyRenderPass(
            logicalDevice, // [in] logical device
            renderPass,    // [in] render pass
            NULL);         // [in] allocator callback

        depthStencilImage.destroy();
        renderPass = VK_NULL_HANDLE;
    }

    bool isValid()
    {
        return renderPass != VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkRenderPass renderPass = VK_NULL_HANDLE;

    // From user
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    std::vector<VkSubpassDescription> subpassDescriptions;
    std::vector<VkSubpassDependency> subpassDependencies;
    VkExtent2D imageExtent;
    std::vector<VkImageView> swapchainImageViews;

    std::vector<VkFramebuffer> swapchainFramebuffers;
    Image depthStencilImage;
};

RenderPass::RenderPass(const VkPhysicalDevice& physicalDevice,
                       const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>(physicalDevice, logicalDevice))
{}

RenderPass& RenderPass::addAttachmentDescription(
    const VkAttachmentDescription& description)
{
    impl->attachmentDescriptions.push_back(description);
    return *this;
}

std::vector<VkAttachmentDescription> RenderPass::attachmentDescriptions() const
{ return impl->attachmentDescriptions; }

RenderPass& RenderPass::addSubpassDescription(
    const VkSubpassDescription& description)
{
    impl->subpassDescriptions.push_back(description);
    return *this;
}

std::vector<VkSubpassDescription> RenderPass::subpassDescriptions() const
{ return impl->subpassDescriptions; }

RenderPass& RenderPass::addSubpassDependency(
    const VkSubpassDependency& dependency)
{
    impl->subpassDependencies.push_back(dependency);
    return *this;
}

std::vector<VkSubpassDependency> RenderPass::subpassDependencies() const
{ return impl->subpassDependencies; }

RenderPass& RenderPass::setSwapchainImageViews(
    const std::vector<VkImageView>& imageViews,
    const VkExtent2D& imageExtent)
{
    impl->imageExtent = imageExtent;
    impl->swapchainImageViews = imageViews;
    return *this;
}

bool RenderPass::create()
{
    if (!isValid())
        return impl->create();
    return true;
}

void RenderPass::destroy()
{
    if (isValid())
        impl->destroy();
}

bool RenderPass::isValid() const
{ return impl->isValid(); }

VkRenderPass RenderPass::handle() const
{ return impl->renderPass; }

VkFramebuffer RenderPass::framebuffer(uint32_t index) const
{
    if (index >= impl->swapchainFramebuffers.size())
        return VK_NULL_HANDLE;
    return impl->swapchainFramebuffers[index];
}

} // namespace vk
} // namespace kuu
