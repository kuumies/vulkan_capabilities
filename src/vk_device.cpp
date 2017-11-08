/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Device class
 * ---------------------------------------------------------------- */

#include "vk_device.h"

/* ---------------------------------------------------------------- */

#include <iostream>

/* ---------------------------------------------------------------- */

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct Device::Data
{
    VkPhysicalDevice physicalDevice;
    VkDevice device;
};

/* ---------------------------------------------------------------- */

Device::Device(VkPhysicalDevice physicalDevice)
    : d(std::make_shared<Data>())
{
    d->physicalDevice = physicalDevice;
}

/* ---------------------------------------------------------------- */

VkDevice Device::handle() const
{ return d->device; }

/* ---------------------------------------------------------------- */

bool Device::create(
        std::vector<Queue> queues,
        const std::vector<std::string>& physicalDeviceExtensions)
{
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    for (const Queue& q : queues)
        queueInfos.push_back(q.deviceQueue());

    std::vector<const char*> extensionNamesStr;
    for (auto& extension : physicalDeviceExtensions)
        extensionNamesStr.push_back(extension.c_str());

    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pQueueCreateInfos       = queueInfos.data();
    deviceInfo.queueCreateInfoCount    = uint32_t(queueInfos.size());
    deviceInfo.pEnabledFeatures        = &deviceFeatures;
    deviceInfo.enabledExtensionCount   = uint32_t(extensionNamesStr.size());
    deviceInfo.ppEnabledExtensionNames = extensionNamesStr.data();
    deviceInfo.enabledLayerCount       = 0;

    // Create the logical device.
    VkResult result = vkCreateDevice(
        d->physicalDevice, &deviceInfo,
        nullptr, &d->device);
    if (result != VK_SUCCESS)
        std::cerr << __FUNCTION__
                  << ": failed to create logical device"
                  << std::endl;

    for (Queue& q : queues)
    {
        VkQueue queue;
        vkGetDeviceQueue(d->device, q.familyIndex(), 0, &queue);
        q.setHandle(queue);
    }

    return result == VK_SUCCESS;
}

/* ---------------------------------------------------------------- */

void Device::destroy()
{
    vkDestroyDevice(d->device, nullptr);
}

} // namespace vk
} // namespace kuu
