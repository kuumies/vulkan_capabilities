/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::ShaderStage class
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

/* ---------------------------------------------------------------- *
   A vulkan shader stage.

   This class is used to create a shaders from the SPIR-V source
   that is loaded from a binary file.
 * ---------------------------------------------------------------- */
class ShaderStage
{
public:
    // Constructs the shader stage.
    ShaderStage(VkDevice device);

    // Returns the shader stages.
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages() const;

    // Creates the shader stage from vertex and fragment shader
    // source files.
    void create(const std::string& vshPath,
                const std::string& fshPath);

    // Destroys the shader stage.
    void destroy();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
