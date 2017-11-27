/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::helper namespace.
 * -------------------------------------------------------------------------- */
 
#include "vk_helper.h"

/* -------------------------------------------------------------------------- */

#include <algorithm>

namespace kuu
{
namespace vk
{
namespace helper
{

/* -------------------------------------------------------------------------- *
   Returns the first queue family index of queue with the graphics ability.
   The return value is -1 if the queue family is not supported.
 * -------------------------------------------------------------------------- */
int findQueueFamilyIndex(
    const VkQueueFlags queue,
    const std::vector<VkQueueFamilyProperties>& queueFamilies,
    const std::vector<int>& ignoreIndices)
{
    int index = -1;
    for(size_t i = 0; i < queueFamilies.size(); ++i)
    {
        const auto it = std::find(
            ignoreIndices.begin(),
            ignoreIndices.end(),
            int(i));
        if (it != ignoreIndices.end())
            continue;

        const VkQueueFamilyProperties& props = queueFamilies[i];
        if (props.queueFlags & queue)
            index = int(i);
    }
    return index;
}

/* -------------------------------------------------------------------------- *
   Returns the queue family index of queue with the presentation ability.
   The return value is -1 if the queue family is not supported.
 * -------------------------------------------------------------------------- */
int findPresentationQueueFamilyIndex(
        const VkPhysicalDevice& physicalDevice,
        const VkSurfaceKHR& surface,
        const std::vector<VkQueueFamilyProperties>& queueFamilies,
    const std::vector<int>& ignoreIndices)
{
    int index = -1;
    for(size_t i = 0; i < queueFamilies.size(); ++i)
    {
        const auto it = std::find(
            ignoreIndices.begin(),
            ignoreIndices.end(),
            int(i));
        if (it != ignoreIndices.end())
            continue;

        VkBool32 supported = VK_FALSE;
        const VkResult result =
            vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice,
                uint32_t(i),
                surface,
                &supported);
        if (result == VK_SUCCESS && supported == VK_TRUE)
            index = int(i);
    }
    return index;
}

/* -------------------------------------------------------------------------- *
   Finds the memory type index.
 * -------------------------------------------------------------------------- */
int findMemoryTypeIndex(
    const VkPhysicalDeviceMemoryProperties& memProperties,
    const VkMemoryRequirements& memRequirements,
    uint32_t propertyFlags)
{
    // Go trought the memory types.
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        // Check that the type bit is enabled.
        const uint32_t memoryTypeBits = (i << i);
        if (!(memRequirements.memoryTypeBits & memoryTypeBits))
            continue;

        // Check that the memory supports required properties.
        if (!(memProperties.memoryTypes[i].propertyFlags & propertyFlags))
            continue;

        return i;
    }

    return -1;
}

/* -------------------------------------------------------------------------- *
   Finds the swap chain surface format.
 * -------------------------------------------------------------------------- */
VkSurfaceFormatKHR findSwapchainSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
    {
        if (availableFormat.format     == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

/* -------------------------------------------------------------------------- *
   Finds the swap chain present mode.
 * -------------------------------------------------------------------------- */
VkPresentModeKHR findSwapchainPresentMode(
    const std::vector<VkPresentModeKHR> availablePresentModes)
{
    for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;

    return VK_PRESENT_MODE_FIFO_KHR;
}

/* -------------------------------------------------------------------------- *
   Finds the swap chain extent.
 * -------------------------------------------------------------------------- */
VkExtent2D findSwapchainImageExtent(
    const VkSurfaceCapabilitiesKHR &capabilities,
    const VkExtent2D& targetExtent)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    VkExtent2D  minImageExtent = capabilities.minImageExtent;
    VkExtent2D  maxImageExtent = capabilities.maxImageExtent;
    VkExtent2D actualExtent = targetExtent;
    actualExtent.width  = std::max(minImageExtent.width,  std::min(maxImageExtent.width,  actualExtent.width));
    actualExtent.height = std::max(minImageExtent.height, std::min(maxImageExtent.height, actualExtent.height));
    return actualExtent;
}

/* -------------------------------------------------------------------------- *
   Finds the swap image count.
 * -------------------------------------------------------------------------- */
int findSwapchainImageCount(const VkSurfaceCapabilitiesKHR &capabilities)
{
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;
    return imageCount;
}

} // namespace helper
} // namespace vk
} // namespace kuu
