/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::helper namespace.
 * -------------------------------------------------------------------------- */
 
#pragma once

#include <string>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{
namespace helper
{

// Returns true if the instance extension is supported.
bool isInstanceExtensionSupported(const std::string& extension);

} // namespace helper
} // namespace vk
} // namespace kuu
