/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::helper namespace.
 * -------------------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{
namespace helper
{

// Finds the index of the queue family based on the given in queue flags
// and queue family indices to ignore. Note that to find a queue family with
// a presentation support is found with findPresentationQueueFamilyIndex
// function.
//
// queueFamilies vector is a property of PhysicalDeviceInfo.
//
// User can give in indices that are not valid for the queue, e.g. graphics
// and presentation queue families should be different when a logical device
// is created therefore used indices must be set.
//
// The return value is -1 if the queue family is not supported.
int findQueueFamilyIndex(
    const VkQueueFlags queue,
    const std::vector<VkQueueFamilyProperties>& queueFamilies,
    const std::vector<int>& ignoreIndices = std::vector<int>());

// Finds the index of the queue family with presentation support.
//
// User can give in indices that are not valid for the queue, e.g. graphics
// and presentation queue families should be different when a logical device
// is created therefore used indices must be set.
//
// queuePresentation vector is a property of PhysicalDeviceInfo.
//
// The return value is -1 if there is no queue family available with a
// presentation support.
int findPresentationQueueFamilyIndex(
    const VkPhysicalDevice& physicalDevice,
    const VkSurfaceKHR& surface,
    const std::vector<VkQueueFamilyProperties>& queueFamilies,
    const std::vector<int>& ignoreIndices = std::vector<int>());

// Returns the memory type index based on the memory type and needed
// memory properties.
//
// Example to find the index of host visible and coherent memory type index:
// const uint32_t index =
//  findMemoryTypeIndex(
//      getMemProperties(),
//      getMemRequirements(),
//      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//
int findMemoryTypeIndex(
    const VkPhysicalDeviceMemoryProperties& memProperties,
    const VkMemoryRequirements& memRequirements,
    uint32_t propertyFlags);

// Returns a surface format for swapchain.
VkSurfaceFormatKHR findSwapchainSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats);

// Returns a present mode for swapchain.
VkPresentModeKHR findSwapchainPresentMode(
    const std::vector<VkPresentModeKHR> availablePresentModes);

// Returns a image extent for swapchain.
VkExtent2D findSwapchainImageExtent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    const VkExtent2D& targetExtent);

// Returns a image count for swapchain.
int findSwapchainImageCount(const VkSurfaceCapabilitiesKHR& capabilities);

} // namespace helper
} // namespace vk
} // namespace kuu
