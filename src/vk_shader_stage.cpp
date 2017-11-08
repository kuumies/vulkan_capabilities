/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::ShaderStage class
 * ---------------------------------------------------------------- */

#include "vk_shader_stage.h"

/* ---------------------------------------------------------------- */

#include <iostream>
#include <fstream>

/* ---------------------------------------------------------------- */

#include "vk_shader.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- *
   Reads a SPIR-V binary shader file form the path.
 * ---------------------------------------------------------------- */
std::vector<char> readShaderSourceFile(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error(
            __FUNCTION__ +
                std::string(": failed to open shader file ") +
                path);

    size_t fileSize = (size_t) file.tellg();
    if (fileSize == 0)
        throw std::runtime_error(
            __FUNCTION__ +
                std::string(": shader source is zero-sized ") +
                path);

    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

/* ---------------------------------------------------------------- */

struct ShaderStage::Data
{
    Data(VkDevice device)
        : device(device)
        , vsh(device, Shader::Type::Vertex)
        , fsh(device, Shader::Type::Fragment)
    {}

    VkDevice device; // logical device
    Shader vsh;      // vertex shader
    Shader fsh;      // fragment shader
};

/* ---------------------------------------------------------------- */

ShaderStage::ShaderStage(VkDevice device)
    : d(std::make_shared<Data>(device))
{}

/* ---------------------------------------------------------------- */

std::vector<VkPipelineShaderStageCreateInfo> ShaderStage::shaderStages() const
{
    std::vector<VkPipelineShaderStageCreateInfo> out;
    if (d->vsh.isCreated())
        out.push_back(d->vsh.createInfo());
    if (d->fsh.isCreated())
        out.push_back(d->fsh.createInfo());
    return out;
}

/* ---------------------------------------------------------------- */

void ShaderStage::create(const std::string& vshPath,
                         const std::string& fshPath)
{
    d->vsh.create(readShaderSourceFile(vshPath));
    d->fsh.create(readShaderSourceFile(fshPath));
}

/* ---------------------------------------------------------------- */

void ShaderStage::destroy()
{
    d->vsh.destroy();
    d->fsh.destroy();
}

} // namespace vk
} // namespace kuu
