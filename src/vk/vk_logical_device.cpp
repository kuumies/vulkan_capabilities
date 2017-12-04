/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::LogicalDevice class
 * -------------------------------------------------------------------------- */

#include "vk_logical_device.h"

/* -------------------------------------------------------------------------- */

#include <algorithm>
#include <iostream>

/* -------------------------------------------------------------------------- */

#include "vk_instance.h"
#include "vk_stringify.h"

#ifdef _WIN32
    #include "vk_windows.h"
#endif

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct LogicalDevice::Impl
{
    Impl(const VkPhysicalDevice& physicalDevice)
        : physicalDevice(physicalDevice)
    {
        features.samplerAnisotropy    = VK_TRUE;
        features.fillModeNonSolid     = VK_TRUE;
    }

    ~Impl()
    {
        if (isValid())
            destroy();
    }

    void create()
    {
        // Fill the queue create infos.
        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        for (size_t i = 0; i < queueFamilyParams.size(); ++i)
        {
            const QueueFamilyParams& params = queueFamilyParams[i];

            VkStructureType type = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            VkDeviceQueueCreateInfo queueInfo;
            queueInfo.sType            = type;                    // Type of the struct.
            queueInfo.queueFamilyIndex = params.queueFamilyIndex; // Queue family index.
            queueInfo.queueCount       = params.queueCount;       // Count of queues.
            queueInfo.pQueuePriorities = &params.priority;        // Priority of the queue.
            queueInfo.pNext            = NULL;                    // No extension usage
            queueInfo.flags            = 0;                       // Must be 0.

            queueInfos.push_back(queueInfo);
        }

        // Extensions
        std::vector<const char*> extensionNamesStr;
        for (auto& extension : extensions)
            extensionNamesStr.push_back(extension.c_str());

        // Layers
        std::vector<const char*> layerNamesStr;
        for (auto& layer : layers)
            layerNamesStr.push_back(layer.c_str());

        // Fill create info
        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; // Type of the struct.
        deviceInfo.pNext                   = NULL;                                 // Extension chain.
        deviceInfo.flags                   = 0;                                    // Must be 0.
        deviceInfo.queueCreateInfoCount    = uint32_t(queueInfos.size());          // Queue info count.
        deviceInfo.pQueueCreateInfos       = queueInfos.data();                    // Queue infos.
        deviceInfo.enabledLayerCount       = uint32_t(layerNamesStr.size());       // Layer count.
        deviceInfo.ppEnabledLayerNames     = layerNamesStr.data();                 // Layer names.
        deviceInfo.enabledExtensionCount   = uint32_t(extensionNamesStr.size());   // Extension count.
        deviceInfo.ppEnabledExtensionNames = extensionNamesStr.data();             // Extension names.
        deviceInfo.pEnabledFeatures        = &features;                            // Features to enable.
        
        // Create the logical device.
        const VkResult result = vkCreateDevice(
            physicalDevice,     // [in]  physical device handle
            &deviceInfo,        // [in]  device info
            NULL,               // [in]  allocator
            &logicalDevice);    // [out] handle to logical device opaque object

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create logical device"
                      << std::endl;
        }
    }

    void destroy()
    {
        vkDestroyDevice(
            logicalDevice, // [in] logical device
            NULL);         // [in] allocator

        logicalDevice = VK_NULL_HANDLE;
    }

    bool isValid() const
    {
        return logicalDevice != VK_NULL_HANDLE;
    }

    // Parent
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    // Child
    VkDevice logicalDevice          = VK_NULL_HANDLE;

    // From user
    std::vector<QueueFamilyParams> queueFamilyParams;
    std::vector<std::string> extensions;
    std::vector<std::string> layers;
    VkPhysicalDeviceFeatures features = {};
};

LogicalDevice::LogicalDevice(const VkPhysicalDevice& physicalDevice)
    : impl(std::make_shared<Impl>(physicalDevice))
{}

LogicalDevice& LogicalDevice::setExtensions(
    const std::vector<std::string>& extensions)
{
    impl->extensions = extensions;
    return *this;
}

std::vector<std::string> LogicalDevice::extensions() const
{ return impl->extensions; }

LogicalDevice& LogicalDevice::setLayers(
    const std::vector<std::string>& layers)
{
    impl->layers = layers;
    return *this;
}

std::vector<std::string> LogicalDevice::layers() const
{ return impl->layers; }

LogicalDevice& LogicalDevice::setFeatures(
    const VkPhysicalDeviceFeatures& features)
{ 
    impl->features = features; 
    return *this;
}

VkPhysicalDeviceFeatures LogicalDevice::features() const
{ return impl->features; }

LogicalDevice& LogicalDevice::addQueueFamily(
        const QueueFamilyParams& queue)
{
    impl->queueFamilyParams.push_back(queue);
    return *this;
}

LogicalDevice& LogicalDevice::addQueueFamily(
    uint32_t queueFamilyIndex,
    uint32_t queueCount,
    float priority)
{
    return addQueueFamily(
        QueueFamilyParams({
            queueFamilyIndex, 
            queueCount, 
            priority}));
}

std::vector<LogicalDevice::QueueFamilyParams>
    LogicalDevice::queueFamilyParams() const
{ return impl->queueFamilyParams; }

bool LogicalDevice::create()
{
    if (!isValid()) 
        impl->create();
    return isValid();
}

void LogicalDevice::destroy()
{
    if (isValid())
        impl->destroy(); 
}

bool LogicalDevice::isValid() const
{ return impl->isValid(); }

VkDevice LogicalDevice::handle() const
{ return impl->logicalDevice; }

} // namespace vk
} // namespace kuu
