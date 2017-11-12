/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Buffer class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- *
   A vulkan buffer.
 * ---------------------------------------------------------------- */
class Buffer
{
public:
    // Constructs the buffer.
    Buffer(VkDevice device, VkPhysicalDevice physicalDevice);

    // Returns the vulkan handle. Handle is valid only after
    // the buffer is created.
    VkBuffer handle() const;

    // Creates the buffer of given in size and writes the data in
    // there. Old existing buffer is destroyd beforehand if it
    // exisits.
    void create(VkDeviceSize size, VkBufferUsageFlagBits usage, void* data);

    // Destroys the buffer.
    void destroy();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
