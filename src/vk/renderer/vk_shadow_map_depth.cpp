/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::ShadowMapDepth class
 * -------------------------------------------------------------------------- */

#pragma once

#include "vk_shadow_map_depth.h"
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
#include "../vk_stringify.h"
#include "../vk_texture.h"
#include "../../common/mesh.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct ShadowMapDepth::Impl
{
    ~Impl()
    {
        vkDestroyDescriptorSetLayout(
            device,
            descriptorSetLayout,
            NULL);
    }

    // Input Vulkan handles
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    // Graphics queue.
    uint32_t graphicsQueueFamilyIndex;

    // Texture
    std::shared_ptr<Texture2D> texture;

    // For pipeline
    VkRenderPass renderPass;
    VkExtent3D extent;

    // Descriptor sets
    std::shared_ptr<DescriptorSets> descriptorSets;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    std::shared_ptr<DescriptorPool> descriptorPool;

    std::shared_ptr<ShaderModule> vshModule;
    std::shared_ptr<ShaderModule> fshModule;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<Mesh> mesh;
};

/* -------------------------------------------------------------------------- */

ShadowMapDepth::ShadowMapDepth(
        const VkPhysicalDevice& physicalDevice,
        const VkDevice& device,
        const uint32_t& graphicsQueueFamilyIndex,
        const VkRenderPass& renderPass)
    : impl(std::make_shared<Impl>())
{
    impl->physicalDevice           = physicalDevice;
    impl->device                   = device;
    impl->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    impl->extent                   = { uint32_t(512), uint32_t(512), uint32_t(1) };
    impl->renderPass               = renderPass;

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

    impl->mesh =
        std::make_shared<Mesh>(
            impl->physicalDevice,
            impl->device);

    impl->mesh->setVertices(vertexVector);
    impl->mesh->setIndices(m.indices);
    impl->mesh->addVertexAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
    impl->mesh->addVertexAttributeDescription(1, 0, VK_FORMAT_R32G32_SFLOAT,    3 * sizeof(float));
    impl->mesh->setVertexBindingDescription(0, 5 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX);
    if (!impl->mesh->create())
        return;

    //--------------------------------------------------------------------------
    // Shader modules.

    const std::string vshFilePath = "shaders/shadow_map_depth.vert.spv";
    const std::string fshFilePath = "shaders/shadow_map_depth.frag.spv";

    impl->vshModule = std::make_shared<ShaderModule>(impl->device, vshFilePath);
    impl->vshModule->setStageName("main");
    impl->vshModule->setStage(VK_SHADER_STAGE_VERTEX_BIT);
    if (!impl->vshModule->create())
        return;

    impl->fshModule = std::make_shared<ShaderModule>(impl->device, fshFilePath);
    impl->fshModule->setStageName("main");
    impl->fshModule->setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
    if (!impl->fshModule->create())
        return;

    // -------------------------------------------------------------------------
    // Descriptor sets.

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings =
    { { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,   NULL } };

    VkDescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext        = NULL;
    layoutInfo.flags        = 0;
    layoutInfo.bindingCount = uint32_t(layoutBindings.size());
    layoutInfo.pBindings    = layoutBindings.data();

    VkResult result =
        vkCreateDescriptorSetLayout(
            device,
            &layoutInfo,
            NULL,
            &impl->descriptorSetLayout);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": descriptor set layout creation failed as "
                  << vk::stringify::resultDesc(result)
                  << std::endl;
        return;
    }

    impl->descriptorPool = std::make_shared<DescriptorPool>(impl->device);
    impl->descriptorPool->addTypeSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  1);
    impl->descriptorPool->setMaxCount(1);
    impl->descriptorPool->create();

    impl->descriptorSets = std::make_shared<DescriptorSets>(
        impl->device,
        impl->descriptorPool->handle());
    impl->descriptorSets->setLayout(impl->descriptorSetLayout);
    impl->descriptorSets->create();

    //--------------------------------------------------------------------------
    // Pipeline

    VkPipelineColorBlendAttachmentState colorBlend = {};
    colorBlend.blendEnable    = VK_FALSE;
    colorBlend.colorWriteMask = 0xf;

    float blendConstants[4] = { 0, 0, 0, 0 };

    impl->pipeline = std::make_shared<Pipeline>(impl->device);
    impl->pipeline->addShaderStage(impl->vshModule->createInfo());
    impl->pipeline->addShaderStage(impl->fshModule->createInfo());
    impl->pipeline->setVertexInputState(
        { impl->mesh->vertexBindingDescription() },
          impl->mesh->vertexAttributeDescriptions() );
    impl->pipeline->setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
    impl->pipeline->setViewportState(
        { { 0, 0, float(impl->extent.width), float(impl->extent.height), 0, 1 } },
        { { { 0, 0 }, { impl->extent.width, impl->extent.height }  } } );
    impl->pipeline->setRasterizerState(
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE);
    impl->pipeline->setMultisampleState(VK_FALSE, VK_SAMPLE_COUNT_1_BIT);
    impl->pipeline->setDepthStencilState(VK_FALSE, VK_FALSE);
    impl->pipeline->setColorBlendingState(
            VK_FALSE,
            VK_LOGIC_OP_CLEAR,
            { colorBlend },
            blendConstants);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts { impl->descriptorSetLayout };

    const std::vector<VkPushConstantRange> pushConstantRanges;
    impl->pipeline->setPipelineLayout(descriptorSetLayouts, pushConstantRanges);
    impl->pipeline->setDynamicState( { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } );
    impl->pipeline->setRenderPass(renderPass);
    if (!impl->pipeline->create())
        return;
}

void ShadowMapDepth::recordCommands(const VkCommandBuffer& cmdBuf)
{
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
    scissor.extent.width  = impl->extent.width;
    scissor.extent.height = impl->extent.height;
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    VkDescriptorSet descriptorHandle = impl->descriptorSets->handle();
    VkPipelineLayout pipelineLayout  = impl->pipeline->pipelineLayoutHandle();
    vkCmdBindDescriptorSets(
        cmdBuf,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1,
        &descriptorHandle, 0, NULL);

    vkCmdBindPipeline(
        cmdBuf,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        impl->pipeline->handle());

    const VkBuffer vertexBuffer = impl->mesh->vertexBufferHandle();
    const VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers(
        cmdBuf, 0, 1,
        &vertexBuffer,
        offsets);

    const VkBuffer indexBuffer = impl->mesh->indexBufferHandle();
    vkCmdBindIndexBuffer(
        cmdBuf,
        indexBuffer,
        0, VK_INDEX_TYPE_UINT32);

    uint32_t indexCount = uint32_t(impl->mesh->indices().size());
    vkCmdDrawIndexed(
        cmdBuf,
        indexCount,
        1, 0, 0, 1);
}

void ShadowMapDepth::setShadowMap(std::shared_ptr<Texture2D> texture)
{
    impl->texture = texture;
    impl->descriptorSets->writeImage(
        0,
        impl->texture->sampler,
        impl->texture->imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

} // namespace vk
} // namespace kuu
