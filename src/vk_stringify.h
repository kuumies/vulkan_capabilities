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

// Returns the result as a string.
std::string toString(const VkResult result);
// Returns the result description
std::string toDescription(const VkResult result);

// Returns version number as a string in form <major>.<minor>.<patch>
std::string versionNumber(const uint32_t version);

// Returns the device type as a string.
std::string toString(const VkPhysicalDeviceType type);
// Returns the device type description
std::string toDescription(const VkPhysicalDeviceType type);

// Returns the UUIID as a string.
std::string toString(const uint8_t* uuid, int size = VK_UUID_SIZE);

// Returns queue capabilities as a string.
std::string toString(VkQueueFlags flags);
// Returns format features as a string.
std::string formatFeature(VkFormatFeatureFlags flags);

// Returns the 3D extent as a string.
std::string toString(const VkExtent3D& e);

// Returns the hex value as a string.
std::string hexValueToString(uint32_t v);

} // namespace stringify
} // namespace vk
} // namespace kuu
