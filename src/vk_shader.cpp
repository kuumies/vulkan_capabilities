/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Shader class
 * ---------------------------------------------------------------- */

#include "vk_shader.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct Shader::Data
{
    VkDevice device;            // logical device
    VkShaderModule module;      // shader module
    Type type;                  // shader type
    bool created = false;       // is the shader created.
};

/* ---------------------------------------------------------------- */

Shader::Shader(VkDevice device, Type type)
    : d(std::make_shared<Data>())
{
    d->device = device;
    d->type   = type;
}

/* ---------------------------------------------------------------- */

VkPipelineShaderStageCreateInfo Shader::createInfo() const
{
    VkShaderStageFlagBits stage;
    switch(d->type)
    {
        case Type::Vertex:   stage = VK_SHADER_STAGE_VERTEX_BIT;   break;
        case Type::Fragment: stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
    }

    VkPipelineShaderStageCreateInfo vshStageInfo = {};
    vshStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vshStageInfo.stage  = stage;
    vshStageInfo.module = d->module;
    vshStageInfo.pName  = "main";
    return vshStageInfo;
}

/* ---------------------------------------------------------------- */

bool Shader::isCreated() const
{ return d->created; }

/* ---------------------------------------------------------------- */

void Shader::create(const std::vector<char>& spirvSourceCode)
{
    VkShaderModuleCreateInfo info = {};
    info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = spirvSourceCode.size();
    info.pCode    = reinterpret_cast<const uint32_t*>(spirvSourceCode.data());

    VkResult result = vkCreateShaderModule(d->device, &info, nullptr, &d->module);
    if (result != VK_SUCCESS)
        throw std::runtime_error(
            __FUNCTION__ +
                std::string(": failed to create vulkan vertex shader module"));

    d->created = true;
}

/* ---------------------------------------------------------------- */

void Shader::destroy()
{
    if (d->created)
        vkDestroyShaderModule(d->device, d->module, nullptr);
}

} // namespace vk
} // namespace kuu
