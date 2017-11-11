/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Semaphore class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

/* ---------------------------------------------------------------- */

#include "vk_swap_chain.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

class Device;

/* ---------------------------------------------------------------- *
   A semaphore
 * ---------------------------------------------------------------- */
class Semaphore
{
public:
    // Constructs the pipeline
    Semaphore(const Device& device);

    // Returns true if the semaphore is valid.
    bool isValid() const;

    // Returns handle.
    VkSemaphore handle() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
