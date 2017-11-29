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
        destroy();
    }

    void create()
    {
        VkResult result;

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": command pool creation failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;

            return;
        }
    }

    void destroy()
    {
        commandPool = VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkCommandPool  commandPool = VK_NULL_HANDLE;
};

/* -------------------------------------------------------------------------- */

CommandPool::CommandPool(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
}

void CommandPool::create()
{
    if (!isValid())
        impl->create();
}

void CommandPool::destroy()
{
    if (isValid())
        impl->destroy();
}

bool CommandPool::isValid() const
{ return impl->commandPool != VK_NULL_HANDLE; }

VkCommandPool  CommandPool::handle() const
{ return impl->commandPool; }

} // namespace vk
} // namespace kuu
