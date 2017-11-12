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
    // Constructs the command buffer.
    CommandBuffer(const Device& device,
                  const Queue& graphicsQueue,
                  const int count);

    // Returns true if the command buffer is valid. Command buffer
    // is valid if the command buffer pool and comman buffer 
    // creation succeeded.
    bool isValid() const;

    // Returns the buffer count.
    int bufferCount() const;
    // Returns the buffer.
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
