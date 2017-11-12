/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::CommandBuffer class
 * ---------------------------------------------------------------- */

#include "vk_command_buffer.h"

/* ---------------------------------------------------------------- */

#include <iostream>

/* ---------------------------------------------------------------- */

#include "vk_device.h"
#include "vk_queue.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct CommandBuffer::Data
{
    Data(const Device& device, 
         const Queue& queue,
         const int count)
        : device(device)
    {
        // Create the command pool info for the queue. Commands are
        // recorded only once and the executed multiple times on
        // the main loop (flags = 0).
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queue.familyIndex();
        poolInfo.flags            = 0;

        // Create command pool.
        VkResult result = vkCreateCommandPool(
            device.handle(),
            &poolInfo,
            nullptr,
            &commandPool);
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create vulkan command pool"
                      << std::endl;
            return;
        }

        // Each image in the swap chain needs to have its own
        // command buffer.
        commandBuffers.resize(count);

        // Create the command buffer allocation information.
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = uint32_t(commandBuffers.size());

        // Allocate the command buffers.
        result = vkAllocateCommandBuffers(
            device.handle(),
            &allocInfo,
            commandBuffers.data());
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to allocate command buffers"
                      << std::endl;

            return;
        }
    }

    ~Data()
    {
        // Destroys the command pool. This will also delete the
        // created buffers.
        vkDestroyCommandPool(device.handle(), commandPool, nullptr);
    }

    // Logical device.
    Device device;
    // Pool for commands.
    VkCommandPool commandPool = VkCommandPool();
    // Command buffers.
    std::vector<VkCommandBuffer> commandBuffers;
};

/* ---------------------------------------------------------------- */

CommandBuffer::CommandBuffer(
        const Device& device,
        const Queue& queue,
        const int count)
    : d(std::make_shared<Data>(device, queue, count))
{}

/* ---------------------------------------------------------------- */

bool CommandBuffer::isValid() const
{
    return d->commandPool != VkCommandPool() &&
           d->commandBuffers.size() > 0;
}

/* ---------------------------------------------------------------- */

int CommandBuffer::bufferCount() const
{ return int(d->commandBuffers.size()); }

/* ---------------------------------------------------------------- */

VkCommandBuffer CommandBuffer::buffer(int bufferIndex) const
{
    if (bufferIndex < 0 || bufferIndex >= d->commandBuffers.size())
        return VkCommandBuffer();

    return d->commandBuffers[bufferIndex];
}

/* ---------------------------------------------------------------- */

void CommandBuffer::begin(int buffer)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    // Begin recording commands
    vkBeginCommandBuffer(d->commandBuffers[buffer], &beginInfo);
}

/* ---------------------------------------------------------------- */

void CommandBuffer::end(int buffer)
{
    VkResult result = vkEndCommandBuffer(d->commandBuffers[buffer]);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to end command buffer"
                  << std::endl;
    }
}

} // namespace vk
} // namespace kuu
