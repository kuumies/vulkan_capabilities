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
    // Layer data.
    struct Layer
    {
        const std::string name;
        const std::string desc;
        const std::string specVersion;
        const std::string implVersion;
    };
    // Limits data.
    struct Limit
    {
        const std::string name;
        const std::string value;
        const std::string desc;
    };
    // Memory Data
    struct Memory
    {
        struct Heap
        {
            const std::string index;
            const std::string size;
            const std::string flags;
            const std::string properties;
        };

        std::vector<Heap> heaps;
    };

    // Queue data
    struct Queue
    {
        const std::string familyIndex;
        const std::string queueCount;
        const std::string capabilities;
        const std::string minImageTransferGranularity;
        const std::string timestampValidBits;
    };

    // True if the system has Vulkan implementation.
    bool hasVulkan = false;

    // Instance extensions
    std::vector<std::pair<std::string, std::string>> instanceExtensions;
    // Instance layers
    std::vector<Layer> instanceLayers;

    // Physical device data.
    struct PhysicalDeviceData
    {
        // Main properties, [key -> label name, value -> label value]
        std::vector<std::pair<std::string, std::string>> mainProperties;
        // Features, [key -> label name, value -> label value, supported/unsupported]
        std::vector<std::pair<std::string, bool>> mainFeatures;
        // Extensions, [key -> label name, value -> label value]
        std::vector<std::pair<std::string, std::string>> extensions;
        // Limits
        std::vector<Limit> limits;
        // Queues
        std::vector<Queue> queues;
        // Memory
        Memory memory;

        // Returns the name string. If the name does not exits then a
        // empty string is returned.
        std::string name() const;
    };
    std::vector<PhysicalDeviceData> physicalDeviceData;
};

} // namespace vk_capabilities
} // namespace kuu
