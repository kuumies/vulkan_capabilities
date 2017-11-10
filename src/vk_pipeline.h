/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Pipeline class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "vk_mesh.h"
#include "vk_shader_stage.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

class Device;
class Surface;

/* ---------------------------------------------------------------- *
   A pipeline
 * ---------------------------------------------------------------- */
class Pipeline
{
public:
    // Defines pipeline params
    struct Parameters
    {
        Parameters(Mesh mesh, ShaderStage shaderStage)
            : mesh(mesh)
            , shaderStage(shaderStage)
        {}

        uint32_t viewportWidth;
        uint32_t viewportHeight;

        VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;

        Mesh mesh;
        ShaderStage shaderStage;
    };

    // Constructs the pipeline
    Pipeline(const Device& device,
             const Surface& surface,
             const Parameters& params);

    VkPipelineLayout pipelineLayout() const;
    VkRenderPass renderPass() const;
    VkPipeline graphicsPipeline() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
