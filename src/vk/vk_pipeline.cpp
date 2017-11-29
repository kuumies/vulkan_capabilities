/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Pipeline class
 * -------------------------------------------------------------------------- */

#include "vk_pipeline.h"
#include <iostream>
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct Pipeline::Impl
{
    ~Impl()
    {
        destroy();
    }

    void create()
    {
        VkGraphicsPipelineCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.pNext = NULL;
        info.flags = 0;
        info.stageCount = uint32_t(shaderStages.size());
        info.pStages = shaderStages.data();
        info.pVertexInputState = &vertexInputState;
        info.pInputAssemblyState = &inputAssemblyState;



        VkResult result;

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": descriptor pool creation failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;

            return;
        }
    }

    void destroy()
    {
        pipeline = VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkPipeline pipeline = VK_NULL_HANDLE;

    // From user
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputState;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
};

/* -------------------------------------------------------------------------- */

Pipeline::Pipeline(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
}

Pipeline& Pipeline::addShaderStage(VkPipelineShaderStageCreateInfo stage)
{
    impl->shaderStages.push_back(stage);
    return *this;
}

std::vector<VkPipelineShaderStageCreateInfo> Pipeline::shaderStages() const
{ return impl->shaderStages; }

Pipeline& Pipeline::setVertexInputState(
    const VkPipelineVertexInputStateCreateInfo& vertexInputState)
{
    impl->vertexInputState = vertexInputState;
    return *this;
}

Pipeline& Pipeline::setVertexInputState(
    const std::vector<VkVertexInputBindingDescription>& bindingDescs,
    const std::vector<VkVertexInputAttributeDescription>& attributeDescs)
{
    VkPipelineVertexInputStateCreateInfo vertexInputState;
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.pNext = NULL;
    vertexInputState.flags = 0;
    vertexInputState.vertexBindingDescriptionCount = uint32_t(bindingDescs.size());
    vertexInputState.pVertexBindingDescriptions = bindingDescs.data();
    vertexInputState.vertexAttributeDescriptionCount = uint32_t(attributeDescs.size());
    vertexInputState.pVertexAttributeDescriptions= attributeDescs.data();
    return setVertexInputState(vertexInputState);
}

VkPipelineVertexInputStateCreateInfo Pipeline::vertexInputState() const
{ return impl->vertexInputState; }

Pipeline& Pipeline::setInputAssemblyState(
    const VkPipelineInputAssemblyStateCreateInfo& state)
{
    impl->inputAssemblyState = state;
    return *this;
}

Pipeline& Pipeline::setInputAssemblyState(
    VkPrimitiveTopology topology,
    VkBool32 primitiveRestartEnable)
{
    VkPipelineInputAssemblyStateCreateInfo state;
    state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    state.pNext = NULL;
    state.flags = 0;
    state.topology = topology;
    state.primitiveRestartEnable = primitiveRestartEnable;

    return setInputAssemblyState(state);
}

VkPipelineInputAssemblyStateCreateInfo Pipeline::inputAssemblyState() const
{ return impl->inputAssemblyState; }

void Pipeline::create()
{
    if (!isValid())
        impl->create();
}

void Pipeline::destroy()
{
    if (isValid())
        impl->destroy();
}

bool Pipeline::isValid() const
{ return impl->pipeline != VK_NULL_HANDLE; }

VkPipeline Pipeline::handle() const
{ return impl->pipeline; }

} // namespace vk
} // namespace kuu
