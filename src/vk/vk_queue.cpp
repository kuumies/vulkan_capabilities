/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Queue class
 * -------------------------------------------------------------------------- */

#include "vk_queue.h"
#include <iostream>
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct Queue::Impl
{
    bool create()
    {
        vkGetDeviceQueue(
            logicalDevice,
            queueFamilyIndex,
            queueIndex,
            &queue);

        return queue != VK_NULL_HANDLE;
    }

    bool isValid() const
    {
        return queue != VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkQueue queue;

    // From user
    uint32_t queueFamilyIndex;
    uint32_t queueIndex;
};

/* -------------------------------------------------------------------------- */

Queue::Queue(const VkDevice& logicalDevice,
             uint32_t queueFamilyIndex,
             uint32_t queueIndex)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
    impl->queueFamilyIndex = queueFamilyIndex;
    impl->queueIndex       = queueIndex;
}

VkQueue Queue::handle() const
{ return impl->queue; }

bool Queue::create()
{
    if (!impl->isValid())
        return impl->create();
    return true;
}

bool Queue::isValid() const
{ return impl->isValid(); }

bool Queue::submit(
        const std::vector<VkCommandBuffer>& commandBuffers,
        const std::vector<VkSemaphore>& signalSync,
        const std::vector<VkSemaphore>& waitSync,
        const std::vector<VkPipelineStageFlags>& waitStageFlags,
        const VkFence& fence)
{
    if (waitSync.size() != waitStageFlags.size())
    {
        std::cerr << __FUNCTION__
                  << ": wait sync / wait stage flags size mismatch"
                  << std::endl;
        return false;
    }

    VkSubmitInfo submitInfo;
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                = NULL;
    submitInfo.waitSemaphoreCount   = uint32_t(waitSync.size());
    submitInfo.pWaitSemaphores      = waitSync.data();
    submitInfo.pWaitDstStageMask    = waitStageFlags.data();
    submitInfo.commandBufferCount   = uint32_t(commandBuffers.size());
    submitInfo.pCommandBuffers      = commandBuffers.data();
    submitInfo.signalSemaphoreCount = uint32_t(signalSync.size());
    submitInfo.pSignalSemaphores    = signalSync.data();

    const VkResult result = vkQueueSubmit(
        impl->queue, 1,
        &submitInfo,
        fence);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": graphics queue submit failed as "
                  << vk::stringify::resultDesc(result)
                  << std::endl;
        return false;
    }

    return true;
}

bool Queue::submit(
    const VkCommandBuffer& commandBuffer,
    const VkSemaphore &signalSync,
    const VkSemaphore &waitSync,
    const VkPipelineStageFlags& waitStageFlag,
    const VkFence &fence)
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                = NULL;
    if (waitSync != VK_NULL_HANDLE)
    {
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = &waitSync;
        submitInfo.pWaitDstStageMask    = &waitStageFlag;
    }
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &commandBuffer;
    if (signalSync != VK_NULL_HANDLE)
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = &signalSync;
    }

    const VkResult result = vkQueueSubmit(
        impl->queue, 1,
        &submitInfo,
        fence);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": queue submit failed as "
                  << vk::stringify::resultDesc(result)
                  << std::endl;
        return false;
    }

    return true;
}

bool Queue::present(
    const std::vector<VkSwapchainKHR>& swapchains,
    const std::vector<VkSemaphore>& waitSync,
    const std::vector<uint32_t>& imageIndices)
{
    if (swapchains.size() != imageIndices.size())
    {
        std::cerr << __FUNCTION__
                  << ": swapchain / image index size mismatch"
                  << std::endl;
        return false;
    }

    VkPresentInfoKHR presentInfo;
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = NULL;
    presentInfo.waitSemaphoreCount = uint32_t(waitSync.size());
    presentInfo.pWaitSemaphores    = waitSync.data();
    presentInfo.swapchainCount     = uint32_t(swapchains.size());
    presentInfo.pSwapchains        = swapchains.data();
    presentInfo.pImageIndices      = imageIndices.data();
    presentInfo.pResults           = NULL;

    const VkResult result = vkQueuePresentKHR(impl->queue, &presentInfo);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": queue present failed as "
                  << vk::stringify::resultDesc(result)
                  << std::endl;
        return false;
    }

    return true;
}

bool Queue::present(
    const VkSwapchainKHR& swapchain,
    const VkSemaphore& waitSync,
    const uint32_t& imageIndex)
{
    VkPresentInfoKHR presentInfo;
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = NULL;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &waitSync;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapchain;
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = NULL;

    const VkResult result = vkQueuePresentKHR(impl->queue, &presentInfo);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": queue present failed as "
//                  << vk::stringify::resultDesc(result)
                  << std::endl;
        return false;
    }

    return true;
}

bool Queue::waitIdle()
{
    const VkResult result = vkQueueWaitIdle(impl->queue);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": queue idle wait failed as "
//                  << vk::stringify::resultDesc(result)
                  << std::endl;
        return false;
    }

    return true;
}

} // namespace vk
} // namespace kuu
