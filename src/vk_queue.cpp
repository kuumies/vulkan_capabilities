/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Queue class
 * ---------------------------------------------------------------- */

#include "vk_queue.h"

/* ---------------------------------------------------------------- */

#include <iostream>
#include <vector>

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

} // namespace vk
} // namespace kuu
