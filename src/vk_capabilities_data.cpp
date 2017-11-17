/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::Data class
 * -------------------------------------------------------------------------- */

#include "vk_capabilities_data.h"

namespace kuu
{
namespace vk_capabilities
{

std::string Data::PhysicalDeviceData::name() const
{
    for (const std::pair<std::string, std::string>& v : mainProperties)
        if (v.first == "Name")
            return v.second;
    return std::string();
}

} // namespace vk_capabilities
} // namespace kuu
