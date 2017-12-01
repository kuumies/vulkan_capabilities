/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Queue class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A vulkan queue wrapper class
 * -------------------------------------------------------------------------- */
class Queue
{
public:
    // Constructs the queue.
    Queue(const VkDevice& logicalDevice,
          uint32_t queueFamilyIndex,
          uint32_t queueIndex);

    // Creates the queue.
    bool create();

    // Returns true if the queue handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the handle.
    VkQueue handle() const;

    // Submits a single batch of commands into queue.
    bool submit(const std::vector<VkCommandBuffer>& commandBuffers,
                const std::vector<VkSemaphore>& signalSync,
                const std::vector<VkSemaphore>& waitSync,
                const std::vector<VkPipelineStageFlags>& waitStageFlags,
                const VkFence& fence = VK_NULL_HANDLE);

    // Submits a single command into queue.
    bool submit(const VkCommandBuffer& commandBuffer,
                const VkSemaphore& signalSync = VK_NULL_HANDLE,
                const VkSemaphore& waitSync = VK_NULL_HANDLE,
                const VkPipelineStageFlags& waitStageFlag = 0,
                const VkFence& fence = VK_NULL_HANDLE);

    // Present images to swapchain surfaces.
    bool present(
        const std::vector<VkSwapchainKHR>& swapchains,
        const std::vector<VkSemaphore>& waitSync,
        const std::vector<uint32_t>& imageIndices);

    // Present image to swapchain surface.
    bool present(
        const VkSwapchainKHR& swapchain,
        const VkSemaphore& waitSync,
        const uint32_t& imageIndex);

    // Host waits for completion of outstanding queue operations
    bool waitIdle();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
