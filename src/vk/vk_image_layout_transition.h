/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::image_layout_transition namespace.
 * -------------------------------------------------------------------------- */

#pragma once

#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{
namespace image_layout_transition
{

// Records a image layout transition command into command buffer.
void record(const VkCommandBuffer& cmdbuffer,
            const VkImage& image,
            const VkImageLayout& oldImageLayout,
            const VkImageLayout& newImageLayout,
            const VkImageSubresourceRange& subresourceRange,
            const VkPipelineStageFlags& srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            const VkPipelineStageFlags& dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

void record(const VkCommandBuffer& cmdbuffer,
            const VkImage& image,
            const VkImageAspectFlags& aspectMask,
            const VkImageLayout& oldImageLayout,
            const VkImageLayout& newImageLayout,
            const VkPipelineStageFlags& srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            const VkPipelineStageFlags& dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

} // namespace image_layout_transition
} // namespace vk
} // namespace kuu

