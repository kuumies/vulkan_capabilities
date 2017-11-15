/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Widget class
 * ---------------------------------------------------------------- */

#include "vk_widget.h"
#include <assert.h>
#include <iostream>
#include <vulkan/vulkan.h>
#include "vk_windows.h"

namespace kuu
{
namespace vk
{
namespace
{

#ifdef _WIN32


/* ---------------------------------------------------------------- *
   Creates the Windows Vulkan surface. If the creation fails then
   a VK_NULL_HANDLE is returned.
 * ---------------------------------------------------------------- */
VkSurfaceKHR createWindowsSurface(
    const VkInstance& instance,
    const HWND& handle)
{
    using namespace windows;

    // Fill the Windows surface create info
    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext     = NULL; // Needs to be null
    createInfo.flags     = 0;    // Needs to be 0
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd      = handle;

    // Create the surface.
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    const VkResult result = vkCreateWin32SurfaceKHR(
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
