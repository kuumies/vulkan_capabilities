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

/* ---------------------------------------------------------------- *
   A vulkan queue
 * ---------------------------------------------------------------- */
class Queue
{
    friend class Device;

public:
    // Defines the type of the queue.
    enum class Type
    {
        Graphics,       // Graphics queue
        Presentation    // Presentation queue
    };

    // Constructs the queue.
    Queue(VkPhysicalDevice physicalDevice,
          VkSurfaceKHR surface,
          Type type);

    // Returns the queue create info. The info is valid after
    // the queue is created.
    VkDeviceQueueCreateInfo deviceQueue() const;
    // Returns the family index. Queues must have been created.
    uint32_t familyIndex() const;
    // Returns queue handle. The handle is valid only if the
    // queue has been added into logical device.
    VkQueue handle() const;

    // Creates the family index. Returns true if the creation
    // succeeded.
    bool create(uint32_t count, float priority);

    // Destroys the queue.
    void destroy();

private:
    void setHandle(VkQueue handle);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
