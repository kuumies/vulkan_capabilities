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
    if (properties.size() == 0)
        return std::string();

    for (const Row& r : properties[0].valueRows)
        for (const Cell& c : r.cells)
            if (c.value == "Name" && r.cells.size() > 1)
                return r.cells[1].value;
    return std::string();

}

} // namespace vk_capabilities
} // namespace kuu
