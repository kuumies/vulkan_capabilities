/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::RenderPass class
 * -------------------------------------------------------------------------- */

#include "vk_render_pass.h"

/* -------------------------------------------------------------------------- */

#include <algorithm>
#include <iostream>

/* -------------------------------------------------------------------------- */

#include "vk_stringify.h"

/* -------------------------------------------------------------------------- */

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   Implementation of the render pass.
 * -------------------------------------------------------------------------- */
struct RenderPass::Impl
{
    /* ----------------------------------------------------------------------- *
        Destroys the render pass.
     * ----------------------------------------------------------------------- */
    ~Impl()
    {
        destroy();
    }

    /* ----------------------------------------------------------------------- *
        Creates the render pass.
     * ----------------------------------------------------------------------- */
    void create()
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
            return;
        }
    }

    /* ----------------------------------------------------------------------- *
        Destroys the render pass.
     * ----------------------------------------------------------------------- */
    void destroy()
    {
        vkDestroyRenderPass(
            logicalDevice, // [in] logical device
            renderPass,    // [in] render pass
            NULL);         // [in] allocator callback
    }

    VkDevice logicalDevice;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    std::vector<VkSubpassDescription> subpassDescriptions;
    std::vector<VkSubpassDependency> subpassDependencies;
};

/* -------------------------------------------------------------------------- *
   Constructs the render pass.
 * -------------------------------------------------------------------------- */
RenderPass::RenderPass(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
}

/* -------------------------------------------------------------------------- *
   Sets the attachment descriptions.
 * -------------------------------------------------------------------------- */
RenderPass& RenderPass::setAttachmentDescriptions(
    const std::vector<VkAttachmentDescription>& descriptions)
{
    impl->attachmentDescriptions =  descriptions;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the attachment descriptions.
 * -------------------------------------------------------------------------- */
std::vector<VkAttachmentDescription> RenderPass::attachmentDescriptions() const
{ return impl->attachmentDescriptions; }

/* -------------------------------------------------------------------------- *
   Sets the subpass descriptions.
 * -------------------------------------------------------------------------- */
RenderPass& RenderPass::setSubpassDescriptions(
    const std::vector<VkSubpassDescription>& descriptions)
{
    impl->subpassDescriptions = descriptions;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the subpass descriptions.
 * -------------------------------------------------------------------------- */
std::vector<VkSubpassDescription> RenderPass::subpassDescriptions() const
{ return impl->subpassDescriptions; }

/* -------------------------------------------------------------------------- *
   Sets the subpass dependencies;
 * -------------------------------------------------------------------------- */
RenderPass &RenderPass::setSubpassDependencies(
    const std::vector<VkSubpassDependency>& dependencies)
{
    impl->subpassDependencies = dependencies;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the subpass dependencies;
 * -------------------------------------------------------------------------- */
std::vector<VkSubpassDependency> RenderPass::subpassDependencies() const
{ return impl->subpassDependencies; }

/* -------------------------------------------------------------------------- *
   Creates the render pass.
 * -------------------------------------------------------------------------- */
void RenderPass::create()
{
    if (!isValid())
        impl->create();
}

/* -------------------------------------------------------------------------- *
   Destroys the render pass.
 * -------------------------------------------------------------------------- */
void RenderPass::destroy()
{
    if (isValid())
        impl->destroy();
}

/* -------------------------------------------------------------------------- *
   Returns true if the swap chain is not a null handle.
 * -------------------------------------------------------------------------- */
bool RenderPass::isValid() const
{ return impl->renderPass != VK_NULL_HANDLE; }

/* -------------------------------------------------------------------------- *
   Returns the render pass handle.
 * -------------------------------------------------------------------------- */
VkRenderPass RenderPass::handle() const
{ return impl->renderPass; }

} // namespace vk
} // namespace kuu
