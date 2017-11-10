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
    {
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

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable   = VK_FALSE;
        multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading      = 1.0f;
        multisampling.pSampleMask           = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable      = VK_FALSE;

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

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount         = 0;
        pipelineLayoutInfo.pSetLayouts            = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges    = 0;

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

        /* ------------------------------------------------------------ *
           Viewport state.
         * ------------------------------------------------------------ */

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

        /* ------------------------------------------------------------ *
           Render pass

            * framebuffer attachments
            * framebuffer content handling

         * ------------------------------------------------------------ */

        // Create the description of the colorbuffer attachment. The
        // colorbuffer is an image in the swap chain.
        //      * no multisampling
        //      * clear the content before rendering
        //      * keep the contetn after rendering
        //      * do not care stencil operations
        //      * do not create what is the pixel layout before rendering
        //        as it is going to be cleared
        //      * set pixel layout to be presentation format after
        //        rendering as its going to be display on the screen
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format         = surface.format().format;
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Attachment descriptions
        const VkAttachmentDescription attachmentDescriptions[] =
        { colorAttachment };

        // Graphics attachment reference for subpass which is the color
        // attachment above. Use the optimal layout for color attachment
        // as thats what it is.
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Subpass that renders into colorbuffer attachment.
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments    = &colorAttachmentRef;

        // Add a subpass dependecy to prevent the image layout transition
        // being run too early. Make render pass to wait by waiting for
        // the color attachment output stage.
        VkSubpassDependency dependency = {};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // Fill the render pass info.
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments    = attachmentDescriptions;
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        // Create the render pass.
        result = vkCreateRenderPass(device.handle(), &renderPassInfo,
                                    nullptr, &renderPass);
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create vulkan render pass"
                      << std::endl;

            return;
        }

        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        bindingDescriptions.push_back(params.mesh.bindingDescription());
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        for (auto desc : params.mesh.attributeDescriptions())
            attributeDescriptions.push_back(desc);

        // Vertex input
        VkPipelineVertexInputStateCreateInfo vertexInput = {};
        vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount   = uint32_t(bindingDescriptions.size());
        vertexInput.pVertexBindingDescriptions      = bindingDescriptions.data();
        vertexInput.vertexAttributeDescriptionCount = uint32_t(attributeDescriptions.size());
        vertexInput.pVertexAttributeDescriptions    = attributeDescriptions.data();

        // Input assemply state
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        auto shaderStages = params.shaderStage.shaderStages();

        // Graphics pipeline
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
        pipelineInfo.renderPass          = renderPass;
        pipelineInfo.subpass             = 0;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex   = -1;


        result = vkCreateGraphicsPipelines(device.handle(),
                                           VK_NULL_HANDLE,
                                           1,
                                           &pipelineInfo,
                                           nullptr,
                                           &graphicsPipeline);

    }

    ~Data()
    {
        vkDestroyPipeline(device.handle(), graphicsPipeline, nullptr);
        vkDestroyRenderPass(device.handle(), renderPass, nullptr);
        vkDestroyPipelineLayout(device.handle(), pipelineLayout, nullptr);
    }

    Device device;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
};

/* ---------------------------------------------------------------- */

Pipeline::Pipeline(const Device& device,
                   const Surface& surface,
                   const Parameters& params)
    : d(std::make_shared<Data>(device, surface, params))
{}

VkPipelineLayout Pipeline::pipelineLayout() const
{
    return d->pipelineLayout;
}

VkRenderPass Pipeline::renderPass() const
{
    return d->renderPass;
}

VkPipeline Pipeline::graphicsPipeline() const
{
    return d->graphicsPipeline;
}

} // namespace vk
} // namespace kuu
