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
   A command buffer
 * ---------------------------------------------------------------- */
class CommandBuffer
{
public:
    // Constructs the command buffer.
    CommandBuffer(const Device& device,
                  const Queue& graphicsQueue,
                  const int count);

    // Returns true if the command buffer is valid.
    bool isValid() const;

    // Returns the buffer count.
    int bufferCount() const;
    // Returns the buffer.
    VkCommandBuffer buffer(int bufferIndex) const;

    void begin(int buffer);
    void end(int buffer);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
