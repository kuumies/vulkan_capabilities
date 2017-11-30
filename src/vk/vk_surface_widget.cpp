/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::SurfaceWidget class
 * -------------------------------------------------------------------------- */

#include "vk_surface_widget.h"
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

/* -------------------------------------------------------------------------- *
   Creates the Windows Vulkan surface. If the creation fails then
   a VK_NULL_HANDLE is returned.
 * -------------------------------------------------------------------------- */
VkSurfaceKHR createWindowsSurface(
    const VkInstance& instance,
    const HWND& handle)
{
    using namespace vk::windows;

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

/* -------------------------------------------------------------------------- */

struct SurfaceWidget::Impl
{
    ~Impl()
    {
        destroy();
    }

    void create(const HWND& winId)
    {
        // Creates the surface
    #ifdef _WIN32
        surface = createWindowsSurface(instance, winId);
    #endif
    }

    void destroy()
    {
        vkDestroySurfaceKHR(
            instance,
            surface,
            NULL);

        surface = VK_NULL_HANDLE;
    }

    // Instance handle.
    VkInstance instance;
    // Surface handle.
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};

/* -------------------------------------------------------------------------- */

SurfaceWidget::SurfaceWidget(QWidget* parent)
    : QWidget(parent)
    , impl(std::make_shared<Impl>())
{
    // Disable over-painting the Vulkan surface
    setUpdatesEnabled(false);
}

SurfaceWidget& SurfaceWidget::setInstance(const VkInstance& instance)
{
    impl->instance = instance;
    return *this;
}

VkInstance SurfaceWidget::instance() const
{ return impl->instance; }

void SurfaceWidget::createSurface()
{
    if (!isValid())
        impl->create((HWND) winId());
}

void SurfaceWidget::destroySurface()
{
    if (isValid())
        impl->destroy();
}

bool SurfaceWidget::isValid() const
{ return impl->surface != VK_NULL_HANDLE; }

VkSurfaceKHR SurfaceWidget::handle() const
{ return impl->surface; }

void SurfaceWidget::paintEvent(QPaintEvent* /*e*/)
{}

} // namespace vk
} // namespace kuu
