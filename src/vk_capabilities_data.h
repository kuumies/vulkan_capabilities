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
    struct Extension
    {
        const std::string name;
        const std::string version;
    };

    struct Layer
    {
        const std::string name;
        const std::string desc;
        const std::string specVersion;
        const std::string implVersion;
    };

    struct Limit
    {
        const std::string name;
        const std::string value;
        const std::string desc;
    };

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

    struct Queue
    {
        const std::string familyIndex;
        const std::string queueCount;
        const std::string capabilities;
        const std::string minImageTransferGranularity;
        const std::string timestampValidBits;
    };

    struct Format
    {
        const std::string format;
        const std::string desc;
        const std::string linearTilingFeatures;
        const std::string optimalTilingFeatures;
        const std::string bufferFeatures;
    };

    struct Property
    {
        const std::string name;
        const std::string value;
    };

    struct Feature
    {
        const std::string name;
        const bool supported;
    };

    // True if the system has Vulkan implementation.
    bool hasVulkan = false;

    // Instance data
    std::vector<Extension> instanceExtensions;
    std::vector<Layer> instanceLayers;

    // Physical device data.
    struct PhysicalDeviceData
    {
        std::vector<Property> mainProperties;
        std::vector<Feature> mainFeatures;
        std::vector<Extension> extensions;
        std::vector<Limit> limits;
        std::vector<Queue> queues;
        std::vector<Format> formats;
        Memory memory;

        // Returns the name string. If the name does not exits then a
        // empty string is returned.
        std::string name() const;
    };
    std::vector<PhysicalDeviceData> physicalDeviceData;
};

} // namespace vk_capabilities
} // namespace kuu
