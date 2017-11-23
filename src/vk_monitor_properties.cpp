/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::MonitorProperties class
 * -------------------------------------------------------------------------- */

#include "vk_monitor_properties.h"

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

std::vector<VkDisplayPropertiesKHR> getDisplayProperties(
        const VkPhysicalDevice& physicalDevice)
{
    uint32_t propertyCount = 0;
    VkResult result = vkGetPhysicalDeviceDisplayPropertiesKHR(
        physicalDevice,
        &propertyCount,
        NULL);

    std::vector<VkDisplayPropertiesKHR> properties(propertyCount);
    result = vkGetPhysicalDeviceDisplayPropertiesKHR(
        physicalDevice,
        &propertyCount,
        properties.data());

    return properties;
}

/* -------------------------------------------------------------------------- */

std::vector<VkDisplayModePropertiesKHR> getDisplayModeProperties(
    const VkPhysicalDevice& physicalDevice,
    const VkDisplayKHR& display)
{
    uint32_t propertyCount;
    VkResult result = vkGetDisplayModePropertiesKHR(
        physicalDevice,
        display,
        &propertyCount,
        NULL);

    std::vector<VkDisplayModePropertiesKHR> properties(propertyCount);
    result = vkGetDisplayModePropertiesKHR(
        physicalDevice,
        display,
        &propertyCount,
        properties.data());

    return properties;
}

/* -------------------------------------------------------------------------- */

std::vector<VkDisplayPlanePropertiesKHR> getDisplayPlaneProperties(
        const VkPhysicalDevice& physicalDevice)
{
    uint32_t propertyCount = 0;
    VkResult result = vkGetPhysicalDeviceDisplayPlanePropertiesKHR(
        physicalDevice,
        &propertyCount,
        NULL);

    std::vector<VkDisplayPlanePropertiesKHR> properties(propertyCount);
    result = vkGetPhysicalDeviceDisplayPlanePropertiesKHR(
        physicalDevice,
        &propertyCount,
        properties.data());

    return properties;
}

/* -------------------------------------------------------------------------- */

std::vector<std::vector<VkDisplayKHR>> getDisplayPlaneSupportedDisplays(
        const VkPhysicalDevice& physicalDevice,
        const uint32_t& planeCount)
{
    std::vector<std::vector<VkDisplayKHR>> planeDisplays;
    for (uint32_t planeIndex = 0; planeIndex < planeCount; ++planeIndex)
    {
        uint32_t displayCount = 0;
        VkResult result = vkGetDisplayPlaneSupportedDisplaysKHR(
            physicalDevice,
            planeIndex,
            &displayCount,
            NULL);

        std::vector<VkDisplayKHR> displays(displayCount);
        result = vkGetDisplayPlaneSupportedDisplaysKHR(
            physicalDevice,
            planeIndex,
            &displayCount,
            displays.data());

        planeDisplays.push_back(displays);
    }

    return planeDisplays;
}

/* -------------------------------------------------------------------------- */

VkDisplayPlaneCapabilitiesKHR getDisplayPlaneCapabilities(
    const VkPhysicalDevice& physicalDevice,
    const VkDisplayModeKHR& displayMode,
    const uint32_t planeIndex)
{
    VkDisplayPlaneCapabilitiesKHR capabilities;
    VkResult result = vkGetDisplayPlaneCapabilitiesKHR(
        physicalDevice,
        displayMode,
        planeIndex,
        &capabilities);
    return capabilities;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

MonitorProperties::MonitorProperties(
        const VkPhysicalDevice& physicalDevice)
{
    std::vector<VkDisplayPlanePropertiesKHR> displayPlaneProperties =
        getDisplayPlaneProperties(physicalDevice);

    std::vector<VkDisplayPropertiesKHR> displayProperties =
        getDisplayProperties(physicalDevice);

    for(const VkDisplayPropertiesKHR& properties: displayProperties)
    {
        Display display;
        display.display        = properties.display;
        display.properties     = properties;
        display.modeProperties = getDisplayModeProperties(physicalDevice,
                                                          display.display);

        for (uint32_t planeIndex = 0; planeIndex < displayPlaneProperties.size(); ++planeIndex)
        {
            const VkDisplayPlanePropertiesKHR& displayPlaneProperty =
                displayPlaneProperties[planeIndex];

            for (const VkDisplayModePropertiesKHR& properties : display.modeProperties)
            {
                VkDisplayPlaneCapabilitiesKHR planeCapabilities =
                        getDisplayPlaneCapabilities(
                            physicalDevice,
                            properties.displayMode,
                            planeIndex);

                DisplayPlane displayPlane;
                displayPlane.properties = displayPlaneProperty;
                displayPlane.capabilities.push_back(
                    { properties.displayMode, planeCapabilities });

                display.planes.push_back(displayPlane);
            }
        }

        this->displays.push_back(display);
    }
}

} // namespace vk
} // namespace kuu
