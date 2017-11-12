/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::CommandBuffer class
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

/* ---------------------------------------------------------------- */

class Device;
class Queue;

/* ---------------------------------------------------------------- *
   A command buffers where user can record Vulkan commands. The  
   count of buffers created is set during construction. The count
   should be the same as the count of images in swap chain.

   This will also create a command pool for buffers.
 * ---------------------------------------------------------------- */
class CommandBuffer
{
public:
    // Constructs command buffer for the given in queue type.
    // Queue must gave been created from the same logical device
    // as the argument. Count must be one or more.
    CommandBuffer(const Device& device,
                  const Queue& queue,
                  const int count);

    // Returns true if the command buffer is valid. Command buffer
    // is valid if the command buffer pool creation succeeded and
    // there is one or more command buffers created.
    bool isValid() const;

    // Returns the buffer count.
    int bufferCount() const;
    // Returns the buffer. If the bufferIndex is less than zero or
    // larger than the count of buffers then a null buffer is
    // returned.
    VkCommandBuffer buffer(int bufferIndex) const;

    // Begins recording commands into ith buffer.
    void begin(int buffer);
    // Ends recording commands into ith buffer.
    void end(int buffer);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
