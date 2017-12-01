/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of Vulkan command related classes.
 * -------------------------------------------------------------------------- */

#include "vk_command.h"
#include <iostream>
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct CommandPool::Impl
{
    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {
        VkCommandPoolCreateInfo info;
        info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext            = NULL;
        info.flags            = 0;
        info.queueFamilyIndex = queueFamilyIndex;

        const VkResult result =
            vkCreateCommandPool(
                logicalDevice,
                &info,
                NULL,
                &commandPool);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": command pool creation failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;
            return false;
        }

        return true;
    }

    void destroy()
    {
        vkDestroyCommandPool(
            logicalDevice,
            commandPool,
            NULL);

        commandPool = VK_NULL_HANDLE;
    }

    bool isValid() const
    {
        return commandPool != VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkCommandPool  commandPool = VK_NULL_HANDLE;

    // From user.
    uint32_t queueFamilyIndex;
};

/* -------------------------------------------------------------------------- */

CommandPool::CommandPool(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
}

CommandPool& CommandPool::setQueueFamilyIndex(uint32_t queueFamilyIndex)
{
    impl->queueFamilyIndex = queueFamilyIndex;
    return *this;
}

uint32_t CommandPool::queueFamilyIndex() const
{ return impl->queueFamilyIndex; }

bool CommandPool::create()
{
    if (!isValid())
        return impl->create();
    return true;
}

void CommandPool::destroy()
{
    if (isValid())
        impl->destroy();
}

bool CommandPool::isValid() const
{ return impl->isValid(); }

VkCommandPool  CommandPool::handle() const
{ return impl->commandPool; }

std::vector<VkCommandBuffer> CommandPool::allocateBuffers(
        VkCommandBufferLevel level,
        uint32_t count)
{
    VkCommandBufferAllocateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext              = NULL;
    info.commandPool        = impl->commandPool;
    info.level              = level;
    info.commandBufferCount = count;

    std::vector<VkCommandBuffer> buffers(count);
    const VkResult result =
        vkAllocateCommandBuffers(
            impl->logicalDevice,
            &info, buffers.data());

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": command pool allocation failed as "
                  << vk::stringify::resultDesc(result)
                  << std::endl;
    }

    return buffers;
}

VkCommandBuffer CommandPool::allocateBuffer(VkCommandBufferLevel level)
{
    std::vector<VkCommandBuffer> out = allocateBuffers(level, 1);
    return out.size() > 0 ? out[0] : VK_NULL_HANDLE;
}

void CommandPool::freeBuffers(const std::vector<VkCommandBuffer> &buffers)
{
    vkFreeCommandBuffers(
        impl->logicalDevice,
        impl->commandPool,
        uint32_t(buffers.size()),
        buffers.data());
}

} // namespace vk
} // namespace kuu
