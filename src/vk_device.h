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
#include "vk_swap_chain.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

class Surface;
class PhysicalDevice;

/* ---------------------------------------------------------------- *
   A vulkan logical device.
 * ---------------------------------------------------------------- */
class Device
{
public:
    // Queue parameters
    struct QueueParameters
    {
        Queue::Type type;
        uint32_t count;
        float priority;

        Surface* surface = nullptr;
    };

    // Constructs the device.
    Device(const PhysicalDevice& physicalDevice,
           const std::vector<QueueParameters>& params,
           const std::vector<std::string>& extensions);

    // Returns true if the device is valid.
    bool isValid() const;

    // Returns the device handle. Device must have been created.
    VkDevice handle() const;

    // Returns the queues.
    std::vector<Queue> queues() const;
    // Returns queue.
    Queue queue(Queue::Type type) const;

    // Creates and returns a swap chain.
    SwapChain createSwapChain(const Surface& surface) const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
