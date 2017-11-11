/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::SwapChain class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

class Device;
class Semaphore;
class Surface;

/* ---------------------------------------------------------------- *
   A vulkan swap chain.
 * ---------------------------------------------------------------- */
class SwapChain
{
public:
    // Constructs the swap chain.
    SwapChain(const Device& device,
              const Surface& surface);

    // Returns true if the swap chain is valid.
    bool isValid() const;

    // Returns handle.
    VkSwapchainKHR handle() const;

    // Returns the image views.
    std::vector<VkImageView> imageViews() const;
    // Returns the count of image views.
    int imageViewSize() const;

    uint32_t acquireImage(const Semaphore& semaphore);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
