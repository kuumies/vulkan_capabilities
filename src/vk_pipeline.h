/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Pipeline class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

/* ---------------------------------------------------------------- */

#include "vk_mesh.h"
#include "vk_shader_stage.h"
#include "vk_swap_chain.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

class Device;
class RenderPass;
class Surface;

/* ---------------------------------------------------------------- *
   A pipeline. To create a pipeline these states are needed to be
   set:

        * render pass
        * pipeline layout 
        * rasterization
        * multisampling
        * color blending
        * viewport 
        * vertex input
        * input assembly

    The Pipeline sets the states to be the common states. User can
    give in some of the parameters that affect the state.

 * ---------------------------------------------------------------- */
class Pipeline
{
public:
    // Defines pipeline params
    struct Parameters
    {
        Parameters(Mesh mesh, 
                   ShaderStage shaderStage, 
                   SwapChain swapChain)
            : mesh(mesh)
            , shaderStage(shaderStage)
            , swapChain(swapChain)
        {}

        // Viewport dimensions
        uint32_t viewportWidth;
        uint32_t viewportHeight;

        // Mesh cull mode
        VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT;
        // Mesh front face winding
        VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;

        // Mesh
        Mesh mesh;
        // Shader stage
        ShaderStage shaderStage;

        // Swap chain
        SwapChain swapChain;
    };

    // Constructs the pipeline
    Pipeline(const Device& device,
             const Surface& surface,
             const Parameters& params);

    // Returns the render pass of the pipeline.
    RenderPass renderPass() const;
    // Returns handle.
    VkPipeline handle() const;

    // Adds a bind command into buffer to bind the pipeline.
    void bind(VkCommandBuffer buf);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
