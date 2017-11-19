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
    for (const Property& v : mainProperties)
        if (v.name == "Name")
            return v.value;
    return std::string();
}

} // namespace vk_capabilities
} // namespace kuu
