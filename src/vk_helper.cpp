/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::helper namespace.
 * -------------------------------------------------------------------------- */
 
#include "vk_helper.h"

/* -------------------------------------------------------------------------- */

#include <algorithm>
#include <vector>

namespace kuu
{
namespace vk
{
namespace helper
{

/* -------------------------------------------------------------------------- */

bool isInstanceExtensionSupported(const std::string& extension)
{
    // Get the extensions count.
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &extensionCount,
                                           nullptr);
    if (extensionCount == 0)
        return false;

    // Get the extensions.
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &extensionCount,
                                           extensions.data());

    const auto it = std::find_if(
        extensions.begin(),
        extensions.end(),
        [extension](const VkExtensionProperties& ex)
    {
        return std::string(ex.extensionName) ==
               std::string(extension);
    });

    return it != extensions.end();
}

} // namespace helper
} // namespace vk
} // namespace kuu
