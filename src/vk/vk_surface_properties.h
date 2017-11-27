/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::SurfaceProperties class
 * -------------------------------------------------------------------------- */

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A Vulkan physical device-dependant surface properties.
 * -------------------------------------------------------------------------- */
struct SurfaceProperties
{
    // Constructs the surface properties.
    SurfaceProperties(const VkInstance& instance,
                      const VkPhysicalDevice& physicalDevice,
                      const VkSurfaceKHR& surface);

    // Basic surface capabilities
    VkSurfaceCapabilitiesKHR surfaceCapabilities;

    // Extended surface capabilities
    VkSurfaceCapabilities2KHR surfaceCapabilities2;
    VkSharedPresentSurfaceCapabilitiesKHR  presentCapabilities;

    // Surface formats.
    std::vector<VkSurfaceFormatKHR> surfaceFormats;

    // Present modes.
    std::vector<VkPresentModeKHR> presentModes;
};

} // namespace vk
} // namespace kuu
