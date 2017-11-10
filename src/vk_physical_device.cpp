/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::PhysicalDevice class
 * ---------------------------------------------------------------- */

#include "vk_physical_device.h"

/* ---------------------------------------------------------------- */

#include <algorithm>
#include <iostream>

/* ---------------------------------------------------------------- */

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats(
    VkSurfaceKHR surface,
    VkPhysicalDevice physicalDevice)
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &formatCount,
            nullptr);

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    if (formatCount > 0)
    {
        surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice, surface,
            &formatCount, surfaceFormats.data());
    }

    return surfaceFormats;
}

/* ---------------------------------------------------------------- */

struct PhysicalDevice::Data
{
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    std::vector<VkExtensionProperties> extensions;
};

/* ---------------------------------------------------------------- */

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device)
    : d(std::make_shared<Data>())
{
    d->device  = device;

    vkGetPhysicalDeviceProperties(d->device, &d->properties);
    vkGetPhysicalDeviceFeatures(d->device, &d->features);

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

VkSurfaceFormatKHR PhysicalDevice::suitableSurfaceFormat(
        const Surface& surface,
        VkFormat format,
        VkColorSpaceKHR colorSpace) const
{
    std::vector<VkSurfaceFormatKHR> formats =
            physicalDeviceSurfaceFormats(
                surface.handle(),
                d->device);
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        return { VK_FORMAT_B8G8R8A8_UNORM,
                 VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    for (const VkSurfaceFormatKHR& availableFormat : formats)
    {
        if (availableFormat.format == format &&
            availableFormat.colorSpace == colorSpace)
        {
            return availableFormat;
        }
    }

    return formats[0];
}

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

bool PhysicalDevice::isImageExtentSupported(
        const Surface& surface,
        const glm::ivec2& extent) const
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            d->device,
            surface.handle(),
            &capabilities);

    const uint32_t w = extent.x;
    const uint32_t h = extent.y;

    return w >= capabilities.minImageExtent.width  &&
           w <= capabilities.maxImageExtent.width  &&
           h >= capabilities.minImageExtent.height &&
           h <= capabilities.maxImageExtent.height;
}

/* ---------------------------------------------------------------- */

bool PhysicalDevice::isSwapChainImageCountSupported(
        const Surface& surface,
        uint32_t count) const
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            d->device,
            surface.handle(),
            &capabilities);

    return count >= capabilities.minImageCount &&
           count <= capabilities.maxImageCount;
}

/* ---------------------------------------------------------------- */

bool PhysicalDevice::isPresentModeSupported(
        const Surface& surface,
        VkPresentModeKHR presentMode) const
{

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        d->device, surface.handle(),
        &presentModeCount, nullptr);

    std::vector<VkPresentModeKHR> presentModes;
    if (presentModeCount > 0)
    {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            d->device, surface.handle(),
            &presentModeCount, presentModes.data());
    }

    const auto it = std::find_if(
        presentModes.begin(),
        presentModes.end(),
        [presentMode](const VkPresentModeKHR& mode)
    { return mode == presentMode; });

    return it != presentModes.end();
}

/* ---------------------------------------------------------------- */

bool PhysicalDevice::isSurfaceSupported(
        const Surface& surface,
        VkFormat format,
        VkColorSpaceKHR colorSpace) const
{  
    std::vector<VkSurfaceFormatKHR> surfaceFormats =
            physicalDeviceSurfaceFormats(
                surface.handle(),
                d->device);

    const auto it = std::find_if(
        surfaceFormats.begin(),
        surfaceFormats.end(),
        [format, colorSpace](const VkSurfaceFormatKHR& fmt)
    {
        return fmt.format     == format &&
               fmt.colorSpace == colorSpace;
    });

    return it != surfaceFormats.end();
}

/* ---------------------------------------------------------------- */

void PhysicalDevice::dump()
{
    std::string deviceType;
    switch(d->properties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:          deviceType = "Other";          break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: deviceType = "Integrated GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   deviceType = "Discrete GPU";   break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    deviceType = "Virtual  GPU";   break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:            deviceType = "CPU";            break;
        default:
            break;
    }

    std::cout << "Physical device" << std::endl;
    std::cout << "\tDevice name... " << d->properties.deviceName << std::endl;
    std::cout << "\tDevice type... " << deviceType.c_str() << std::endl;
}

} // namespace vk
} // namespace kuu
