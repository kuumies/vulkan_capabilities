/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::ShadowMapDepth class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

struct Texture2D;

class ShadowMapDepth
{
public:
    ShadowMapDepth(
        const VkPhysicalDevice& physicalDevice,
        const VkDevice& device,
        const uint32_t& graphicsQueueFamilyIndex,
        const VkRenderPass& renderPass);

    void recordCommands(const VkCommandBuffer& commandBuffer);
    void setShadowMap(std::shared_ptr<Texture2D> texture);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
