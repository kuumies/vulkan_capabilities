/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::RenderPass class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A Vulkan render pass wrapper class.
 * -------------------------------------------------------------------------- */
class RenderPass
{
public:
    // Constructs the render pass.
    RenderPass(const VkPhysicalDevice& physicalDevice,
               const VkDevice& logicalDevice);

    // Sets the logical device if not set during construction.
    RenderPass& addAttachmentDescription(const VkAttachmentDescription& description);
    std::vector<VkAttachmentDescription> attachmentDescriptions() const;

    // Sets and gets the subpass descriptions.
    RenderPass& addSubpassDescription(const VkSubpassDescription& description);
    std::vector<VkSubpassDescription> subpassDescriptions() const;

    // Sets and gets the subpass dependencies.
    RenderPass& addSubpassDependency(const VkSubpassDependency& dependency);
    std::vector<VkSubpassDependency> subpassDependencies() const;

    // Sets the swapchain image views for framebuffer creation.
    RenderPass& setSwapchainImageViews(
        const std::vector<VkImageView>& imageViews,
        const VkExtent2D& imageExtent);

    // Creates and destroys the render pass
    bool create();
    void destroy();

    // Returns true if the render pass handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the render pass handle.
    VkRenderPass handle() const;

    // Returns a swapchain framebuffer by index. If the index is not valid then
    // the output handle is a VK_NULL_HANDLE.
    VkFramebuffer framebuffer(uint32_t index) const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
