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
    RenderPass(const VkDevice& logicalDevice);

    // Sets and gets the attachment descriptions.
    RenderPass& setAttachmentDescriptions(
        const std::vector<VkAttachmentDescription>& descriptions);
    std::vector<VkAttachmentDescription> attachmentDescriptions() const;

    // Sets and gets the subpass descriptions.
    RenderPass& setSubpassDescriptions(
        const std::vector<VkSubpassDescription>& descriptions);
    std::vector<VkSubpassDescription> subpassDescriptions() const;

    // Sets and gets the subpass dependencies.
    RenderPass& setSubpassDependencies(
        const std::vector<VkSubpassDependency>& dependencies);
    std::vector<VkSubpassDependency> subpassDependencies() const;

    // Creates and destroys the render pass
    void create();
    void destroy();

    // Returns true if the render pass handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the render pass handle.
    VkRenderPass handle() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
