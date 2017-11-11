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
   A queue
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

    void submit(VkCommandBuffer buffer,
                const Semaphore& waitSemaphore,
                const Semaphore& signalSemaphore);
    void present(const SwapChain& swapChain,
                 const uint32_t imageIndex,
                 const Semaphore& waitSemaphore);
    void waitIdle();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
