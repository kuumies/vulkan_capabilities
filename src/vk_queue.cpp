/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Queue class
 * ---------------------------------------------------------------- */

#include "vk_queue.h"

/* ---------------------------------------------------------------- */

#include <iostream>
#include <vector>

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct Queue::Data
{
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkQueue queue;
    Type type;
    uint32_t queueFamilyIndex;
    uint32_t count;
    float priority;
};

/* ---------------------------------------------------------------- */

Queue::Queue(VkPhysicalDevice physicalDevice,
             VkSurfaceKHR surface,
             Type type)
    : d(std::make_shared<Data>())
{
    d->physicalDevice = physicalDevice;
    d->surface        = surface;
    d->type           = type;
}

/* ---------------------------------------------------------------- */

VkDeviceQueueCreateInfo Queue::deviceQueue() const
{
    VkDeviceQueueCreateInfo queueInfo;

    queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = d->queueFamilyIndex;
    queueInfo.queueCount       = d->count;
    queueInfo.pQueuePriorities = &d->priority;
    queueInfo.pNext            = nullptr;
    queueInfo.flags            = 0;

    return queueInfo;
}

/* ---------------------------------------------------------------- */

uint32_t Queue::familyIndex() const
{ return d->queueFamilyIndex; }

/* ---------------------------------------------------------------- */

VkQueue Queue::handle() const
{ return d->queue; }

/* ---------------------------------------------------------------- */

bool Queue::create(uint32_t count, float priority)
{
    // Get the queue family count.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        d->physicalDevice,
        &queueFamilyCount,
        nullptr);

    if (queueFamilyCount == 0)
    {
        std::cerr << __FUNCTION__
                  << ": no queue families"
                  << std::endl;
        return false;
    }

    // Get the queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        d->physicalDevice,
        &queueFamilyCount,
        queueFamilies.data());

    // Find the queue family index.
    bool found = false;
    for (int i = 0; i < queueFamilies.size(); ++i)
    {
        const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
        if (queueFamily.queueCount <= 0)
            continue;

        switch(d->type)
        {
            case Type::Graphics:
            {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    d->queueFamilyIndex = i;
                    found = true;
                }
                break;
            }

            case Type::Presentation:
            {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(
                    d->physicalDevice, i,
                    d->surface, &presentSupport);

                if (presentSupport)
                {
                    d->queueFamilyIndex = i;
                    found = true;
                }

                break;
            }
        }
    }

    if (found)
    {
        d->count    = count;
        d->priority = priority;
    }
    else
        std::cerr << __FUNCTION__
                  << ": failed to find queue family index"
                  << std::endl;

    return found;
}

/* ---------------------------------------------------------------- */

void Queue::destroy()
{}

/* ---------------------------------------------------------------- */

void Queue::setHandle(VkQueue handle)
{
    d->queue = handle;
}

} // namespace vk
} // namespace kuu
