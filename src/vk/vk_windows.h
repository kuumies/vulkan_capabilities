/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::windows namespace.
 * -------------------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

#ifdef _WIN32
    #include <Windows.h>
#endif

namespace kuu
{
namespace vk
{
namespace windows
{

#ifdef _WIN32

/* -------------------------------------------------------------------------- *
   Below is declarations of Windows-OS related functions, flags and
   structures that are need to create a surface and swap chain.
 * -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */

// Takes instance as a extra argument compared to spec. This is needed to
// find the correct function pointer. If the function fails to locate the
// correct function pointer then an error message is printed into cerr and
// VK_FALSE is returned.
VkBool32 vkGetPhysicalDeviceWin32PresentationSupportKHR(
    VkInstance                                  instance,
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex);

/* -------------------------------------------------------------------------- */

typedef VkFlags VkWin32SurfaceCreateFlagsKHR;

typedef struct VkWin32SurfaceCreateInfoKHR
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkWin32SurfaceCreateFlagsKHR    flags;
    HINSTANCE                       hinstance;
    HWND                            hwnd;
} VkWin32SurfaceCreateInfoKHR;

// If the function fails to locate the correct function pointer then an error
// message is printed into cerr and VK_ERROR_EXTENSION_NOT_PRESENT is returned
// (surface creation is an extension to Vulkan).
VkResult vkCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);

#endif

} // namespace windows
} // namespace vk
} // namespace kuu
