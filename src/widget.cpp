/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Widget class
 * ---------------------------------------------------------------- */

#include "widget.h"

#include <assert.h>
#include <iostream>
#include <QWindow>
#include <qpa/qplatformnativeinterface.h>
#include <QGuiApplication>
#include <vulkan/vulkan.h>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace kuu
{
namespace vk
{
namespace
{

#ifdef _WIN32

/* ---------------------------------------------------------------- *
   Define surface create flags.
 * ---------------------------------------------------------------- */
typedef VkFlags VkWin32SurfaceCreateFlagsKHR;

/* ---------------------------------------------------------------- *
   Define surface create info.
 * ---------------------------------------------------------------- */
typedef struct VkWin32SurfaceCreateInfoKHR
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkWin32SurfaceCreateFlagsKHR    flags;
    HINSTANCE                       hinstance;
    HWND                            hwnd;
} VkWin32SurfaceCreateInfoKHR;

/* ---------------------------------------------------------------- *
   Define surface creation function for windows.
 * ---------------------------------------------------------------- */
typedef VkResult (APIENTRY *PFN_vkCreateWin32SurfaceKHR)(
    VkInstance,
    const VkWin32SurfaceCreateInfoKHR*,
    const VkAllocationCallbacks*,
    VkSurfaceKHR*);

/* ---------------------------------------------------------------- *
   Creates the Windows Vulkan surface. If the creation fails then
   a VK_NULL_HANDLE is returned.
 * ---------------------------------------------------------------- */
VkSurfaceKHR createWindowsSurface(
    const VkInstance& instance,
    const HWND& handle)
{
    // Fill the Windows surface create info
    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext     = NULL; // Needs to be null
    createInfo.flags     = 0;    // Needs to be 0
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd      = handle;

    // Get the function pointer.
    auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)
        vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    if (!CreateWin32SurfaceKHR)
    {
        std::cerr << __FUNCTION__
                  << ": failed to get function pointer to surface "
                  << "creation function."
                  << std::endl;

        return VK_NULL_HANDLE;
    }

    // Create the surface.
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    const VkResult result = CreateWin32SurfaceKHR(
        instance,
        &createInfo,
        nullptr,
        &surface);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create Windows Vulkan surface."
                  << std::endl;

        return VK_NULL_HANDLE;
    }

    return surface;
}

#endif

} // anonymous namespace

struct Widget::Data
{
    Data(const VkInstance& instance, const HWND& winId)
        : instance(instance)
    {
        // Instance handle needs to be valid.
        assert(instance !=  VK_NULL_HANDLE);

        // Creates the surface
    #ifdef _WIN32
        surface = createWindowsSurface(instance, winId);
    #endif
    }

    ~Data()
    {
        // Destroys the surface.
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    // Instance handle.
    VkInstance instance = VK_NULL_HANDLE;
    // Surface handle.
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};

Widget::Widget(const VkInstance& instance, QWidget* parent)
    : QWidget(parent)
    , d(std::make_shared<Data>(instance, (HWND) winId()))
{
    // Disable over-painting the Vulkan surface
    setUpdatesEnabled(false);
}

bool Widget::isValid() const
{ return d->surface == VK_NULL_HANDLE; }

VkSurfaceKHR Widget::surface() const
{ return d->surface; }

void Widget::paintEvent(QPaintEvent* /*e*/)
{}

} // namespace vk
} // namespace kuu
