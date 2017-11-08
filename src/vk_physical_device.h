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

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- *
   A physical device with Vulkan ability. Use Instance class to
   get the available physical devices.
 * ---------------------------------------------------------------- */
class PhysicalDevice
{
public:
    // Constructs the physical device.
    PhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface);

    // Returns the handle to physical device.
    VkPhysicalDevice handle() const;

    // Returns true if the extension is supported.
    bool isExtensionSupported(const std::string& extension) const;
    bool isExtensionSupported(const std::vector<std::string>& extensions) const;
    // Returns true if the image extent is supported.
    bool isImageExtentSupported(const glm::ivec2& extent) const;
    // Returns true if the swap image count is supported.
    bool isSwapChainImageCountSupported(uint32_t count) const;
    // Returns true if the preset mode is supported.
    bool isPresentModeSupported(VkPresentModeKHR presentMode) const;
    // Returns true if the surface format is supported.
    bool isSurfaceSupported(VkFormat format,
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
