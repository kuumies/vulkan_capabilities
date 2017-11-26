/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::PhysicalDevice class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A Vulkan physical device wrapper struct. Physical device handle is valid
   until Vulkan instance is destroyed.
 * -------------------------------------------------------------------------- */
struct PhysicalDevice
{   
    // Constructs the physical device.
    PhysicalDevice(const VkPhysicalDevice& physicalDevice,
                   const VkInstance& instance);

    // Physical device handle.
    VkPhysicalDevice physicalDevice;

    // Main properties (name, type etc.)
    VkPhysicalDeviceProperties properties;

    // Features that user can enable before creating the logical device.
    // Remember that the each feature needs to be MANUALLY enabled.
    VkPhysicalDeviceFeatures features;

    // Memory properties (memory heaps, memory types)
    VkPhysicalDeviceMemoryProperties memoryProperties;

    // Queue families. Vector index is the queue family index.
    std::vector<VkQueueFamilyProperties> queueFamilies;
    // Queue presentation support status.
    std::vector<bool> queuePresentation;

    // A vector of all of the formats that Vulkan support and the properties
    // associated with the format.
    std::vector<std::pair<VkFormat, VkFormatProperties>> formats;

    // All the available extensions that the device supports.
    std::vector<VkExtensionProperties> extensions;

    // Features that requires an extension use
    bool hasExtensionsFeatures;
    VkPhysicalDeviceVariablePointerFeaturesKHR featuresVariablePointer;
    VkPhysicalDeviceMultiviewFeaturesKHX multiviewFeatures;
    VkPhysicalDevice16BitStorageFeaturesKHR features16BitStorage;
    VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR yuvSamplerFeatures;
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT blendFeatures;

    // Properties that requires an extension use
    bool hasExtensionsProperties;
    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blendProperties;
    VkPhysicalDeviceDiscardRectanglePropertiesEXT  discardRectangleProperties;
    VkPhysicalDeviceIDPropertiesKHR  idProperties;
    VkPhysicalDeviceMultiviewPropertiesKHX  multiviewProperties;
    VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX multiviewPerView;
    VkPhysicalDevicePointClippingPropertiesKHR clippingProperties;
    VkPhysicalDevicePushDescriptorPropertiesKHR  pushDescriptorProperties;
    VkPhysicalDeviceSampleLocationsPropertiesEXT sampleLocationsProperties;
    VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT samplerMinMaxProperties;
};

} // namespace vk_capabilities
} // namespace kuu
