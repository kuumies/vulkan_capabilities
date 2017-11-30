/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Renderer class.
 * -------------------------------------------------------------------------- */

#include "vk_renderer.h"
#include <iostream>
#include "vk_helper.h"
#include "vk_logical_device.h"
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

namespace
{

std::vector<VkQueueFamilyProperties> getQueueFamilies(
    const VkPhysicalDevice& physicalDevice)
{
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,            // [in]  physical device handle
        &queueFamilyPropertyCount, // [out] queue family property count
        NULL);                     // [in]  properties, NULL to get count

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,                // [in]  physical device handle
        &queueFamilyPropertyCount,     // [in]  queue family property count
        queueFamilyProperties.data()); // [out] queue family properties

    return queueFamilyProperties;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct Renderer::Impl
{
    Impl(const VkPhysicalDevice& physicalDevice,
         const VkSurfaceKHR& surface)
        : physicalDevice(physicalDevice)
        , surface(surface)
        , device(physicalDevice)
    {}

    ~Impl()
    {
        destroy();
    }

    bool create()
    {
        if (!createLogicalDevice())
            return false;

        return true;
    }

    bool createLogicalDevice()
    {
        // Get the queue family indices for graphics and presentation queues
        auto queueFamilies = getQueueFamilies(physicalDevice);
        const int graphics     = helper::findQueueFamilyIndex(
            VK_QUEUE_GRAPHICS_BIT,
            queueFamilies);

        if (graphics == -1)
            return false;

        const int presentation = helper::findPresentationQueueFamilyIndex(
            physicalDevice,
            surface,
            queueFamilies,
            { graphics });

        if (presentation == -1)
            return false;

        graphicsFamilyIndex     = graphics;
        presentationFamilyIndex = presentation;

        vk::LogicalDevice logicalDevice(physicalDevice);
        logicalDevice.setExtensions( { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
        logicalDevice.addQueueFamily(graphicsFamilyIndex,     1, 1.0f);
        logicalDevice.addQueueFamily(presentationFamilyIndex, 1, 1.0f);
        return logicalDevice.create();
    }

    void destroy()
    {
        device.destroy();
    }

    // From user.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR surface            = VK_NULL_HANDLE;

    // Queue family indices
    uint32_t graphicsFamilyIndex;
    uint32_t presentationFamilyIndex;

    // Device.
    LogicalDevice device;
};

/* -------------------------------------------------------------------------- */

Renderer::Renderer(const VkPhysicalDevice& physicalDevice,
                   const VkSurfaceKHR& surface)
    : impl(std::make_shared<Impl>(physicalDevice, surface))
{}

bool Renderer::create()
{
    if (!isValid())
        return impl->create();
    return isValid();
}

bool Renderer::destroy()
{
    if (isValid())
        impl->destroy();
    return true;
}

bool Renderer::isValid() const
{ return true; }


} // namespace vk
} // namespace kuu
