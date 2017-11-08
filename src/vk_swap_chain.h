/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::SwapChain class
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
   A vulkan swap chain.
 * ---------------------------------------------------------------- */
class SwapChain
{
public:
    // Constructs the swap chain.
    SwapChain(VkDevice device);

    // Creates the swap chain.
    void create();
    // Destroys the swap chain.
    void destroy();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
