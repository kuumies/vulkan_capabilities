/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Queue class
 * ---------------------------------------------------------------- */

#include "vk_queue.h"

/* ---------------------------------------------------------------- */

#include <iostream>
#include <vector>

/* ---------------------------------------------------------------- */

#include "vk_semaphore.h"
#include "vk_swap_chain.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct Queue::Data
{
    Data(const VkDevice& device,
         uint32_t familyIndex,
         Type type)
        : type(type)
        , familyIndex(familyIndex)
    {
        vkGetDeviceQueue(device, familyIndex, 0, &queue);
    }

    VkQueue queue;
    Type type;
    uint32_t familyIndex;
};

/* ---------------------------------------------------------------- */

Queue::Queue(const VkDevice& device, uint32_t familyIndex, Type type)
    : d(std::make_shared<Data>(device, familyIndex, type))
{}

/* ---------------------------------------------------------------- */

uint32_t Queue::familyIndex() const
{ return d->familyIndex; }

/* ---------------------------------------------------------------- */

Queue::Type Queue::type() const
{ return d->type; }

/* ---------------------------------------------------------------- */

VkQueue Queue::handle() const
{ return d->queue; }

/* ---------------------------------------------------------------- */

void Queue::submit(VkCommandBuffer buffer,
                   const Semaphore& waitSemaphore,
                   const Semaphore& signalSemaphore)
{
    const VkSemaphore waitSemaphores[] =
    { waitSemaphore.handle() };
    VkSemaphore signalSemaphores[]  =
    { signalSemaphore.handle() };

    const VkPipelineStageFlags waitStages[] =
    { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &buffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    VkResult result = vkQueueSubmit(
        d->queue,
        1,
        &submitInfo,
        VK_NULL_HANDLE);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to submit into graphics queue"
                  << std::endl;
    }
}

/* ---------------------------------------------------------------- */

void Queue::present(const SwapChain& swapChain,
                    const uint32_t imageIndex,
                    const Semaphore& waitSemaphore)
{
    VkSemaphore signalSemaphores[]  =
    { waitSemaphore.handle() };

    VkSwapchainKHR swapChainHandle = swapChain.handle();

    // Create the present info which tells:
    //   * semaphore to wait until the rendering result image can
    //     be presented to screen surface
    //   * swap chain and swap chain image to present to screen.
    //   * (other options are needed only when using multiple swap
    //      chains)
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphores;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapChainHandle;
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = nullptr;

    // Present the rendering result into screen surface.
    VkResult result = vkQueuePresentKHR(d->queue, &presentInfo);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to present the rendering results "
                  << "to screen surface."
                  << std::endl;
    }
}

/* ---------------------------------------------------------------- */

void Queue::waitIdle()
{

    // Wait until the queue has finished.
    VkResult result = vkQueueWaitIdle(d->queue);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": vkQueueWaitIdle failed"
                  << std::endl;
    }
}

} // namespace vk
} // namespace kuu
