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
        destroy();
    }

    void create()
    {
        VkResult result;

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": semaphore creation failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;

            return;
        }
    }

    void destroy()
    {
        semaphore = VK_NULL_HANDLE;
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

void Semaphore::create()
{
    if (!isValid())
        impl->create();
}

void Semaphore::destroy()
{
    if (isValid())
        impl->destroy();
}

bool Semaphore::isValid() const
{ return impl->semaphore != VK_NULL_HANDLE; }

VkSemaphore Semaphore::handle() const
{ return impl->semaphore; }

} // namespace vk
} // namespace kuu
