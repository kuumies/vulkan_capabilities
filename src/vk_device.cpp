/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Device class
 * ---------------------------------------------------------------- */

#include "vk_device.h"

/* ---------------------------------------------------------------- */

#include <iostream>

/* ---------------------------------------------------------------- */

#include "vk_physical_device.h"
#include "vk_surface.h"

namespace kuu
{
namespace vk
{
namespace
{

/* ---------------------------------------------------------------- */

std::vector<VkQueueFamilyProperties> queueFamilyProperties(
    const VkPhysicalDevice& physicalDevice)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCount,
        nullptr);

    if (queueFamilyCount == 0)
    {
        std::cerr << __FUNCTION__
                  << ": no queue families"
                  << std::endl;
        return std::vector<VkQueueFamilyProperties>();
    }

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCount,
        queueFamilies.data());

    return queueFamilies;
}

/* ---------------------------------------------------------------- */

int findGraphicsQueueFamilyIndex(
    const std::vector<VkQueueFamilyProperties> queueFamilies)
{
    int index = -1;
    for(int i = 0; i < queueFamilies.size(); ++i)
    {
        const VkQueueFamilyProperties& props = queueFamilies[i];
        if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            index = i;
    }
    return index;
}

/* ---------------------------------------------------------------- */

int findPresentQueueFamilyIndex(
    const std::vector<VkQueueFamilyProperties> queueFamilies,
    const VkPhysicalDevice& device,
    const VkSurfaceKHR& surface)
{
    int index = -1;
    for(int i = 0; i < queueFamilies.size(); ++i)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device, i, surface, &presentSupport);

        if (presentSupport)
            index = i;
    }
    return index;
}

} // anonymous namespace

/* ---------------------------------------------------------------- */

struct Device::Data
{
    Data(const PhysicalDevice& physicalDevice,
         const std::vector<QueueParameters>& params,
         const std::vector<std::string>& extensions)
    {
        const std::vector<VkQueueFamilyProperties> queueFamilies =
            queueFamilyProperties(physicalDevice.handle());

        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        for (const QueueParameters& param : params)
        {
            uint32_t queueFamilyIndex = 0;
            switch(param.type)
            {
                case Queue::Type::Graphics:
                    queueFamilyIndex =
                        findGraphicsQueueFamilyIndex(queueFamilies);
                    break;
                case Queue::Type::Presentation:
                {
                    Surface surf = *param.surface;
                    queueFamilyIndex =
                        findPresentQueueFamilyIndex(
                            queueFamilies,
                            physicalDevice.handle(),
                            surf.handle());
                    break;
                }
            }

            VkDeviceQueueCreateInfo queueInfo;
            queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndex;
            queueInfo.queueCount       = param.count;
            queueInfo.pQueuePriorities = &param.priority;
            queueInfo.pNext            = nullptr;
            queueInfo.flags            = 0;

            queueInfos.push_back(queueInfo);
        }

        std::vector<const char*> extensionNamesStr;
        for (auto& extension : extensions)
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
            physicalDevice.handle(), &deviceInfo,
            nullptr, &device);
        if (result != VK_SUCCESS)
            std::cerr << __FUNCTION__
                      << ": failed to create logical device"
                      << std::endl;

        for (int q = 0; q < queueInfos.size(); ++q)
        {
            queues.push_back(Queue(device,
                                   queueInfos[q].queueFamilyIndex,
                                   params[q].type));
        }
    }

    ~Data()
    {
        vkDestroyDevice(device, nullptr);
    }

    VkDevice device;
    std::vector<Queue> queues;
    bool valid = false;
};

/* ---------------------------------------------------------------- */

Device::Device(const PhysicalDevice& physicalDevice,
               const std::vector<QueueParameters>& params,
               const std::vector<std::string>& extensions)
    : d(std::make_shared<Data>(physicalDevice, params, extensions))
{}

/* ---------------------------------------------------------------- */

bool Device::isValid() const
{ return d->valid; }

/* ---------------------------------------------------------------- */

VkDevice Device::handle() const
{ return d->device; }

/* ---------------------------------------------------------------- */

std::vector<Queue> Device::queues() const
{ return d->queues; }

Queue Device::queue(Queue::Type type) const
{
    for (Queue q : d->queues)
        if (q.type() == type)
            return q;

}

/* ---------------------------------------------------------------- */

SwapChain Device::createSwapChain(const Surface& surface) const
{
    return SwapChain(*this, surface);
}

} // namespace vk
} // namespace kuu
