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
        info.pViewportState = &viewportState;
        info.pRasterizationState = &rasterizerState;


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
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineRasterizationStateCreateInfo rasterizerState;
    VkPipelineMultisampleStateCreateInfo multisampleState;
    VkPipelineDepthStencilStateCreateInfo depthStencilState;
    VkPipelineColorBlendStateCreateInfo colorBlendState;
    VkPipelineDynamicStateCreateInfo dynamicState;
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
    vertexInputState.vertexBindingDescriptionCount   = uint32_t(bindingDescs.size());
    vertexInputState.pVertexBindingDescriptions      = bindingDescs.data();
    vertexInputState.vertexAttributeDescriptionCount = uint32_t(attributeDescs.size());
    vertexInputState.pVertexAttributeDescriptions    = attributeDescs.data();
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
    state.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    state.pNext                  = NULL;
    state.flags                  = 0;
    state.topology               = topology;
    state.primitiveRestartEnable = primitiveRestartEnable;

    return setInputAssemblyState(state);
}

VkPipelineInputAssemblyStateCreateInfo Pipeline::inputAssemblyState() const
{ return impl->inputAssemblyState; }

Pipeline& Pipeline::setViewportState(const VkPipelineViewportStateCreateInfo& state)
{
    impl->viewportState = state;
    return *this;
}

Pipeline& Pipeline::setViewportState(
    const std::vector<VkViewport>& viewports,
    const std::vector<VkRect2D>& scissors)
{
    VkPipelineViewportStateCreateInfo state;
    state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    state.pNext         = NULL;
    state.flags         = 0;
    state.viewportCount = uint32_t(viewports.size());
    state.pViewports    = viewports.data();
    state.scissorCount  = uint32_t(scissors.size());
    state.pScissors     = scissors.data();
    return setViewportState(state);
}

VkPipelineViewportStateCreateInfo Pipeline::viewportState() const
{ return impl->viewportState; }

Pipeline& Pipeline::setRasterizerState(const VkPipelineRasterizationStateCreateInfo& state)
{
    impl->rasterizeState = state;
    return *this;
}

Pipeline& Pipeline::setRasterizerState(
    VkPolygonMode polygonMode,
    VkCullModeFlags cullMode,
    VkFrontFace frontFace,
    VkBool32 depthClampEnable,
    VkBool32 rasterizerDiscardEnable,
    VkBool32 depthBiasEnable,
    float depthBiasConstantFactor,
    float depthBiasClamp,
    float depthBiasSlopeFactor,
    float lineWidth);
{
    VkPipelineRasterizationStateCreateInfo state;
    state.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    state.pNext                   = NULL;
    state.flags                   = 0;
    state.depthClampEnable        = depthClampEnable;
    state.rasterizerDiscardEnable = rasterizerDiscardEnable;
    state.polygonMode             = polygonMode;
    state.cullMode                = cullMode;
    state.frontFace               = frontFace;
    state.depthBiasEnable         = depthBiasEnable;
    state.depthBiasConstantFactor = depthBiasConstantFactor;
    state.depthBiasClamp          = depthBiasClamp;
    state.depthBiasSlopeFactor    = depthBiasSlopeFactor;
    state.lineWidth               = lineWidth;

    return setRasterizerState(state);
}

VkPipelineRasterizationStateCreateInfo Pipeline::rasterizeState() const
{ return impl->rasterizeState; }

Pipeline& Pipeline::setMultisampleState(const VkPipelineMultisampleStateCreateInfo& state)
{
    impl->multisampleState = state;
    return *this;
}

Pipeline& Pipeline::setMultisampleState(
    VkBool32 sampleShadingEnable,
    VkSampleCountFlagBits rasterizationSamples,
    float minSampleShading,
    const VkSampleMask* pSampleMask,
    VkBool32 alphaToCoverageEnable,
    VkBool32 alphaToOneEnable)
{
    VkPipelineMultisampleStateCreateInfo state;
    state.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    state.pNext                 = NULL;
    state.flags                 = 0;
    state.rasterizationSamples  = rasterizationSamples;
    state.sampleShadingEnable   = sampleShadingEnable;
    state.minSampleShading      = minSampleShading;
    state.pSampleMask           = pSampleMask;
    state.alphaToCoverageEnable = alphaToCoverageEnable;
    state.alphaToOneEnable      = alphaToOneEnable;

    return setMultisampleState(state);
}

VkPipelineMultisampleStateCreateInfo Pipeline::multisampleState() const
{ return impl->multisampleState; }


Pipeline& Pipeline::setDepthStencilState(const VkPipelineDepthStencilStateCreateInfo& state)
{
    impl->depthStencilState = state;
    return *this;
}

Pipeline& Pipeline::setDepthStencilState(
    VkBool32 depthTestEnable,
    VkBool32 depthWriteEnable,
    VkCompareOp depthCompareOpm,
    VkBool32 depthBoundsTestEnable,
    VkBool32 stencilTestEnable,
    VkStencilOpState front,
    VkStencilOpState back,
    float minDepthBounds,
    float maxDepthBounds)
{
    VkPipelineDepthStencilStateCreateInfo state;
    state.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    state.pNext                 = NULL;
    state.flags                 = 0;
    state.depthTestEnable       = depthTestEnable;
    state.depthWriteEnable      = depthWriteEnable;
    state.depthCompareOp        = depthCompareOp;
    state.depthBoundsTestEnable = depthBoundsTestEnable;;
    state.stencilTestEnable     = stencilTestEnable;
    state.front                 = front;
    state.back                  = back;;
    state.minDepthBounds        = minDepthBounds;
    state.maxDepthBounds        = maxDepthBounds;

    return setDepthStencilState(state);
}

VkPipelineDepthStencilStateCreateInfo depthStencilState() const
{ return impl->depthStencilState; }


// Sets  and gets the color blending state.
Pipeline& Pipeline::setColorBlendingState(const VkPipelineColorBlendStateCreateInfo& state)
{
    impl->colorBlendState = state;
    return *this;
}

Pipeline& Pipeline::setColorBlendingState(
    VkBool32 logicOpEnable,
    VkLogicOp logicOp,
    const std::vector<VkPipelineColorBlendAttachmentState> attachments,
    float blendConstants[4])
{

}

VkPipelineColorBlendStateCreateInfo Pipeline::colorBlendingState() const
{ return impl->colorBlendState; }

Pipeline& Pipeline::setDynamicState(const VkPipelineDynamicStateCreateInfo& state)
{
    impl->dynamicState = state;
    return *this;
}

Pipeline& Pipeline::setDynamicState(const std::vector<VkDynamicState>& states)
{

}

VkPipelineDynamicStateCreateInfo Pipeline::dynamicState() const
{ return impl->dynamicState; }

Pipeline& Pipeline::setPipelineLayout(const VkPipelineLayoutCreateInfo& layout)
{

}

Pipeline& Pipeline::setPipelineLayout(
    std::vector<VkDescriptorSetLayout>& layouts,
    std::vector<VkPushConstantRange>& pushConstantRanges)
{

}

VkPipelineLayoutCreateInfo Pipeline::pipelineLayout() const
{

}

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
