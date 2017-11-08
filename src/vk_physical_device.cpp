/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::PhysicalDevice class
 * ---------------------------------------------------------------- */

#include "vk_physical_device.h"

/* ---------------------------------------------------------------- */

#include <algorithm>

/* ---------------------------------------------------------------- */

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct PhysicalDevice::Data
{
    VkPhysicalDevice device;
    VkSurfaceKHR surface;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
    std::vector<VkExtensionProperties> extensions;
};

/* ---------------------------------------------------------------- */

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface)
    : d(std::make_shared<Data>())
{
    d->device  = device;
    d->surface = surface;

    vkGetPhysicalDeviceProperties(d->device, &d->properties);
    vkGetPhysicalDeviceFeatures(d->device, &d->features);
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(d->device, surface, &d->capabilities);

    // Get the surface format count.
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(d->device, d->surface,
                                         &formatCount, nullptr);
    if (formatCount > 0)
    {
        d->surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            d->device, d->surface,
            &formatCount, d->surfaceFormats.data());
    }

    // Get the present modes count.
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        d->device, surface,
        &presentModeCount, nullptr);

    if (presentModeCount > 0)
    {
        d->presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            d->device, surface,
            &presentModeCount, d->presentModes.data());
    }

    // Get the extension count
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(d->device, nullptr,
                                         &extensionCount, nullptr);

    // Get the extensions
    if (extensionCount)
    {

        d->extensions.resize(extensionCount);
        vkEnumerateDeviceExtensionProperties(
            d->device, nullptr,
            &extensionCount, d->extensions.data());
    }
}

/* ---------------------------------------------------------------- */

VkPhysicalDevice PhysicalDevice::handle() const
{ return d->device; }

/* ---------------------------------------------------------------- */

bool PhysicalDevice::isExtensionSupported(
    const std::string& extension) const
{
    const auto it = std::find_if(
        d->extensions.begin(),
        d->extensions.end(),
        [extension](const VkExtensionProperties& ex)
    {
        return std::string(ex.extensionName) == extension;
    });

    return it != d->extensions.end();
}

/* ---------------------------------------------------------------- */

bool PhysicalDevice::isExtensionSupported(
    const std::vector<std::string> &extensions) const
{
    for (const std::string& extension : extensions)
        if (!isExtensionSupported(extension))
            return false;
    return true;
}

/* ---------------------------------------------------------------- */

bool PhysicalDevice::isImageExtentSupported(const glm::ivec2& extent) const
{
    const uint32_t w = extent.x;
    const uint32_t h = extent.y;

    return w < d->capabilities.minImageExtent.width  ||
           w > d->capabilities.maxImageExtent.width  ||
           h < d->capabilities.minImageExtent.height ||
           h > d->capabilities.maxImageExtent.height;
}

/* ---------------------------------------------------------------- */

bool PhysicalDevice::isSwapChainImageCountSupported(uint32_t count) const
{
    return count < d->capabilities.minImageCount ||
           count > d->capabilities.maxImageCount;
}

/* ---------------------------------------------------------------- */

bool PhysicalDevice::isPresentModeSupported(
    VkPresentModeKHR presentMode) const
{
    // Check that the device supports the present mode.
    const auto it = std::find_if(
        d->presentModes.begin(),
        d->presentModes.end(),
        [presentMode](const VkPresentModeKHR& mode)
    { return mode == presentMode; });

    return it != d->presentModes.end();
}

/* ---------------------------------------------------------------- */

bool PhysicalDevice::isSurfaceSupported(
    VkFormat format,
    VkColorSpaceKHR colorSpace) const
{
    // Check that the device supports the surface mode.
    const auto it = std::find_if(
        d->surfaceFormats.begin(),
        d->surfaceFormats.end(),
        [format, colorSpace](const VkSurfaceFormatKHR& fmt)
    {
        return fmt.format     == format &&
               fmt.colorSpace == colorSpace;
    });

    return it != d->surfaceFormats.end();
}

} // namespace vk
} // namespace kuu
