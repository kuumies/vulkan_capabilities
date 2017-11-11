/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Semaphore class
 * ---------------------------------------------------------------- */

#include "vk_semaphore.h"

/* ---------------------------------------------------------------- */

#include <iostream>

/* ---------------------------------------------------------------- */

#include "vk_device.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct Semaphore::Data
{
    Data(const Device& device)
        : device(device)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        // Create semaphore for rendering to know when an image in
        // swap chain is available
        VkResult result = vkCreateSemaphore(
            device.handle(),
            &semaphoreInfo,
            nullptr,
            &semaphore);
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create semaphore"
                      << std::endl;

            return;
        }

        valid = true;
    }

    ~Data()
    {
        vkDestroySemaphore(
            device.handle(),
            semaphore,
            nullptr);
    }

    Device device;
    VkSemaphore semaphore;
    bool valid = false;
};

/* ---------------------------------------------------------------- */

Semaphore::Semaphore(const Device& device)
    : d(std::make_shared<Data>(device))
{}

/* ---------------------------------------------------------------- */

bool Semaphore::isValid() const
{ return d->valid; }

/* ---------------------------------------------------------------- */

VkSemaphore Semaphore::handle() const
{ return d->semaphore; }

} // namespace vk
} // namespace kuu
