/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of Vulkan sync related classes.
 * -------------------------------------------------------------------------- */

#include "vk_sync.h"
#include <iostream>
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct Semaphore::Impl
{
    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {
        VkSemaphoreCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = NULL;
        info.flags = 0;

        const VkResult result = vkCreateSemaphore(
            logicalDevice,
            &info, NULL,
            &semaphore);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": semaphore creation failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;
            return false;
        }

        return true;
    }

    void destroy()
    {
        vkDestroySemaphore(
            logicalDevice,
            semaphore,
            NULL);

        semaphore     = VK_NULL_HANDLE;
        logicalDevice = VK_NULL_HANDLE;
    }

    bool isValid() const
    {
        return semaphore != VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkSemaphore semaphore = VK_NULL_HANDLE;
};

/* -------------------------------------------------------------------------- */

Semaphore::Semaphore(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
}

bool Semaphore::create()
{
    if (!isValid())
        return impl->create();
    return true;
}

void Semaphore::destroy()
{
    if (isValid())
        impl->destroy();
}

bool Semaphore::isValid() const
{ return impl->isValid(); }

VkSemaphore Semaphore::handle() const
{ return impl->semaphore; }

} // namespace vk
} // namespace kuu
