/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Swapchain class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A Vulkan swapchain wrapper class.
 * -------------------------------------------------------------------------- */
class Swapchain
{
public:
    // Constructs the swap chain.
    Swapchain(const VkDevice& logicalDevice,
              const VkSurfaceKHR& surface);

    // Sets and gets the surface format.
    Swapchain& setSurfaceFormat(const VkSurfaceFormatKHR& surfaceFormat);
    VkSurfaceFormatKHR surfaceFormat() const;

    // Sets and gets the present mode.
    Swapchain& setPresentMode(const VkPresentModeKHR& presentMode);
    VkPresentModeKHR presentMode() const;

    // Sets and gets the image extent.
    Swapchain& setImageExtent(const VkExtent2D& imageExtent);
    VkExtent2D imageExtent() const;

    // Sets and gets the image count..
    Swapchain& setImageCount(const uint32_t imageCount);
    uint32_t imageCount() const;

    // Sets and gets the surface pre-transform.
    Swapchain& setPreTransform(VkSurfaceTransformFlagBitsKHR preTransform);
    VkSurfaceTransformFlagBitsKHR preTransform() const;

    // Sets the queue family indices who uses the image from swapchain.
    Swapchain& setQueueIndicies(const std::vector<uint32_t>& indices);
    std::vector<uint32_t> queueIndices() const;

    // Creates and destroys the swap chain
    bool create();
    void destroy();

    // Returns true if the swap chain handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the handle.
    VkSwapchainKHR handle() const;

    // Returns image views of the swapchain images.
    std::vector<VkImageView> imageViews() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
