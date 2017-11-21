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

/* -------------------------------------------------------------------------- */

struct Instance;

/* -------------------------------------------------------------------------- *
   A Vulkan physical device wrapper struct. Physical device handle is valid
   until Vulkan instance is destroyed.
 * -------------------------------------------------------------------------- */
struct PhysicalDevice
{   
    // Constructs the physical device.
    PhysicalDevice(const VkPhysicalDevice& physicalDevice,
                   const Instance& instance);

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

    // SPIR-V VariablePointers and VariablePointersStorageBuffer capabilities.
    VkPhysicalDeviceVariablePointerFeaturesKHR featuresVariablePointer;

    // Render pass multiview capablities
    VkPhysicalDeviceMultiviewFeaturesKHX multiviewFeatures;

    // Storage 16 bit capabilities
    VkPhysicalDevice16BitStorageFeaturesKHR features16BitStorage;

    // Samper Yuv conversion capability
    VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR yuvSamplerFeatures;

    // Advanced blending operation capability
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT blendFeatures;
};

} // namespace vk_capabilities
} // namespace kuu
