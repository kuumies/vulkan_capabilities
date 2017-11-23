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
        bool showHeader = true;
        Row header;
        std::vector<Row> valueRows;
    };

    // True if the system has Vulkan implementation.
    bool hasVulkan = false;

    // Physical device data.
    struct PhysicalDeviceData
    {
        std::string name;

        std::vector<Entry> properties;
        std::vector<Entry> extensions;
        std::vector<Entry> layers;
        std::vector<Entry> features;
        std::vector<Entry> formats;
        std::vector<Entry> queues;
        std::vector<Entry> memories;
        std::vector<Entry> limits;
        std::vector<Entry> surface;
    };
    std::vector<PhysicalDeviceData> physicalDeviceData;
};

} // namespace vk_capabilities
} // namespace kuu
