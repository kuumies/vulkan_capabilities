/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Pipeline class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A vulkan pipeline wrapper class
 * -------------------------------------------------------------------------- */
class Pipeline
{
public:
    // Constructs the pipeline.
    Pipeline(const VkDevice& logicalDevice);

    // Adds a shader stage
    Pipeline& addShaderStage(VkPipelineShaderStageCreateInfo stage);
    // Returns the shader stages.
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages() const;

    // Sets and gets the vertex input state.
    Pipeline& setVertexInputState(const VkPipelineVertexInputStateCreateInfo& state);
    Pipeline& setVertexInputState(
        const std::vector<VkVertexInputBindingDescription>& bindingDescs,
        const std::vector<VkVertexInputAttributeDescription>& attributeDescs);
    VkPipelineVertexInputStateCreateInfo vertexInputState() const;

    // Sets and gets the input assembly state.
    Pipeline& setInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo& state);
    Pipeline& setInputAssemblyState(
        VkPrimitiveTopology topology,
        VkBool32 primitiveRestartEnable);
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState() const;

    // Sets and gets the viewport state.
    Pipeline& setViewportState(const VkPipelineViewportStateCreateInfo& state);
    Pipeline& setViewportState(
        const std::vector<VkViewport>& viewports,
        const std::vector<VkRect2D>& scissors);
    VkPipelineViewportStateCreateInfo viewportState() const;

    // Sets and gets the rasterized state.
    Pipeline& setRasterizerState(const VkPipelineRasterizationStateCreateInfo& state);
    Pipeline& setRasterizerState(
        VkPolygonMode polygonMode,
        VkCullModeFlags cullMode,
        VkFrontFace frontFace,
        VkBool32 depthClampEnable = VK_FALSE,
        VkBool32 rasterizerDiscardEnable = VK_FALSE,
        VkBool32 depthBiasEnable = VK_FALSE,
        float depthBiasConstantFactor = 0.0f,
        float depthBiasClamp = 0.0f,
        float depthBiasSlopeFactor = 0.0f,
        float lineWidth = 1.0f);
    VkPipelineRasterizationStateCreateInfo rasterizeState() const;

    // Sets and gets the multisample state.
    Pipeline& setMultisampleState(const VkPipelineMultisampleStateCreateInfo& state);
    Pipeline& setMultisampleState(
        VkBool32 sampleShadingEnable,
        VkSampleCountFlagBits rasterizationSamples,
        float minSampleShading = 1.0f,
        const VkSampleMask* sampleMask = NULL,
        VkBool32 alphaToCoverageEnable = VK_FALSE,
        VkBool32 alphaToOneEnable = VK_FALSE);
    VkPipelineMultisampleStateCreateInfo multisampleState() const;

    // Sets and gets the depth/stencil state.
    Pipeline& setDepthStencilState(const VkPipelineDepthStencilStateCreateInfo& state);
    Pipeline& setDepthStencilState(
        VkBool32 depthTestEnable,
        VkBool32 depthWriteEnable = VK_TRUE,
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        VkBool32 depthBoundsTestEnable = VK_FALSE,
        VkBool32 stencilTestEnable = VK_FALSE,
        VkStencilOpState front = {},
        VkStencilOpState back = {},
        float minDepthBounds = 0.0f,
        float maxDepthBounds = 0.0f);
    VkPipelineDepthStencilStateCreateInfo depthStencilState() const;

    // Sets  and gets the color blending state.
    Pipeline& setColorBlendingState(const VkPipelineColorBlendStateCreateInfo& state);
    Pipeline& setColorBlendingState(
        VkBool32 logicOpEnable,
        VkLogicOp logicOp,
        const std::vector<VkPipelineColorBlendAttachmentState> attachments,
        float blendConstants[4]);
    VkPipelineColorBlendStateCreateInfo colorBlendingState() const;

    // Sets and gets the dynamic state.
    Pipeline& setDynamicState(const VkPipelineDynamicStateCreateInfo& state);
    Pipeline& setDynamicState(const std::vector<VkDynamicState>& states);
    VkPipelineDynamicStateCreateInfo dynamicState() const;

    // Sets and gets the pipeline layout.
    Pipeline& setPipelineLayout(const VkPipelineLayoutCreateInfo& layout);
    Pipeline& setPipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts,
                                const std::vector<VkPushConstantRange>& pushConstantRanges);
    VkPipelineLayoutCreateInfo pipelineLayout() const;

    // Sets and gets the render pass.
    Pipeline& setRenderPass(const VkRenderPass& renderPass);
    VkRenderPass renderPass() const;

    // Creates and destroys the pipeline.
    void create();
    void destroy();

    // Returns true if the pipeline handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the handle.
    VkPipeline handle() const;
    VkPipelineLayout pipelineLayoutHandle() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
