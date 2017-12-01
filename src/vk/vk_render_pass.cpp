/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::RenderPass class
 * -------------------------------------------------------------------------- */

#include "vk_render_pass.h"
#include <algorithm>
#include <iostream>
#include "vk_stringify.h"

/* -------------------------------------------------------------------------- */

namespace kuu
{
namespace vk
{

struct RenderPass::Impl
{
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

        return true;
    }

    void destroy()
    {
        vkDestroyRenderPass(
            logicalDevice, // [in] logical device
            renderPass,    // [in] render pass
            NULL);         // [in] allocator callback

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
};

RenderPass::RenderPass(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
}

RenderPass& RenderPass::setLogicalDevice(const VkDevice& logicalDevice)
{
    impl->logicalDevice = logicalDevice;
    return *this;
}

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

} // namespace vk
} // namespace kuu
