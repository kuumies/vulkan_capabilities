/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of Vulkan sync related classes.
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A vulkan semaphore wrapper class
 * -------------------------------------------------------------------------- */
class Semaphore
{
public:
    // Constructs the semaphore.
    Semaphore(const VkDevice& logicalDevice);

    // Creates and destroys the semaphore.
    void create();
    void destroy();

    // Returns true if the handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the handle.
    VkSemaphore handle() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
