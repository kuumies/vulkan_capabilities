/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Device class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

/* ---------------------------------------------------------------- */

#include "vk_queue.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- *
   A vulkan logical device.
 * ---------------------------------------------------------------- */
class Device
{
public:
    // Constructs the device.
    Device(VkPhysicalDevice physicalDevice);

    // Returns the device handle. Device must have been created.
    VkDevice handle() const;

    // Creates the device. This function will update the input queue
    // handles. Returns true if the device creation succeeded.
    bool create(std::vector<Queue> queues,
                 const std::vector<std::string>& physicalDeviceExtensions);
    // Destroys the device.
    void destroy();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
