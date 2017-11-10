/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Surface class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

/* ---------------------------------------------------------------- */

struct GLFWwindow;

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

class Instance;
class PhysicalDevice;

/* ---------------------------------------------------------------- *
   A surface for Vulkan-based rendering.

   Surface is needed to present rendered images on the window. As
   Vulkan does not care about window, to use surface one must use
   extensions, e.g. for Windows these are VK_KHR_surface and
   VK_KHR_win32_surface.

   Surface is per-window object therefore user must give in the
   window object during creation.

   TODO: this now works with a GLFW window but what about Qt
         window? That would allow stacking widgets to window.

 * ---------------------------------------------------------------- */
class Surface
{
public:
    // Constructs the surface.
    Surface(const Instance& instance, GLFWwindow* window);

    // Returns true if the surface is valid.
    bool isValid() const;

    // Returns the handle to surface.
    VkSurfaceKHR handle() const;

    // Sets and gets the surface format.
    Surface& setFormat(VkSurfaceFormatKHR format);
    VkSurfaceFormatKHR format() const;

    // Sets and gets the present mode.
    Surface& setPresentMode(VkPresentModeKHR presentMode);
    VkPresentModeKHR presentMode() const;

    // Sets and gets the image extent.
    Surface& setImageExtent(VkExtent2D imageExtent);
    VkExtent2D imageExtent() const;

    // Sets and gets the swap chain image count.
    Surface& setSwapChainImageCount(uint32_t swapChainImageCount);
    uint32_t swapChainImageCount() const;

    // Returns true if the surface is combatible with the physical
    // device. AMD/NVIDIA does not seems to support all of the
    // formats, e.g. NVIDIA 1080TI does not support
    // VK_FORMAT_R8G8B8A8_UNORM surface format.
    //
    // If the surface format is not compatible then use
    // setCompatibleWith to enable the compatibility.
    bool isCompatibleWith(const PhysicalDevice& device) const;

    // Sets the surface to be compatible with the physical device.
    //
    // Returns true if the compatible surface properties were found.
    bool setCompatibleWith(const PhysicalDevice& device);

    // Returns true if the implementation supports all of the
    // extensions that the surface needs.
    static bool areExtensionsSupported();
    // Returns the instance extensions implementation must support.
    // These can be asked before creating the surface.
    static std::vector<std::string> extensions();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
