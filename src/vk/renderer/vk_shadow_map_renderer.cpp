/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::ShadowMapRenderer class
 * -------------------------------------------------------------------------- */

#pragma once

#include "vk_shadow_map_renderer.h"
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
#include "../../common/camera.h"
#include "../../common/light.h"
#include "../../common/mesh.h"
#include "../../common/model.h"
#include "../../common/scene.h"
#include "vk_mesh_manager.h"

namespace kuu
{
namespace vk
{
namespace
{

/* -------------------------------------------------------------------------- */

struct Matrices
{
    glm::mat4 model;
    glm::mat4 light;
};

/* -------------------------------------------------------------------------- *
   A model for shadow mapping.
 * -------------------------------------------------------------------------- */
struct ShadowMapModel
{
    ShadowMapModel(const VkPhysicalDevice& physicalDevice,
             const VkDevice& device,
             const VkDescriptorSetLayout& descriptorSetLayout,
             const VkDescriptorPool& descriptorPool,
             std::shared_ptr<Model> model,
             std::shared_ptr<MeshManager> meshManager)
        : model(model)
    {
        // ---------------------------------------------------------------------
        // Mesh

        mesh = meshManager->mesh(model->mesh);

        // ---------------------------------------------------------------------
        // Descriptor sets.

        descriptorSets = std::make_shared<DescriptorSets>(device, descriptorPool);
        descriptorSets->setLayout(descriptorSetLayout);
        descriptorSets->create();

        // ---------------------------------------------------------------------
        // Uniform buffers

        matricesUniformBuffer = std::make_shared<Buffer>(physicalDevice, device);
        matricesUniformBuffer->setSize(sizeof(Matrices));
        matricesUniformBuffer->setUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        matricesUniformBuffer->setMemoryProperties(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        matricesUniformBuffer->create();

        descriptorSets->writeUniformBuffer(
            0, matricesUniformBuffer->handle(),
            0, matricesUniformBuffer->size());
    }

    // Model
    std::shared_ptr<Model> model;

    // Mesh
    std::shared_ptr<Mesh> mesh;

    // Uniform buffers
    std::shared_ptr<Buffer> matricesUniformBuffer;

    // Descriptor sets
    std::shared_ptr<DescriptorSets> descriptorSets;
};

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct ShadowMapRenderer::Impl
{
    Impl(const VkPhysicalDevice& physicalDevice,
         const VkDevice& device,
         const uint32_t& graphicsQueueFamilyIndex,
         std::shared_ptr<MeshManager> meshManager)
        : physicalDevice(physicalDevice)
        , device(device)
        , graphicsQueueFamilyIndex(graphicsQueueFamilyIndex)
        , meshManager(meshManager)
    {
        format  = VK_FORMAT_D16_UNORM;
        extent  = { uint32_t(1024), uint32_t(1024), uint32_t(1) };

        texture = std::make_shared<Texture2D>(
            physicalDevice,
            device,
            VkExtent2D( { extent.width, extent.height } ),
            format,
            VK_FILTER_LINEAR,
            VK_FILTER_LINEAR,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT
                | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT);

        //----------------------------------------------------------------------
        // Shader modules.

        const std::string vshFilePath = "shaders/shadow_map.vert.spv";
        const std::string fshFilePath = "shaders/shadow_map.frag.spv";

        vshModule = std::make_shared<ShaderModule>(device, vshFilePath);
        vshModule->setStageName("main");
        vshModule->setStage(VK_SHADER_STAGE_VERTEX_BIT);
        if (!vshModule->create())
            return;

//        fshModule = std::make_shared<ShaderModule>(device, fshFilePath);
//        fshModule->setStageName("main");
//        fshModule->setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
//        if (!fshModule->create())
//            return;

        //--------------------------------------------------------------------------
        // Render pass

        // Depthbuffer attachment description
        VkAttachmentDescription depthAttachment;
        depthAttachment.flags          = 0;
        depthAttachment.format         = format;
        depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Reference to first attachment (depth)
        VkAttachmentReference depthAttachmentRef;
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Subpass
        VkSubpassDescription subpass;
        subpass.flags                   = 0;
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount    = 0;
        subpass.pInputAttachments       = NULL;
        subpass.colorAttachmentCount    = 0;
        subpass.pColorAttachments       = NULL;
        subpass.pResolveAttachments     = NULL;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments    = NULL;

        // Subpass dependency
        VkSubpassDependency dependency;
        dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass      = 0;
        dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask   = 0;
        dependency.dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = 0;

        VkRenderPassCreateInfo rpInfo;
        rpInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpInfo.pNext           = NULL;
        rpInfo.flags           = 0;
        rpInfo.attachmentCount = 1;
        rpInfo.pAttachments    = &depthAttachment;
        rpInfo.subpassCount    = 1;
        rpInfo.pSubpasses      = &subpass;
        rpInfo.dependencyCount = 1;
        rpInfo.pDependencies   = &dependency;

        VkResult result =
            vkCreateRenderPass(
                device,
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
        { texture->imageView };

        VkFramebufferCreateInfo fbInfo;
        fbInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.pNext           = NULL;
        fbInfo.flags           = 0;
        fbInfo.renderPass      = renderPass;
        fbInfo.attachmentCount = uint32_t(attachments.size());
        fbInfo.pAttachments    = attachments.data();
        fbInfo.width           = extent.width;
        fbInfo.height          = extent.height;
        fbInfo.layers          = 1;

        result = vkCreateFramebuffer(
                device,
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

        std::vector<VkVertexInputAttributeDescription> vertexAttributes =
        {
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0                  },
            { 1, 0, VK_FORMAT_R32G32_SFLOAT,    3 * sizeof(float)  },
            { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, 5 * sizeof(float)  },
            { 3, 0, VK_FORMAT_R32G32B32_SFLOAT, 8 * sizeof(float)  },
            { 4, 0, VK_FORMAT_R32G32B32_SFLOAT, 11 * sizeof(float) },
        };

        VkVertexInputBindingDescription vertexBindingDescription;
        vertexBindingDescription.binding   = 0;
        vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        vertexBindingDescription.stride    = 14 * sizeof(float);

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings =
        {
         { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
           1, VK_SHADER_STAGE_VERTEX_BIT,   NULL }
        };

     VkDescriptorSetLayoutCreateInfo layoutInfo;
     layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
     layoutInfo.pNext        = NULL;
     layoutInfo.flags        = 0;
     layoutInfo.bindingCount = uint32_t(layoutBindings.size());
     layoutInfo.pBindings    = layoutBindings.data();

     result = vkCreateDescriptorSetLayout(
             device,
             &layoutInfo,
             NULL,
             &descriptorSetLayout);

     if (result != VK_SUCCESS)
     {
         std::cerr << __FUNCTION__
                   << ": descriptor set layout creation failed as "
                   << vk::stringify::resultDesc(result)
                   << std::endl;
         return;
     }

        VkPipelineColorBlendAttachmentState colorBlend = {};
        colorBlend.blendEnable    = VK_FALSE;
        colorBlend.colorWriteMask = 0xf;

        float blendConstants[4] = { 0, 0, 0, 0 };

        pipeline = std::make_shared<Pipeline>(device);
        pipeline->addShaderStage(vshModule->createInfo());
        //pipeline->addShaderStage(fshModule->createInfo());
        pipeline->setVertexInputState(
            { vertexBindingDescription },
              vertexAttributes );
        pipeline->setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
        pipeline->setViewportState(
            { { 0, 0, float(extent.width), float(extent.height), 0, 1 } },
            { { { 0, 0 }, { extent.width, extent.height }  } } );
        pipeline->setRasterizerState(
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_NONE,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
            VK_FALSE,
            VK_FALSE,
            VK_TRUE);
        pipeline->setMultisampleState(VK_FALSE, VK_SAMPLE_COUNT_1_BIT);
        pipeline->setDepthStencilState(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
        pipeline->setColorBlendingState(
                VK_FALSE,
                VK_LOGIC_OP_CLEAR,
                {},
                blendConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { descriptorSetLayout };

        const std::vector<VkPushConstantRange> pushConstantRanges;
        pipeline->setPipelineLayout(descriptorSetLayouts, pushConstantRanges);
        pipeline->setDynamicState( { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS } );
        pipeline->setRenderPass(renderPass);
        if (!pipeline->create())
            return;

        //--------------------------------------------------------------------------
        // Command pool

        graphicsCommandPool = std::make_shared<CommandPool>(device);
        graphicsCommandPool->setQueueFamilyIndex(graphicsQueueFamilyIndex);
        if (!graphicsCommandPool->create())
            return;
    }

    ~Impl()
    {
        //----------------------------------------------------------------------
        // Clean up

        vkDestroyFramebuffer(
            device,
            framebuffer,
            NULL);

        vkDestroyRenderPass(
            device,
            renderPass,
            NULL);

        vkDestroyDescriptorSetLayout(
            device,
            descriptorSetLayout,
            NULL);
    }

    void recordCommands()
    {
        //--------------------------------------------------------------------------
        // Record commands

        cmdBuf = graphicsCommandPool->allocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext            = NULL;
        beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = NULL;

        vkBeginCommandBuffer(cmdBuf, &beginInfo);

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask  = VK_IMAGE_ASPECT_DEPTH_BIT;
        subresourceRange.levelCount  = 1;
        subresourceRange.layerCount  = 1;

        image_layout_transition::record(
            cmdBuf,
            texture->image,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        std::vector<VkClearValue> clearValues(1);
        clearValues[0].depthStencil = { 1.0, 0 };

        VkViewport viewport;
        viewport.x        = 0;
        viewport.y        = 0;
        viewport.width    = float(extent.width);
        viewport.height   = float(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

        VkRect2D scissor;
        scissor.offset = {};
        scissor.extent = { extent.width, extent.height };
        vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

        float depthBiasConstant = 1.25f;
        float depthBiasSlope = 1.75f;
        vkCmdSetDepthBias(
                    cmdBuf,
                    depthBiasConstant,
                    0.0f,
                    depthBiasSlope);

        VkRenderPassBeginInfo renderPassInfo;
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext             = NULL;
        renderPassInfo.renderPass        = renderPass;
        renderPassInfo.framebuffer       = framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { extent.width, extent.height };
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

        for (std::shared_ptr<ShadowMapModel> model : models)
        {
            VkDescriptorSet descriptorHandle = model->descriptorSets->handle();
            VkPipelineLayout pipelineLayout  = pipeline->pipelineLayoutHandle();
            vkCmdBindDescriptorSets(
                cmdBuf,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout, 0, 1,
                &descriptorHandle, 0, NULL);

            const VkBuffer vertexBuffer = model->mesh->vertexBufferHandle();
            const VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(
                cmdBuf, 0, 1,
                &vertexBuffer,
                offsets);

            const VkBuffer indexBuffer = model->mesh->indexBufferHandle();
            vkCmdBindIndexBuffer(
                cmdBuf,
                indexBuffer,
                0, VK_INDEX_TYPE_UINT32);

            uint32_t indexCount = uint32_t(model->mesh->indices().size());
            vkCmdDrawIndexed(
                cmdBuf,
                indexCount,
                1, 0, 0, 1);
        }

        vkCmdEndRenderPass(cmdBuf);

        image_layout_transition::record(
            cmdBuf,
            texture->image,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        VkResult result = vkEndCommandBuffer(cmdBuf);
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": render commands failed"
                      << std::endl;
            return;
        }
    }

    // Input Vulkan handles
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    // Graphics queue.
    uint32_t graphicsQueueFamilyIndex;

    // Texture
    std::shared_ptr<Texture2D> texture;

    // Dimensions of the texture.
    VkExtent3D extent;

    // Format of the texture.
    VkFormat format;

    // Scene
    std::shared_ptr<Scene> scene;
    std::shared_ptr<MeshManager> meshManager;
    std::vector<std::shared_ptr<ShadowMapModel>> models;

    // Internal objects
    std::shared_ptr<ShaderModule> vshModule;
    std::shared_ptr<ShaderModule> fshModule;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<CommandPool> graphicsCommandPool;
    VkCommandBuffer cmdBuf;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    std::shared_ptr<DescriptorPool> descriptorPool;
};

/* -------------------------------------------------------------------------- */

ShadowMapRenderer::ShadowMapRenderer(
        const VkPhysicalDevice& physicalDevice,
        const VkDevice& device,
        const uint32_t& graphicsQueueFamilyIndex,
        std::shared_ptr<MeshManager> meshManager)
    : impl(std::make_shared<Impl>(physicalDevice, device, graphicsQueueFamilyIndex, meshManager))
{}

/* -------------------------------------------------------------------------- */

void ShadowMapRenderer::setScene(std::shared_ptr<Scene> scene)
{
    std::vector<std::shared_ptr<Model>> pbrModels;
    for (std::shared_ptr<Model> m : scene->models)
        if (m->material->type == Material::Type::Pbr)
            pbrModels.push_back(m);

    uint32_t uniformBufferCount = 1 * uint32_t(pbrModels.size());
    impl->descriptorPool = std::make_shared<DescriptorPool>(impl->device);
    impl->descriptorPool->addTypeSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBufferCount);
    impl->descriptorPool->setMaxCount(uniformBufferCount);
    impl->descriptorPool->create();

    impl->scene = scene;
    for (std::shared_ptr<Model> m : pbrModels)
        impl->models.push_back(
            std::make_shared<ShadowMapModel>(
                impl->physicalDevice,
                impl->device,
                impl->descriptorSetLayout,
                impl->descriptorPool->handle(),
                m,
                impl->meshManager));

    impl->scene = scene;
    impl->recordCommands();
}

/* -------------------------------------------------------------------------- */

void ShadowMapRenderer::render()
{
    const glm::vec4& vp = impl->scene->viewport;
    const glm::mat4 lightMatrix = impl->scene->light.orthoShadowMatrix(impl->scene->camera, vp, 1.0f);

    for (std::shared_ptr<ShadowMapModel> m : impl->models)
    {
        Matrices matrices;
        matrices.model = m->model->worldTransform;
        matrices.light = lightMatrix;

        m->matricesUniformBuffer->copyHostVisible(&matrices, m->matricesUniformBuffer->size());
    }

    //--------------------------------------------------------------------------
    // Render

    Queue graphicsQueue(impl->device, impl->graphicsQueueFamilyIndex, 0);
    graphicsQueue.create();
    graphicsQueue.submit(impl->cmdBuf,
                         VK_NULL_HANDLE,
                         VK_NULL_HANDLE,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    graphicsQueue.waitIdle();
}

/* -------------------------------------------------------------------------- */

std::shared_ptr<Texture2D> ShadowMapRenderer::texture() const
{ return impl->texture; }

} // namespace vk
} // namespace kuu
