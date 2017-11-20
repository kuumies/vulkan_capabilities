/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk_capabilities::DataCreator class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <map>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

/* -------------------------------------------------------------------------- */
#include "vk_capabilities_data.h"
#include "vk_instance.h"

namespace kuu
{
namespace vk_capabilities
{

/* -------------------------------------------------------------------------- */

struct Data;

/* -------------------------------------------------------------------------- *
   A data creator for Vulkan Capabilities application.
 * -------------------------------------------------------------------------- */
class DataCreator
{
public:
    // Construct the APP data creator from the Vulkan instance. This will
    // create fill the data structure. Get it with data() function.
    DataCreator(const vk::Instance& instance);

    // Returns the data.
    std::shared_ptr<Data> data() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk_capabilities
} // namespace kuu
