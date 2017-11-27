/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::MonitorProperties class
 * -------------------------------------------------------------------------- */

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A Vulkan physical device-dependant monitor properties. Note that these
   settings are valid only for e.g. embedded linux devices, not for typical
   OS, e.g Windows.
 * -------------------------------------------------------------------------- */
struct MonitorProperties
{
    struct DisplayPlane
    {
        VkDisplayPlanePropertiesKHR properties;
        std::vector<std::pair<VkDisplayModeKHR, VkDisplayPlaneCapabilitiesKHR>> capabilities;
    };

    struct Display
    {
        VkDisplayKHR display;
        VkDisplayPropertiesKHR properties;
        std::vector<VkDisplayModePropertiesKHR> modeProperties;
        std::vector<DisplayPlane> planes;
    };

    // Constructs the monitor properties.
    MonitorProperties(const VkPhysicalDevice& physicalDevice);

    // Displays
    std::vector<Display> displays;
};



} // namespace vk
} // namespace kuu
