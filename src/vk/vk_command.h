/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of Vulkan command related classes.
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
   A vulkan command pool class
 * -------------------------------------------------------------------------- */
class CommandPool
{
public:
    // Constructs the command pool.
    CommandPool(const VkDevice& logicalDevice);

    // Sets and gets the queue family index.
    CommandPool& setQueueFamilyIndex(uint32_t queueFamilyIndex);
    uint32_t queueFamilyIndex() const;

    // Creates and destroys the command pool.
    void create();
    void destroy();

    // Returns true if the handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the handle.
    VkCommandPool  handle() const;

    // Allocates N command buffers from the pool.
    std::vector<VkCommandBuffer> allocate(
        VkCommandBufferLevel level,
        uint32_t count);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
