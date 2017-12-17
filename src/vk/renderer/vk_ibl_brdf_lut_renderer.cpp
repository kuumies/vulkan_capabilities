/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::IblBrdfLutRenderer class
 * -------------------------------------------------------------------------- */

#pragma once

#include "vk_ibl_brdf_lut_renderer.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "../vk_buffer.h"
#include "../vk_command.h"
#include "../vk_descriptor_set.h"
#include "../vk_image.h"
#include "../vk_mesh.h"
#include "../vk_pipeline.h"
#include "../vk_queue.h"
#include "../vk_image.h"
#include "../vk_image_layout_transition.h"
#include "../vk_render_pass.h"
#include "../vk_shader_module.h"
#include "../vk_texture.h"
#include "../../common/mesh.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct IblBrdfLutRenderer::Impl
{
    // Input Vulkan handles
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    // Graphics queue.
    uint32_t graphicsQueueFamilyIndex;

    // Texture
    std::shared_ptr<Texture2D> texture;

    // Dimensions of the texture cube.
    VkExtent3D extent;

    // Format of the texture cube.
    VkFormat format;
};

/* -------------------------------------------------------------------------- */

IblBrdfLutRenderer::IblBrdfLutRenderer(
        const VkPhysicalDevice& physicalDevice,
        const VkDevice& device,
        const uint32_t& graphicsQueueFamilyIndex)
    : impl(std::make_shared<Impl>())
{
    impl->physicalDevice           = physicalDevice;
    impl->device                   = device;
    impl->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    impl->format                   = VK_FORMAT_R16G16_SFLOAT;
    impl->extent                   = { uint32_t(512), uint32_t(512), uint32_t(1) };
    impl->texture = std::make_shared<Texture2D>(
        physicalDevice,
        device,
        VkExtent2D( { impl->extent.width, impl->extent.height } ),
        impl->format,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

void IblBrdfLutRenderer::render()
{
    //--------------------------------------------------------------------------
    // Quad mesh in NDC space.

    std::vector<Vertex> vertices =
    {
        { { 1,  1, 0 }, { 1, 1 } },
        { {-1,  1, 0 }, { 0, 1 } },
        { {-1, -1, 0 }, { 0, 0 } },
        { { 1,  1, 0 }, { 1, 1 } },
        { {-1, -1, 0 }, { 0, 0 } },
        { { 1, -1, 0 }, { 1, 0 } },
    };

    kuu::Mesh m;
    for(const Vertex& v : vertices)
        m.addVertex(v);

    std::vector<float> vertexVector;
    for (const Vertex& v : m.vertices)
    {
        vertexVector.push_back(v.pos.x);
        vertexVector.push_back(v.pos.y);
        vertexVector.push_back(v.pos.z);
        vertexVector.push_back(v.texCoord.x);
        vertexVector.push_back(v.texCoord.y);
    }

    std::shared_ptr<Mesh> mesh =
        std::make_shared<Mesh>(
            impl->physicalDevice,
            impl->device);

    mesh->setVertices(vertexVector);
    mesh->setIndices(m.indices);
    mesh->addVertexAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
    mesh->addVertexAttributeDescription(1, 0, VK_FORMAT_R32G32_SFLOAT,    3 * sizeof(float));
    mesh->setVertexBindingDescription(0, 5 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX);
    if (!mesh->create())
        return;

    //--------------------------------------------------------------------------
    // Shader modules.

    const std::string vshFilePath = "shaders/pbr_ibl_brdf_lut.vert.spv";
    const std::string fshFilePath = "shaders/pbr_ibl_brdf_lut.frag.spv";

    std::shared_ptr<ShaderModule> vshModule =
        std::make_shared<ShaderModule>(impl->device, vshFilePath);
    vshModule->setStageName("main");
    vshModule->setStage(VK_SHADER_STAGE_VERTEX_BIT);
    if (!vshModule->create())
        return;

    std::shared_ptr<ShaderModule> fshModule =
        std::make_shared<ShaderModule>(impl->device, fshFilePath);
    fshModule->setStageName("main");
    fshModule->setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
    if (!fshModule->create())
        return;

    //--------------------------------------------------------------------------
    // Render pass

    // Colorbuffer attachment description
    VkAttachmentDescription colorAttachment;
    colorAttachment.flags          = 0;
    colorAttachment.format         = impl->format;
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Reference to first attachment (color)
    VkAttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpass;
    subpass.flags                   = 0;
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount    = 0;
    subpass.pInputAttachments       = NULL;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pResolveAttachments     = NULL;
    subpass.pDepthStencilAttachment = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments    = NULL;

    // Subpass dependency
    VkSubpassDependency dependency;
    dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass      = 0;
    dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask   = 0;
    dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo rpInfo;
    rpInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.pNext           = NULL;
    rpInfo.flags           = 0;
    rpInfo.attachmentCount = 1;
    rpInfo.pAttachments    = &colorAttachment;
    rpInfo.subpassCount    = 1;
    rpInfo.pSubpasses      = &subpass;
    rpInfo.dependencyCount = 1;
    rpInfo.pDependencies   = &dependency;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkResult result =
        vkCreateRenderPass(
            impl->device,
            &rpInfo,
            NULL,
            &renderPass);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": render pass creation failed"
                  << std::endl;
        return;
    }

    std::vector<VkImageView> attachments =
    { impl->texture->imageView };

    VkFramebufferCreateInfo fbInfo;
    fbInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.pNext           = NULL;
    fbInfo.flags           = 0;
    fbInfo.renderPass      = renderPass;
    fbInfo.attachmentCount = uint32_t(attachments.size());
    fbInfo.pAttachments    = attachments.data();
    fbInfo.width           = impl->extent.width;
    fbInfo.height          = impl->extent.height;
    fbInfo.layers          = 1;

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    result = vkCreateFramebuffer(
            impl->device,
            &fbInfo,
            NULL,
            &framebuffer);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": framebuffer creation failed"
                  << std::endl;
        return;
    }

    //--------------------------------------------------------------------------
    // Pipeline

    VkPipelineColorBlendAttachmentState colorBlend = {};
    colorBlend.blendEnable    = VK_FALSE;
    colorBlend.colorWriteMask = 0xf;

    float blendConstants[4] = { 0, 0, 0, 0 };

    std::shared_ptr<Pipeline> pipeline =
        std::make_shared<Pipeline>(impl->device);
    pipeline->addShaderStage(vshModule->createInfo());
    pipeline->addShaderStage(fshModule->createInfo());
    pipeline->setVertexInputState(
        { mesh->vertexBindingDescription() },
          mesh->vertexAttributeDescriptions() );
    pipeline->setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
    pipeline->setViewportState(
        { { 0, 0, float(impl->extent.width), float(impl->extent.height), 0, 1 } },
        { { { 0, 0 }, { impl->extent.width, impl->extent.height }  } } );
    pipeline->setRasterizerState(
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pipeline->setMultisampleState(VK_FALSE, VK_SAMPLE_COUNT_1_BIT);
    pipeline->setDepthStencilState(VK_FALSE, VK_FALSE);
    pipeline->setColorBlendingState(
            VK_FALSE,
            VK_LOGIC_OP_CLEAR,
            { colorBlend },
            blendConstants);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

    const std::vector<VkPushConstantRange> pushConstantRanges;
    pipeline->setPipelineLayout(descriptorSetLayouts, pushConstantRanges);
    pipeline->setDynamicState( { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } );
    pipeline->setRenderPass(renderPass);
    if (!pipeline->create())
        return;

    //--------------------------------------------------------------------------
    // Command pool

    std::shared_ptr<CommandPool> graphicsCommandPool =
        std::make_shared<CommandPool>(impl->device);
    graphicsCommandPool->setQueueFamilyIndex(impl->graphicsQueueFamilyIndex);
    if (!graphicsCommandPool->create())
        return;

    //--------------------------------------------------------------------------
    // Record commands

    VkCommandBuffer cmdBuf =
        graphicsCommandPool->allocateBuffer(
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext            = NULL;
    beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = NULL;

    vkBeginCommandBuffer(cmdBuf, &beginInfo);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.levelCount  = 1;
    subresourceRange.layerCount  = 1;

    image_layout_transition::record(
        cmdBuf,
        impl->texture->image,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    std::vector<VkClearValue> clearValues(1);
    clearValues[0].color        = { 0.1f, 0.1f, 0.1f, 1.0f };

    VkViewport viewport;
    viewport.x        = 0;
    viewport.y        = 0;
    viewport.width    = float(impl->extent.width);
    viewport.height   = float(impl->extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset = {};
    scissor.extent = { impl->extent.width, impl->extent.height };
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    VkRenderPassBeginInfo renderPassInfo;
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext             = NULL;
    renderPassInfo.renderPass        = renderPass;
    renderPassInfo.framebuffer       = framebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { impl->extent.width, impl->extent.height };
    renderPassInfo.clearValueCount   = uint32_t(clearValues.size());
    renderPassInfo.pClearValues      = clearValues.data();

    vkCmdBeginRenderPass(
        cmdBuf,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(
        cmdBuf,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline->handle());

    const VkBuffer vertexBuffer = mesh->vertexBufferHandle();
    const VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers(
        cmdBuf, 0, 1,
        &vertexBuffer,
        offsets);

    const VkBuffer indexBuffer = mesh->indexBufferHandle();
    vkCmdBindIndexBuffer(
        cmdBuf,
        indexBuffer,
        0, VK_INDEX_TYPE_UINT32);

    uint32_t indexCount = uint32_t(mesh->indices().size());
    vkCmdDrawIndexed(
        cmdBuf,
        indexCount,
        1, 0, 0, 1);

    vkCmdEndRenderPass(cmdBuf);

    image_layout_transition::record(
        cmdBuf,
        impl->texture->image,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    result = vkEndCommandBuffer(cmdBuf);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": render commands failed"
                  << std::endl;
        return;
    }

    //--------------------------------------------------------------------------
    // Render

    Queue graphicsQueue(impl->device, impl->graphicsQueueFamilyIndex, 0);
    graphicsQueue.create();
    graphicsQueue.submit(cmdBuf,
                         VK_NULL_HANDLE,
                         VK_NULL_HANDLE,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    graphicsQueue.waitIdle();

    //--------------------------------------------------------------------------
    // Clean up

    vkDestroyFramebuffer(
        impl->device,
        framebuffer,
        NULL);

    vkDestroyRenderPass(
        impl->device,
        renderPass,
        NULL);
}

std::shared_ptr<Texture2D> IblBrdfLutRenderer::texture() const
{ return impl->texture; }

} // namespace vk
} // namespace kuu
