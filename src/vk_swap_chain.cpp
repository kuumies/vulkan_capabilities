/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::SwapChain class
 * ---------------------------------------------------------------- */

#include "vk_swap_chain.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct SwapChain::Data
{
    VkDevice device;
};

/* ---------------------------------------------------------------- */

SwapChain::SwapChain(VkDevice device)
    : d(std::make_shared<Data>())
{
    d->device = device;
}

/* ---------------------------------------------------------------- */

void SwapChain::create()
{
}

/* ---------------------------------------------------------------- */

void SwapChain::destroy()
{
}

} // namespace vk
} // namespace kuu
