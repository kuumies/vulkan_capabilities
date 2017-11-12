/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Pipeline class
 * ---------------------------------------------------------------- */

#include "vk_pipeline.h"

/* ---------------------------------------------------------------- */

#include <iostream>

/* ---------------------------------------------------------------- */

#include "vk_surface.h"
#include "vk_device.h"
#include "vk_render_pass.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct Pipeline::Data
{
    Data(const Device& device,
         const Surface& surface,
         const Parameters& params)
        : device(device)
        , renderPass(device, surface, { params.viewportWidth, 
                                        params.viewportHeight, 
                                        params.swapChain })
    {
        /* -------------------------------------------------------- *
           Render pass
         * -------------------------------------------------------- */

        if (!renderPass.isValid())
            return;

        /* -------------------------------------------------------- *
           Pipeline layout
         * -------------------------------------------------------- */

        // Fill the pipeline layout info
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount         = 0;
        pipelineLayoutInfo.pSetLayouts            = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges    = 0;

        // Creat the pipeline layout
        VkResult result = vkCreatePipelineLayout(
            device.handle(),
            &pipelineLayoutInfo,
            nullptr,
            &pipelineLayout);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create vulkan pipeline layout"
                      << std::endl;

            return;
        }

        /* -------------------------------------------------------- *
           Rasterization state
         * -------------------------------------------------------- */

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable        = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth               = 1.0f;
        rasterizer.cullMode                = params.cullMode;;
        rasterizer.frontFace               = params.frontFace;
        rasterizer.depthBiasEnable         = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp          = 0.0f;
        rasterizer.depthBiasSlopeFactor    = 0.0f;

        /* -------------------------------------------------------- *
           Multisampling
         * -------------------------------------------------------- */
        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable   = VK_FALSE;
        multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading      = 1.0f;
        multisampling.pSampleMask           = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable      = VK_FALSE;

        /* -------------------------------------------------------- *
           Color blending
         * -------------------------------------------------------- */

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable         = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable     = VK_FALSE;
        colorBlending.logicOp           = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount   = 1;
        colorBlending.pAttachments      = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        /* -------------------------------------------------------- *
           Viewport
         * -------------------------------------------------------- */

        // Viewport
        VkViewport viewport = {};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = float(params.viewportWidth);
        viewport.height   = float(params.viewportHeight);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        // Scissor
        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = VkExtent2D { params.viewportWidth,
                                      params.viewportHeight };

        // Viewport state.
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports    = &viewport;
        viewportState.scissorCount  = 1;
        viewportState.pScissors     = &scissor;

        /* -------------------------------------------------------- *
           Vertex input.
         * -------------------------------------------------------- */

        // Create binding and attribute descriptions.
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        bindingDescriptions.push_back(params.mesh.bindingDescription());

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        for (auto desc : params.mesh.attributeDescriptions())
            attributeDescriptions.push_back(desc);

        // Fill vertex input state info.
        VkPipelineVertexInputStateCreateInfo vertexInput = {};
        vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount   = uint32_t(bindingDescriptions.size());
        vertexInput.pVertexBindingDescriptions      = bindingDescriptions.data();
        vertexInput.vertexAttributeDescriptionCount = uint32_t(attributeDescriptions.size());
        vertexInput.pVertexAttributeDescriptions    = attributeDescriptions.data();

        /* -------------------------------------------------------- *
           Input assembly
         * -------------------------------------------------------- */

        // Fill input assemply state
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        /* -------------------------------------------------------- *
           Shaders
         * -------------------------------------------------------- */

        auto shaderStages = params.shaderStage.shaderStages();

        /* -------------------------------------------------------- *
           Graphics pipeline
         * -------------------------------------------------------- */

        // Fill graphics pipeline info
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = uint32_t(shaderStages.size());
        pipelineInfo.pStages             = shaderStages.data();
        pipelineInfo.pVertexInputState   = &vertexInput;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState   = &multisampling;
        pipelineInfo.pDepthStencilState  = nullptr;
        pipelineInfo.pColorBlendState    = &colorBlending;
        pipelineInfo.pDynamicState       = nullptr;
        pipelineInfo.layout              = pipelineLayout;
        pipelineInfo.renderPass          = renderPass.handle();
        pipelineInfo.subpass             = 0;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex   = -1;

        // Create the graphics pipeline
        result = vkCreateGraphicsPipelines(
            device.handle(),
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &pipeline);

    }

    ~Data()
    {
        // Destrys the pipeline.
        vkDestroyPipeline(device.handle(), pipeline, nullptr);
        // Destroys the pipeline layout.
        vkDestroyPipelineLayout(
            device.handle(), 
            pipelineLayout, 
            nullptr);
    }

    // Logical device
    Device device;
    // Render pass
    RenderPass renderPass;
    // Handle of pipeline layout
    VkPipelineLayout pipelineLayout;
    // Handle of pipeline
    VkPipeline pipeline;
};

/* ---------------------------------------------------------------- */

Pipeline::Pipeline(const Device& device,
                   const Surface& surface,
                   const Parameters& params)
    : d(std::make_shared<Data>(device, surface, params))
{}

/* ---------------------------------------------------------------- */

RenderPass Pipeline::renderPass() const
{ return d->renderPass; }

/* ---------------------------------------------------------------- */

VkPipeline Pipeline::handle() const
{ return d->pipeline; }

/* ---------------------------------------------------------------- */

void Pipeline::bind(VkCommandBuffer buf)
{
    vkCmdBindPipeline(
        buf,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        d->pipeline);
}

} // namespace vk
} // namespace kuu
