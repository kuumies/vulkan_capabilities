/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::stringify namespace.
 * -------------------------------------------------------------------------- */
 
#pragma once

#include <string>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{
namespace stringify
{

// Returns version number as a string in form <major>.<minor>.<patch>
std::string versionNumber(const uint32_t version);

// Returns the device type as a string.
std::string physicalDeviceType(const VkPhysicalDeviceType type);
// Returns the device type description
std::string physicalDeviceTypeDesc(const VkPhysicalDeviceType type);

// Returns the UUID as a string.
std::string uuid(const uint8_t* uuid, int size = VK_UUID_SIZE);
// Returns the LUID as a string.
std::string luid(const uint8_t* luid, int size = VK_LUID_SIZE_KHR);

// Returns flags as a string.
std::string queue(VkQueueFlags flags);
std::string formatFeature(VkFormatFeatureFlags flags);
std::string surfaceTransformFlags(VkSurfaceTransformFlagsKHR flags);
std::string compositeAlphaFlags(VkCompositeAlphaFlagsKHR flags);
std::string imageUsageFlags(VkImageUsageFlags flags);
std::string memoryProperty(VkMemoryPropertyFlags flags);
std::string memoryHeap(VkMemoryHeapFlags flagBits);

// Returns enums as a string
std::string result(const VkResult result);
std::string resultDesc(const VkResult result);
std::string presentMode(VkPresentModeKHR mode);
std::string format(VkFormat format);
std::string colorSpace(VkColorSpaceKHR colorSpace);

// Returns extents as a string
std::string extent2D(const VkExtent2D& e);
std::string extent3D(const VkExtent3D& e);

// Returns hex value as a string
std::string hexValueToString(uint32_t v);

} // namespace stringify
} // namespace vk
} // namespace kuu
