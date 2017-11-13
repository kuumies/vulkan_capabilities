/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Surface class
 * ---------------------------------------------------------------- */

#include "vk_surface.h"

/* ---------------------------------------------------------------- */

#include <iostream>
#include <glfw/glfw3.h>

/* ---------------------------------------------------------------- */

#include "vk_instance.h"

namespace kuu
{
namespace vk
{
namespace
{

/* ---------------------------------------------------------------- */

std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats(
    VkSurfaceKHR surface,
    VkPhysicalDevice physicalDevice)
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &formatCount,
            nullptr);

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    if (formatCount > 0)
    {
        surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice, surface,
            &formatCount, surfaceFormats.data());
    }

    return surfaceFormats;
}

VkSurfaceFormatKHR compatibleFormat(
    VkSurfaceKHR surface,
    VkPhysicalDevice device,
    VkSurfaceFormatKHR target)
{
    std::vector<VkSurfaceFormatKHR> formats =
        physicalDeviceSurfaceFormats(
            surface,
            device);

    // Zero format count should be impossible unless user created
    // the Vulkan instance with out the surface extentions.
    if (formats.size() == 0)
    {
        std::cerr << __FUNCTION__
                  << ": device does not support any surface format"
                  << std::endl;
        return VkSurfaceFormatKHR();
    }

    // ????
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        return { VK_FORMAT_B8G8R8A8_UNORM,
                 VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    for (const VkSurfaceFormatKHR& format : formats)
    {
        if (format.format     == target.format &&
            format.colorSpace == target.colorSpace)
        {
            return format;
        }
    }

    return formats[0];
}

} // anonymous namespace

/* ---------------------------------------------------------------- */

struct Surface::Data
{
    Data(const Instance& instance)
        : instance(instance)
        , valid(false)
        , format( { VK_FORMAT_B8G8R8A8_UNORM,
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } )
        , presentMode(VK_PRESENT_MODE_FIFO_KHR)
        , imageExtent( { 512,512 } )
        , swapChainImageCount(2)
    {}

    Data(const Instance& instance, GLFWwindow* window)
        : instance(instance)
        , valid(false)
        , format( { VK_FORMAT_B8G8R8A8_UNORM,
                    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } )
        , presentMode(VK_PRESENT_MODE_FIFO_KHR)
        , imageExtent( { 512,512 } )
        , swapChainImageCount(2)
    {
        const VkResult result = glfwCreateWindowSurface(
            instance.handle(),
            window,
            nullptr,
            &surface);

        if (result == VK_SUCCESS)
        {
            valid = true;
        }
        else
        {
            std::cerr << __FUNCTION__
                      << ": failed to create surface for a window"
                      << std::endl;
            valid = false;
        }
    }

     ~Data()
    {
        vkDestroySurfaceKHR(instance.handle(), surface, nullptr);
    }

    Instance instance;
    VkSurfaceKHR surface;
    bool valid = false;

    VkSurfaceFormatKHR format;
    VkPresentModeKHR presentMode;
    VkExtent2D imageExtent;
    uint32_t swapChainImageCount;
};

/* ---------------------------------------------------------------- */

Surface::Surface(const Instance& instance, GLFWwindow* window)
    : d(std::make_shared<Data>(instance, window))
{}

Surface::Surface(const Instance &instance, VkSurfaceKHR s)
    : d(std::make_shared<Data>(instance))
{
    d->surface = s;
    d->valid = true;
}

/* ---------------------------------------------------------------- */

bool Surface::isValid() const
{ return d->valid; }

/* ---------------------------------------------------------------- */

VkSurfaceKHR Surface::handle() const
{ return d->surface; }

/* ---------------------------------------------------------------- */

Surface& Surface::setFormat(VkSurfaceFormatKHR format)
{
    d->format = format;
    return *this;
}

/* ---------------------------------------------------------------- */

VkSurfaceFormatKHR Surface::format() const
{ return d->format; }

/* ---------------------------------------------------------------- */

Surface& Surface::setPresentMode(VkPresentModeKHR presentMode)
{
    d->presentMode = presentMode;
    return *this;
}

/* ---------------------------------------------------------------- */

VkPresentModeKHR Surface::presentMode() const
{ return d->presentMode; }

/* ---------------------------------------------------------------- */

Surface& Surface::setImageExtent(VkExtent2D imageExtent)
{
    d->imageExtent = imageExtent;
    return *this;
}

/* ---------------------------------------------------------------- */

VkExtent2D Surface::imageExtent() const
{ return d->imageExtent; }

/* ---------------------------------------------------------------- */

Surface& Surface::setSwapChainImageCount(uint32_t swapChainImageCount)
{
    d->swapChainImageCount = swapChainImageCount;
    return *this;
}

/* ---------------------------------------------------------------- */

uint32_t Surface::swapChainImageCount() const
{ return d->swapChainImageCount; }

/* ---------------------------------------------------------------- */

bool Surface::isCompatibleWith(const PhysicalDevice& device) const
{
    if (!device.isSurfaceSupported(
        *this,
        d->format.format,
        d->format.colorSpace))
    {
        return false;
    }

    if (!device.isPresentModeSupported(*this, d->presentMode))
        return false;
    if (!device.isImageExtentSupported(*this, d->imageExtent))
        return false;

    if (!device.isSwapChainImageCountSupported(
        *this,
        d->swapChainImageCount))
    {
        return false;
    }

    return true;
}

/* ---------------------------------------------------------------- */

bool Surface::setCompatibleWith(const PhysicalDevice& device)
{
    // Select surface format
    if (!device.isSurfaceSupported(
        *this,
        d->format.format,
        d->format.colorSpace))
    {
        d->format = compatibleFormat(
            d->surface,
            device.handle(),
            d->format);
    }

    return true;
}

/* ---------------------------------------------------------------- */

bool Surface::areExtensionsSupported()
{
    for (const std::string& ex : extensions())
        if (!Instance::isExtensionSupported(ex))
            return false;
    return true;
}

/* ---------------------------------------------------------------- */

std::vector<std::string> Surface::extensions()
{
    std::vector<std::string> out;
    out.push_back("VK_KHR_surface");
    out.push_back("VK_KHR_win32_surface");
    return out;

//    uint32_t glfwExtensionCount = 0;
//    const char** glfwExtensions =
//        glfwGetRequiredInstanceExtensions(
//            &glfwExtensionCount);

//    std::vector<std::string> out;
//    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
//        out.push_back(std::string(glfwExtensions[i]));
//    return out;
}

} // namespace vk
} // namespace kuu
