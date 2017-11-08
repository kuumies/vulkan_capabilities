/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Shader class
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
   A vulkan shader.
 * ---------------------------------------------------------------- */
class Shader
{
public:
    // Defines the shader types.
    enum class Type
    {
        Vertex,  // vertex shader
        Fragment // fragment shader
    };

    // Constructs the shader.
    Shader(VkDevice device, Type type);

    // Returns the shader stage create info.
    VkPipelineShaderStageCreateInfo createInfo() const;

    // Returns true if the shader has been created.
    bool isCreated() const;

    // Creates the shader from the SPIR-V source code.
    void create(const std::vector<char>& spirvSourceCode);

    // Destroys the shader.
    void destroy();

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
