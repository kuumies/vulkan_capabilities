/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::stringify namespace.
 * -------------------------------------------------------------------------- */
 
#pragma once

#include <memory>
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

} // namespace stringify
} // namespace vk
} // namespace kuu
