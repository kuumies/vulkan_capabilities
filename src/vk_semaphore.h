/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Semaphore class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

class Device;

/* ---------------------------------------------------------------- *
   A semaphore for e.g. waiting and signaling queue actions.
 * ---------------------------------------------------------------- */
class Semaphore
{
public:
    // Constructs the semaphore.
    Semaphore(const Device& device);

    // Returns true if the semaphore creation succeeded.
    bool isValid() const;

    // Returns handle.
    VkSemaphore handle() const;
    // Cast to handle.
    operator VkSemaphore() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
