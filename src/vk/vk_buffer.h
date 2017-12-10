/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Buffer class
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
   A vulkan buffer wrapper class
 * -------------------------------------------------------------------------- */
class Buffer
{
public:
    // Constructs the buffer.
    Buffer(const VkPhysicalDevice& physicalDevice,
           const VkDevice& logicalDevice);

    // Sets and gets the size in bytes.
    Buffer& setSize(VkDeviceSize size);
    VkDeviceSize size() const;

    // Sets and gets the usage.
    Buffer& setUsage(VkBufferUsageFlagBits usage);
    VkBufferUsageFlagBits usage() const;

    // Sets and gets the sharing mode. By default the buffer is  not shaded
    // and is an exclusive to queue that uses it. If the sharing is
    // VK_SHARING_MODE_CONCURRENT then also the queue indicies needs to be
    // set. If the sharing is VK_SHARING_MODE_EXCLUSIVE then queue family
    // indices can be an empty vector.
    Buffer& setSharingMode(VkSharingMode mode);
    VkSharingMode shardingMode() const;

    // Sets and gets the queue family indices. An empty vector (the default)
    // is valid if the sharing mode is VK_SHARING_MODE_EXCLUSIVE (the default).
    Buffer& setQueueFamilyIndices(const std::vector<uint32_t>& indices);
    std::vector<uint32_t> queueFamilyIndices() const;

    // Sets and gets the buffer memory properties
    Buffer& setMemoryProperties(VkMemoryPropertyFlags properties);
    VkMemoryPropertyFlags memoryProperties() const;

    // Creates and destroys the buffer
    bool create();
    void destroy();

    // Returns true if the buffer handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the buffer handle.
    VkBuffer handle() const;

    // Maps and unmaps the buffer data.
    void* map();
    void* map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags = 0);
    void unmap();

    // Copies data into host visible buffer. This requires that buffer memory
    // properties has a host visible flag set. Data is copied immediately if
    // the memory properties has host coherent flag set.
    void copyHostVisible(const void* data, size_t size, VkDeviceSize offset = 0);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
