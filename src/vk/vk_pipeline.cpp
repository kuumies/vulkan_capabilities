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
        VkResult result = vkCreatePipelineLayout(
            logicalDevice,
            &layoutInfo,
            NULL,
            &pipelineLayout);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": pipeline layout creation failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;

            return;
        }

        VkGraphicsPipelineCreateInfo info;
        info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.pNext               = NULL;
        info.flags               = 0;
        info.stageCount          = uint32_t(shaderStages.size());
        info.pStages             = shaderStages.data();
        info.pVertexInputState   = &vertexInputState;
        info.pInputAssemblyState = &inputAssemblyState;
        info.pTessellationState  = NULL;
        info.pViewportState      = &viewportState;
        info.pRasterizationState = &rasterizerState;
        info.pMultisampleState   = &multisampleState;
        info.pDepthStencilState  = &depthStencilState;
        info.pColorBlendState    = &colorBlendState;;
        info.pDynamicState       = &dynamicState;
        info.layout              = pipelineLayout;
        info.renderPass          = renderPass;
        info.subpass             = 0;
        info.basePipelineHandle  = VK_NULL_HANDLE;
        info.basePipelineIndex   = -1;

        result =
            vkCreateGraphicsPipelines(
                logicalDevice,
                VK_NULL_HANDLE,
                1,
                &info,
                NULL,
                &pipeline);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": pipeline creation failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;

            return;
        }
    }

    void destroy()
    {
        vkDestroyPipeline(
            logicalDevice,
            pipeline,
            NULL);

        vkDestroyPipelineLayout(
            logicalDevice,
            pipelineLayout,
            NULL);

        pipeline       = VK_NULL_HANDLE;
        pipelineLayout = VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

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
    VkPipelineLayoutCreateInfo layoutInfo;
    VkRenderPass renderPass = VK_NULL_HANDLE;

    // Keep data "alive" as the structs contains pointers into these
    // vectors. They can be cleared after pipeline creation.
    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescs;
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescs;
    std::vector<VkViewport> viewportViewports;
    std::vector<VkRect2D> viewportScissors;
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
    std::vector<VkDynamicState> dynamicStateStates;
    std::vector<VkDescriptorSetLayout> layoutDescriptorlayouts;
    std::vector<VkPushConstantRange> layoutPushConstantRanges;
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
    impl->vertexInputBindingDescs   = bindingDescs;
    impl->vertexInputAttributeDescs = attributeDescs;

    VkPipelineVertexInputStateCreateInfo vertexInputState;
    vertexInputState.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.pNext                           = NULL;
    vertexInputState.flags                           = 0;
    vertexInputState.vertexBindingDescriptionCount   = uint32_t(impl->vertexInputBindingDescs.size());
    vertexInputState.pVertexBindingDescriptions      = impl->vertexInputBindingDescs.data();
    vertexInputState.vertexAttributeDescriptionCount = uint32_t(impl->vertexInputAttributeDescs.size());
    vertexInputState.pVertexAttributeDescriptions    = impl->vertexInputAttributeDescs.data();
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

Pipeline& Pipeline::setViewportState(
    const VkPipelineViewportStateCreateInfo& state)
{
    impl->viewportState = state;
    return *this;
}

Pipeline& Pipeline::setViewportState(
    const std::vector<VkViewport>& viewports,
    const std::vector<VkRect2D>& scissors)
{
    impl->viewportViewports = viewports;
    impl->viewportScissors  = scissors;

    VkPipelineViewportStateCreateInfo state;
    state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    state.pNext         = NULL;
    state.flags         = 0;
    state.viewportCount = uint32_t(impl->viewportViewports.size());
    state.pViewports    = impl->viewportViewports.data();
    state.scissorCount  = uint32_t(impl->viewportScissors.size());
    state.pScissors     = impl->viewportScissors.data();
    return setViewportState(state);
}

VkPipelineViewportStateCreateInfo Pipeline::viewportState() const
{ return impl->viewportState; }

Pipeline& Pipeline::setRasterizerState(
    const VkPipelineRasterizationStateCreateInfo& state)
{
    impl->rasterizerState = state;
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
    float lineWidth)
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
{ return impl->rasterizerState; }

Pipeline& Pipeline::setMultisampleState(
    const VkPipelineMultisampleStateCreateInfo& state)
{
    impl->multisampleState = state;
    return *this;
}

Pipeline& Pipeline::setMultisampleState(
    VkBool32 sampleShadingEnable,
    VkSampleCountFlagBits rasterizationSamples,
    float minSampleShading,
    const VkSampleMask* sampleMask,
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
    state.pSampleMask           = sampleMask;
    state.alphaToCoverageEnable = alphaToCoverageEnable;
    state.alphaToOneEnable      = alphaToOneEnable;

    return setMultisampleState(state);
}

VkPipelineMultisampleStateCreateInfo Pipeline::multisampleState() const
{ return impl->multisampleState; }


Pipeline& Pipeline::setDepthStencilState(
    const VkPipelineDepthStencilStateCreateInfo& state)
{
    impl->depthStencilState = state;
    return *this;
}

Pipeline& Pipeline::setDepthStencilState(
    VkBool32 depthTestEnable,
    VkBool32 depthWriteEnable,
    VkCompareOp depthCompareOp,
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

VkPipelineDepthStencilStateCreateInfo Pipeline::depthStencilState() const
{ return impl->depthStencilState; }


// Sets  and gets the color blending state.
Pipeline& Pipeline::setColorBlendingState(
    const VkPipelineColorBlendStateCreateInfo& state)
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
    impl->blendAttachments = attachments;

    VkPipelineColorBlendStateCreateInfo state;
    state.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    state.pNext           = NULL;
    state.flags           = 0;
    state.logicOpEnable   = logicOpEnable;
    state.logicOp         = logicOp;
    state.attachmentCount = uint32_t(impl->blendAttachments.size());
    state.pAttachments    = impl->blendAttachments.data();
    for (int i = 0; i < 4; ++i)
        state.blendConstants[i]  = blendConstants[i];

    return setColorBlendingState(state);
}

VkPipelineColorBlendStateCreateInfo Pipeline::colorBlendingState() const
{ return impl->colorBlendState; }

Pipeline& Pipeline::setDynamicState(
    const VkPipelineDynamicStateCreateInfo& state)
{
    impl->dynamicState = state;
    return *this;
}

Pipeline& Pipeline::setDynamicState(const std::vector<VkDynamicState>& states)
{
    impl->dynamicStateStates = states;

    VkPipelineDynamicStateCreateInfo state;
    state.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    state.pNext             = NULL;
    state.flags             = 0;
    state.dynamicStateCount = uint32_t(impl->dynamicStateStates.size());
    state.pDynamicStates    = impl->dynamicStateStates.data();

    return setDynamicState(state);
}

VkPipelineDynamicStateCreateInfo Pipeline::dynamicState() const
{ return impl->dynamicState; }

Pipeline& Pipeline::setPipelineLayout(const VkPipelineLayoutCreateInfo& layout)
{
    impl->layoutInfo = layout;
    return *this;
}

Pipeline& Pipeline::setPipelineLayout(
    const std::vector<VkDescriptorSetLayout>& layouts,
    const std::vector<VkPushConstantRange>& pushConstantRanges)
{
    impl->layoutDescriptorlayouts  = layouts;
    impl->layoutPushConstantRanges = pushConstantRanges;

    VkPipelineLayoutCreateInfo info;
    info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext                  = NULL;
    info.flags                  = 0;
    info.setLayoutCount         = uint32_t(impl->layoutDescriptorlayouts.size());
    info.pSetLayouts            = impl->layoutDescriptorlayouts.data();
    info.pushConstantRangeCount = uint32_t(impl->layoutPushConstantRanges.size());
    info.pPushConstantRanges    = impl->layoutPushConstantRanges.data();
    return setPipelineLayout(info);
}

VkPipelineLayoutCreateInfo Pipeline::pipelineLayout() const
{ return impl->layoutInfo; }

Pipeline& Pipeline::setRenderPass(const VkRenderPass& renderPass)
{
    impl->renderPass = renderPass;
    return *this;
}

VkRenderPass Pipeline::renderPass() const
{ return impl->renderPass; }

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
{
    return impl->pipelineLayout != VK_NULL_HANDLE &&
           impl->pipeline       != VK_NULL_HANDLE;
}

VkPipeline Pipeline::handle() const
{ return impl->pipeline; }

VkPipelineLayout Pipeline::pipelineLayoutHandle() const
{ return impl->pipelineLayout; }

} // namespace vk
} // namespace kuu
