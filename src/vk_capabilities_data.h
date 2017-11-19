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

    struct Cell
    {
        enum class Style
        {
            Header,
            NameLabel,
            ValueLabel,
            ValueLabelValid,
            ValueLabelInvalid,
        };

        Style style;
        std::string value;
        std::string desc;
        int size; // -1, no size
    };

    struct Row
    {
        std::vector<Cell> cells;
    };

    struct Entry
    {
        Row header;
        std::vector<Row> valueRows;
    };


    // True if the system has Vulkan implementation.
    bool hasVulkan = false;

    // Physical device data.
    struct PhysicalDeviceData
    {
        std::vector<Entry> properties;
        std::vector<Entry> extensions;
        std::vector<Entry> layers;
        std::vector<Entry> features;

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
