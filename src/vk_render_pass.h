/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::RenderPass class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

/* ---------------------------------------------------------------- */

#include "vk_swap_chain.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

class Device;
class Surface;

/* ---------------------------------------------------------------- *
   A render pass
 * ---------------------------------------------------------------- */
class RenderPass
{
public:
    // Defines render pass params
    struct Parameters
    {
        Parameters(uint32_t viewportWidth, uint32_t viewportHeight, SwapChain swapChain)
            : viewportWidth(viewportWidth)
            , viewportHeight(viewportHeight)
            , swapChain(swapChain)
        {}

        uint32_t viewportWidth;
        uint32_t viewportHeight;
        SwapChain swapChain;
    };

    // Constructs the pipeline
    RenderPass(const Device& device,
               const Surface& surface,
               const Parameters& params);

    // Returns true if the render pass is valid.
    bool isValid() const;

    // Returns the handle.
    VkRenderPass handle() const;
    // Returns swap chain framebuffers.
    std::vector<VkFramebuffer> swapChainFramebuffers() const;

    void begin(int i, VkCommandBuffer buffer, VkClearValue clearColor);
    void end(VkCommandBuffer buffer);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
