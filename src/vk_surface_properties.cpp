/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::SurfaceProperties class
 * -------------------------------------------------------------------------- */

#include "vk_surface_properties.h"

/* -------------------------------------------------------------------------- */

#include <iostream>

/* -------------------------------------------------------------------------- */

#include "vk_stringify.h"

namespace kuu
{
namespace vk
{
namespace
{

/* -------------------------------------------------------------------------- */

bool getSurfaceCapabilities2(
        const VkInstance& instance,
        const VkPhysicalDevice& physicalDevice,
        const VkSurfaceKHR surface,
        VkSharedPresentSurfaceCapabilitiesKHR&  presentCapabilities,
        VkSurfaceCapabilities2KHR& surfaceCapabilities)
{
    auto fun = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)
        vkGetInstanceProcAddr(
            instance,
            "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    if (!fun)
        return false;

    VkPhysicalDeviceSurfaceInfo2KHR info;
    info.surface = surface;
    info.sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    info.pNext   = NULL;

    presentCapabilities.sType = VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR;

    surfaceCapabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    surfaceCapabilities.pNext = &presentCapabilities;
    VkResult result = fun(
            physicalDevice,
            &info,
            &surfaceCapabilities);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to get surface capabilities as "
                  << vk::stringify::toDescription(result)
                  << std::endl;
    }

    return true;
}
/* -------------------------------------------------------------------------- */

VkSurfaceCapabilitiesKHR getSurfaceCapabilities(
        const VkPhysicalDevice& physicalDevice,
        const VkSurfaceKHR surface)
{
    // Query basic surface capabilities needed in order to create a swap chain.
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,        // [in]  physical device handle
        surface,               // [in]  surface handle
        &surfaceCapabilities); // [out] surface capabilities

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to get surface capabilities as "
                  << vk::stringify::toDescription(result)
                  << std::endl;
    }

    return surfaceCapabilities;
}

/* -------------------------------------------------------------------------- */

std::vector<VkSurfaceFormatKHR> getSurfaceFormats(
    const VkPhysicalDevice& physicalDevice,
        const VkSurfaceKHR surface)
{
    uint32_t surfaceFormatCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &surfaceFormatCount,
        NULL);

    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &surfaceFormatCount,
        surfaceFormats.data());

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to get surface formats as "
                  << vk::stringify::toDescription(result)
                  << std::endl;
    }

    return surfaceFormats;

}
/* -------------------------------------------------------------------------- */

std::vector<VkPresentModeKHR> getPresentModes(
    const VkPhysicalDevice& physicalDevice,
        const VkSurfaceKHR surface)
{
    uint32_t presentModeCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &presentModeCount,
        NULL);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &presentModeCount,
        presentModes.data());

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to get present modes as "
                  << vk::stringify::toDescription(result)
                  << std::endl;
    }

    return presentModes;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

SurfaceProperties::SurfaceProperties(
        const VkInstance& instance,
        const VkPhysicalDevice& physicalDevice,
        const VkSurfaceKHR& surface)
{
    surfaceCapabilities = getSurfaceCapabilities(physicalDevice, surface);
    getSurfaceCapabilities2(instance, physicalDevice, surface,
                            presentCapabilities,
                            surfaceCapabilities2);
    surfaceFormats = getSurfaceFormats(physicalDevice, surface);
    presentModes   = getPresentModes(physicalDevice, surface);
}

} // namespace vk
} // namespace kuu
