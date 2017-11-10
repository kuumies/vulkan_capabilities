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
    Queue(const VkDevice& device, uint32_t familyIndex, Type type);

    // Returns the family index. Queues must have been created.
    uint32_t familyIndex() const;
    // Returns the type.
    Type type() const;

    // Returns queue handle.
    VkQueue handle() const;


private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
