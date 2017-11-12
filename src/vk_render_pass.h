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
   A render pass belonging to a pipeline.

   Render pass handles the logic of binding and releasing a
   framebuffer before and after rendering. Each image in the 
   swapchain has its own framebuffer.
 * ---------------------------------------------------------------- */
class RenderPass
{
public:
    // Defines render pass params
    struct Parameters
    {
        Parameters(uint32_t viewportWidth, 
                   uint32_t viewportHeight, 
                   SwapChain swapChain)
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

    // Adds a command into command buffer to begin the render pass.
    // Render target is the ith swapchain framebuffer. Clear color 
    // is the color that is use to clear the framebuffer.
    void begin(int i, VkCommandBuffer buffer, VkClearValue clearColor);
    // Adds a command to end the render pass.
    void end(VkCommandBuffer buffer);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
