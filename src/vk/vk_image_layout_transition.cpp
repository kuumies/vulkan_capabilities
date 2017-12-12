/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::image_layout_transition namespace.
 * -------------------------------------------------------------------------- */

#include "vk_image_layout_transition.h"

namespace kuu
{
namespace vk
{
namespace image_layout_transition
{

void record(const VkCommandBuffer& cmdbuffer,
            const VkImage& image,
            const VkImageLayout& oldImageLayout,
            const VkImageLayout& newImageLayout,
            const VkImageSubresourceRange& subresourceRange,
            const VkPipelineStageFlags& srcStageMask,
            const VkPipelineStageFlags& dstStageMask)
{
    // Create an image barrier object
    VkImageMemoryBarrier barrier = {};
    barrier.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext            = NULL;
    barrier.oldLayout        = oldImageLayout;
    barrier.newLayout        = newImageLayout;
    barrier.image            = image;
    barrier.subresourceRange = subresourceRange;

    // Define the actions that must be finished on the old layout before
    // before the image can be transitioned into new layout.
    switch (oldImageLayout)
    {
        // initial layout, no actions can pend on this
        case VK_IMAGE_LAYOUT_UNDEFINED:
            barrier.srcAccessMask = 0;
            break;

        // preinitialized linear image
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        // color attachment
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        // depth/stencil attachment
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        // transfer source -> wait until reads from the image have been finished.
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        // transfer destination -> wait until writes to image have been finished.
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        // shader read -> wait until shader reads have been finished
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

        default:
            break;
    }

    // Define the actions that must be finished for the new layout before
    // the image can be transitioned into it layout.
    switch (newImageLayout)
    {
        //  transfer destination -> image writes must have been finished
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        //  transfer source -> image reads must have been finished
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        // color attachment -> color buffer writes must have been finished
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        // depth/stencil attachment -> depth/stencil buffer writes must have been finished
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        // shader read as sampler -> writes to image must have been finished
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

        default:
            break;
    }

    // HACK!
    if (barrier.srcAccessMask == 0 && barrier.dstAccessMask == VK_ACCESS_SHADER_READ_BIT)
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        cmdbuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

void record(const VkCommandBuffer& cmdbuffer,
            const VkImage& image,
            const VkImageAspectFlags& aspectMask,
            const VkImageLayout& oldImageLayout,
            const VkImageLayout& newImageLayout,
            const VkPipelineStageFlags& srcStageMask,
            const VkPipelineStageFlags& dstStageMask)
{
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask   = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount   = 1;
    subresourceRange.layerCount   = 1;

    record(cmdbuffer,
           image,
           oldImageLayout,
           newImageLayout,
           subresourceRange,
           srcStageMask,
           dstStageMask);
}

} // namespace image_layout_transition
} // namespace vk
} // namespace kuu
