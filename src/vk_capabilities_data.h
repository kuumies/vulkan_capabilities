/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk_capabilities::Data class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <map>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk_capabilities
{

/* -------------------------------------------------------------------------- *
   A data for Vulkan Capabilities application.
 * -------------------------------------------------------------------------- */
struct Data
{
    // True if the system has Vulkan implementation.
    bool hasVulkan = false;

    // Physical device data.
    struct PhysicalDeviceData
    {
        // Main properties, [key -> label name, value -> label value]
        std::vector<std::pair<std::string, std::string>> mainProperties;

        // Returns the name string. If the name does not exits then a
        // empty string is returned.
        std::string name() const;
    };
    std::vector<PhysicalDeviceData> physicalDeviceData;
};

} // namespace vk_capabilities
} // namespace kuu
