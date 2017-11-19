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
    // Extensions data.
    struct Extension
    {
        const std::string name;
        const std::string version;
    };

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

    // Format data
    struct Format
    {
        const std::string format;
        const std::string desc;
        const std::string linearTilingFeatures;
        const std::string optimalTilingFeatures;
        const std::string bufferFeatures;
    };

    // Property data
    struct Property
    {
        std::string name;
        std::string value;
    };

    // Feature data
    struct Feature
    {
        std::string name;
        bool supported;
    };

    // True if the system has Vulkan implementation.
    bool hasVulkan = false;

    // Instance extensions
    std::vector<Extension> instanceExtensions;
    // Instance layers
    std::vector<Layer> instanceLayers;

    // Physical device data.
    struct PhysicalDeviceData
    {
        // Main properties, [key -> label name, value -> label value]
        std::vector<Property> mainProperties;
        // Features, [key -> label name, value -> label value, supported/unsupported]
        std::vector<Feature> mainFeatures;
        // Extensions, [key -> label name, value -> label value]
        std::vector<Extension> extensions;
        // Limits
        std::vector<Limit> limits;
        // Queues
        std::vector<Queue> queues;
        // Memory
        Memory memory;
        // Formats
        std::vector<Format> formats;

        // Returns the name string. If the name does not exits then a
        // empty string is returned.
        std::string name() const;
    };
    std::vector<PhysicalDeviceData> physicalDeviceData;
};

} // namespace vk_capabilities
} // namespace kuu
