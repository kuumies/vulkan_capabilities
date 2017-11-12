/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Queue class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

class Semaphore;
class SwapChain;

/* ---------------------------------------------------------------- *
   A queue. All the vulkan commands as submitted into queue before
   they are processed. There can be multiple types of queues but
   this class can be used for two types of queue:

    graphics:     rendering queue
    presentation: present rendered images to user on window surface

    Each queue has a queue family index.
 * ---------------------------------------------------------------- */
class Queue
{
public:
    // Defines the type of the queue.
    enum class Type
    {
        Graphics,       // Graphics queue
        Presentation    // Presentation queue
    };

    // Constructs the queue for logical device, queue family
    // index and queue type.
    Queue(const VkDevice& device, uint32_t familyIndex, Type type);

    // Returns the family index.
    uint32_t familyIndex() const;
    // Returns the type.
    Type type() const;
    // Returns handle.
    VkQueue handle() const;

    // Submits a command buffer into queue.
    void submit(VkCommandBuffer buffer,
                const Semaphore& waitSemaphore,
                const Semaphore& signalSemaphore);

    // Presents a image in the swap chain into window surface.
    void present(const SwapChain& swapChain,
                 const uint32_t imageIndex,
                 const Semaphore& waitSemaphore);

    // Waits until the queue has been processed.
    void waitIdle();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
