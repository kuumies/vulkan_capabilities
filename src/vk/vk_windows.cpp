/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::windows namespace.
 * -------------------------------------------------------------------------- */
 
#include "vk_windows.h"
#include <iostream>

namespace kuu
{
namespace vk
{
namespace windows
{

#ifdef _WIN32

/* -------------------------------------------------------------------------- */

typedef VkBool32 (APIENTRY *PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)(
    VkPhysicalDevice,
    uint32_t);

typedef VkResult (APIENTRY *PFN_vkCreateWin32SurfaceKHR)(
    VkInstance,
    const VkWin32SurfaceCreateInfoKHR*,
    const VkAllocationCallbacks*,
    VkSurfaceKHR*);

/* -------------------------------------------------------------------------- */

VkBool32 vkGetPhysicalDeviceWin32PresentationSupportKHR(
    VkInstance instance,
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex)
{
    // Get the function pointer.
    auto fun = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)
        vkGetInstanceProcAddr(
            instance,
            "vkGetPhysicalDeviceWin32PresentationSupportKHR");
    if (!fun)
    {
        std::cerr << __FUNCTION__
                  << ": failed to get vkGetPhysicalDeviceWin32Presentation"
                  << "SupportKHR function pointer"
                  << std::endl;

        return VK_FALSE;
    }

    return fun(physicalDevice, queueFamilyIndex);
}

/* -------------------------------------------------------------------------- */

VkResult vkCreateWin32SurfaceKHR(
    VkInstance instance,
    const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface)
{
    // Get the function pointer.
    auto fun = (PFN_vkCreateWin32SurfaceKHR)
        vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    if (!fun)
    {
        std::cerr << __FUNCTION__
                  << ": failed to get vkGetPhysicalDeviceWin32Presentation"
                  << "SupportKHR function pointer"
                  << std::endl;

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    return fun(instance, pCreateInfo, pAllocator, pSurface);
}

/* -------------------------------------------------------------------------- *
   Implementation of vkCreateDebugReportCallback. This is an extension
   function whose adress needs to be retrieved.
 * -------------------------------------------------------------------------- */
VkResult vkCreateDebugReportCallback(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugReportCallbackEXT* pCallback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT)
        vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugReportCallbackEXT");

    VkResult result = VK_ERROR_INITIALIZATION_FAILED;
    if (func)
        result = func(instance, pCreateInfo, pAllocator, pCallback);
    return result;
}

/* -------------------------------------------------------------------------- *
   Implementation of vkDestroyDebugReportCallback. This is an extension
   function whose adress needs to be retrieved.
 * -------------------------------------------------------------------------- */
void vkDestroyDebugReportCallback(
    VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)
        vkGetInstanceProcAddr(instance,
                              "vkDestroyDebugReportCallbackEXT");
    if (func)
        func(instance, callback, pAllocator);
}

#endif

} // namespace windows
} // namespace vk
} // namespace kuu
