/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::PhysicalDevice class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <vulkan/vulkan.h>

/* ---------------------------------------------------------------- */

#include "vk_surface.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- *
   A physical device with Vulkan support.
 * ---------------------------------------------------------------- */
class PhysicalDevice
{
public:
    // Constructs the physical device.
    PhysicalDevice(VkPhysicalDevice device);

    // Returns the handle to physical device.
    VkPhysicalDevice handle() const;

    // Returns a surface format suitable for the physical device.
    VkSurfaceFormatKHR suitableSurfaceFormat(
        const Surface& surface,
        VkFormat format = VK_FORMAT_B8G8R8A8_UNORM,
        VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) const;

    // Returns true if the extension is supported.
    bool isExtensionSupported(const std::string& extension) const;
    bool isExtensionSupported(const std::vector<std::string>& extensions) const;

    // Returns true if the surface image extent is supported.
    bool isImageExtentSupported(
        const Surface& surface,
        const glm::ivec2& extent) const;
    // Returns true if the surface swap image count is supported.
    bool isSwapChainImageCountSupported(
            const Surface& surface,
            uint32_t count) const;
    // Returns true if the surface preset mode is supported.
    bool isPresentModeSupported(const Surface& surface,
                                VkPresentModeKHR presentMode) const;
    // Returns true if the surface format is supported.
    bool isSurfaceSupported(const Surface& surface,
                            VkFormat format,
                            VkColorSpaceKHR colorSpace) const;

    // Dump information about the physical device into standard
    // output.
    void dump();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
