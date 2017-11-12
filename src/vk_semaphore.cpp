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
        // Fill the semaphore create info
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        // Create the semaphore.
        const VkResult result = vkCreateSemaphore(
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
    }

    ~Data()
    {
        // Destroy the semaphore.
        vkDestroySemaphore(
            device.handle(),
            semaphore,
            nullptr);
    }

    // Logical device.
    Device device;
    // Handle
    VkSemaphore semaphore = VkSemaphore();
};

/* ---------------------------------------------------------------- */

Semaphore::Semaphore(const Device& device)
    : d(std::make_shared<Data>(device))
{}

/* ---------------------------------------------------------------- */

bool Semaphore::isValid() const
{ return d->semaphore != VkSemaphore(); }

/* ---------------------------------------------------------------- */

VkSemaphore Semaphore::handle() const
{ return d->semaphore; }

/* ---------------------------------------------------------------- */

Semaphore::operator VkSemaphore() const
{ return d->semaphore; }

} // namespace vk
} // namespace kuu
