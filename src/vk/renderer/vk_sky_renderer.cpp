/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::SkyRenderer class
 * -------------------------------------------------------------------------- */


#include "vk_sky_renderer.h"

/* -------------------------------------------------------------------------- */

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/* -------------------------------------------------------------------------- */

#include "../../common/camera.h"
#include "../../common/light.h"
#include "../../common/material.h"
#include "../../common/mesh.h"
#include "../../common/model.h"
#include "../../common/scene.h"

/* -------------------------------------------------------------------------- */

#include "../vk_buffer.h"
#include "../vk_command.h"
#include "../vk_descriptor_set.h"
#include "../vk_mesh.h"
#include "../vk_pipeline.h"
#include "../vk_queue.h"
#include "../vk_shader_module.h"
#include "../vk_stringify.h"
#include "../vk_texture.h"

namespace kuu
{
namespace vk
{
namespace
{

/* -------------------------------------------------------------------------- *
   Matrices.
 * -------------------------------------------------------------------------- */
struct Matrices
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct SkyRenderer::Impl
{
    Impl(const VkPhysicalDevice& physicalDevice,
         const VkDevice& device,
         const VkExtent2D& extent,
         const uint32_t queueFamilyIndex,
         const VkRenderPass& renderPass,
         std::shared_ptr<TextureCube> environment)
        : physicalDevice(physicalDevice)
        , device(device)
        , extent(extent)
        , renderPass(renderPass)
        , environment(environment)
    {
        createDescriptorPool();
        createDescriptorSetLayout();
        createDescriptorSets();
        createMesh();
        createShaders();
        createUniformBuffers();
        createPipeline();
    }

    ~Impl()
    {
        vshModule.reset();
        fshModule.reset();
        pipeline.reset();

        mesh.reset();
        descriptorSets.reset();
        matricesUniformBuffer.reset();
        descriptorPool.reset();

        vkDestroyDescriptorSetLayout(
            device,
            descriptorSetLayout,
            NULL);
    }

    void createMesh()
    {
        float width  = 2.0f;
        float height = 2.0f;
        float depth  = 2.0f;
        float w = width  / 2.0f;
        float h = height / 2.0f;
        float d = depth  / 2.0f;

        // Create the vertex list
        std::vector<Vertex> vertices =
        {
            // -------------------------------------------------------
            // Back
            { { -w, -h, -d },  { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
            { {  w,  h, -d },  { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
            { {  w, -h, -d },  { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
            { {  w,  h, -d },  { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
            { { -w, -h, -d },  { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
            { { -w,  h, -d },  { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },

            // -------------------------------------------------------
            // Front
            { { -w, -h,  d },  { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
            { {  w, -h,  d },  { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
            { {  w,  h,  d },  { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
            { {  w,  h,  d },  { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
            { { -w,  h,  d },  { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
            { { -w, -h,  d },  { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

            // -------------------------------------------------------
            // Left
            { { -w,  h,  d },  { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
            { { -w,  h, -d },  { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
            { { -w, -h, -d },  { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
            { { -w, -h, -d },  { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
            { { -w, -h,  d },  { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
            { { -w,  h,  d },  { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },

            // -------------------------------------------------------
            // Right
            { {  w,  h,  d },  { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { {  w, -h, -d },  { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
            { {  w,  h, -d },  { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
            { {  w, -h, -d },  { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
            { {  w,  h,  d },  { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { {  w, -h,  d },  { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },

            // -------------------------------------------------------
            // Bottom
            { { -w, -h, -d },  { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
            { {  w, -h, -d },  { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
            { {  w, -h,  d },  { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
            { {  w, -h,  d },  { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
            { { -w, -h,  d },  { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
            { { -w, -h, -d },  { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },

            // -------------------------------------------------------
            // Top
            { { -w,  h, -d },  { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  w,  h,  d },  { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  w,  h, -d },  { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  w,  h,  d },  { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { { -w,  h, -d },  { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
            { { -w,  h,  d },  { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        };

        kuu::Mesh m;
        for (const Vertex& v : vertices)
            m.addVertex(v);

        std::vector<float> vertexVector;
        for (const Vertex& v : m.vertices)
        {
            vertexVector.push_back(v.pos.x);
            vertexVector.push_back(v.pos.y);
            vertexVector.push_back(v.pos.z);
            vertexVector.push_back(v.texCoord.x);
            vertexVector.push_back(v.texCoord.y);
            vertexVector.push_back(v.normal.x);
            vertexVector.push_back(v.normal.y);
            vertexVector.push_back(v.normal.z);
            vertexVector.push_back(v.tangent.x);
            vertexVector.push_back(v.tangent.y);
            vertexVector.push_back(v.tangent.z);
            vertexVector.push_back(v.bitangent.x);
            vertexVector.push_back(v.bitangent.y);
            vertexVector.push_back(v.bitangent.z);
        }

        mesh = std::make_shared<Mesh>(physicalDevice, device);
        mesh->setVertices(vertexVector);
        mesh->setIndices(m.indices);
        mesh->addVertexAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
        mesh->addVertexAttributeDescription(1, 0, VK_FORMAT_R32G32_SFLOAT,    3 * sizeof(float));
        mesh->addVertexAttributeDescription(2, 0, VK_FORMAT_R32G32B32_SFLOAT, 5 * sizeof(float));
        mesh->addVertexAttributeDescription(3, 0, VK_FORMAT_R32G32B32_SFLOAT, 8 * sizeof(float));
        mesh->addVertexAttributeDescription(4, 0, VK_FORMAT_R32G32B32_SFLOAT, 11 * sizeof(float));
        mesh->setVertexBindingDescription(0, 14 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX);
        mesh->create();
    }

    void createDescriptorPool()
    {
        uint32_t uniformBufferCount = 1;
        uint32_t imageSamplerCount  = 1;
        descriptorPool = std::make_shared<DescriptorPool>(device);
        descriptorPool->addTypeSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          uniformBufferCount);
        descriptorPool->addTypeSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  imageSamplerCount);
        descriptorPool->setMaxCount(uniformBufferCount + imageSamplerCount);
        descriptorPool->create();
    }

    void createDescriptorSets()
    {
        descriptorSets = std::make_shared<DescriptorSets>(device, descriptorPool->handle());
        descriptorSets->setLayout(descriptorSetLayout);
        descriptorSets->create();
    }

    void createShaders()
    {
        vshModule = std::make_shared<ShaderModule>(device, "shaders/skybox.vert.spv");
        vshModule->setStageName("main");
        vshModule->setStage(VK_SHADER_STAGE_VERTEX_BIT);
        vshModule->create();

        fshModule = std::make_shared<ShaderModule>(device, "shaders/skybox.frag.spv");
        fshModule->setStageName("main");
        fshModule->setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
        fshModule->create();
    }

    void createUniformBuffers()
    {
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

        descriptorSets->writeImage(
            1,
            environment->sampler,
            environment->imageView,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void createDescriptorSetLayout()
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings =
        {
            { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_VERTEX_BIT,   NULL },
            { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, NULL }
        };

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
                &descriptorSetLayout);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": descriptor set layout creation failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;
            return;
        }
    }

    void createPipeline()
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        descriptorSetLayouts.push_back(descriptorSetLayout);

        VkPipelineColorBlendAttachmentState colorBlend = {};
        colorBlend.blendEnable    = VK_FALSE;
        colorBlend.colorWriteMask = 0xf;

        float blendConstants[4] = { 0, 0, 0, 0 };

        pipeline = std::make_shared<Pipeline>(device);
        pipeline->addShaderStage(vshModule->createInfo());
        pipeline->addShaderStage(fshModule->createInfo());
        pipeline->setVertexInputState(
            { mesh->vertexBindingDescription() },
              mesh->vertexAttributeDescriptions() );
        pipeline->setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
        pipeline->setViewportState(
            { { 0, 0, float(extent.width), float(extent.height), 0, 1 } },
            { { { 0, 0 }, extent }  } );
        pipeline->setRasterizerState(
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_NONE,
            VK_FRONT_FACE_COUNTER_CLOCKWISE);
        pipeline->setMultisampleState(VK_FALSE, VK_SAMPLE_COUNT_1_BIT);
        pipeline->setDepthStencilState(VK_TRUE, VK_FALSE);
        pipeline->setColorBlendingState(
                VK_FALSE,
                VK_LOGIC_OP_CLEAR,
                { colorBlend },
                blendConstants);

        const std::vector<VkPushConstantRange> pushConstantRanges;
        pipeline->setPipelineLayout(descriptorSetLayouts, pushConstantRanges);
        pipeline->setDynamicState( { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } );
        pipeline->setRenderPass(renderPass);
        pipeline->create();
    }

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkExtent2D extent;
    VkRenderPass renderPass = VK_NULL_HANDLE;

    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Buffer> matricesUniformBuffer;
    std::shared_ptr<DescriptorSets> descriptorSets;

    std::shared_ptr<ShaderModule> vshModule;
    std::shared_ptr<ShaderModule> fshModule;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    std::shared_ptr<DescriptorPool> descriptorPool;

    std::shared_ptr<Pipeline> pipeline;

    std::shared_ptr<TextureCube> environment;
    std::shared_ptr<Scene> scene;
};

/* -------------------------------------------------------------------------- */

SkyRenderer::SkyRenderer(const VkPhysicalDevice& physicalDevice,
                         const VkDevice& device,
                         const uint32_t queueFamilyIndex,
                         const VkExtent2D& extent,
                         const VkRenderPass& renderPass,
                         std::shared_ptr<TextureCube> environment)
    : impl(std::make_shared<Impl>(physicalDevice,
                                  device,
                                  extent,
                                  queueFamilyIndex,
                                  renderPass,
                                  environment))
{}

/* -------------------------------------------------------------------------- */

void SkyRenderer::setScene(std::shared_ptr<Scene> scene)
{
    impl->scene = scene;
}

/* -------------------------------------------------------------------------- */

std::shared_ptr<Scene> SkyRenderer::scene() const
{ return impl->scene; }

/* -------------------------------------------------------------------------- */

void SkyRenderer::resized(const VkExtent2D& extent,
                          const VkRenderPass& renderPass)
{
    impl->extent     = extent;
    impl->renderPass = renderPass;
    impl->createPipeline();
}

/* -------------------------------------------------------------------------- */

void SkyRenderer::recordCommands(const VkCommandBuffer& commandBuffer)
{
    VkDescriptorSet descriptorHandle = impl->descriptorSets->handle();
    VkPipelineLayout pipelineLayout  = impl->pipeline->pipelineLayoutHandle();
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1,
        &descriptorHandle, 0, NULL);

    VkPipeline pipeline = impl->pipeline->handle();
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline);

    const VkBuffer vertexBuffer = impl->mesh->vertexBufferHandle();
    const VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers(
        commandBuffer, 0, 1,
        &vertexBuffer,
        offsets);

    const VkBuffer indexBuffer = impl->mesh->indexBufferHandle();
    vkCmdBindIndexBuffer(
        commandBuffer,
        indexBuffer,
        0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(
        commandBuffer,
        impl->mesh->indexCount(),
        1, 0, 0, 1);
}

/* -------------------------------------------------------------------------- */

void SkyRenderer::updateUniformBuffers()
{
    Matrices matrices;
    matrices.view       = glm::mat4(glm::mat3(impl->scene->camera.viewMatrix()));
    matrices.projection = impl->scene->camera.projectionMatrix();

    impl->matricesUniformBuffer->copyHostVisible(
        &matrices,
        impl->matricesUniformBuffer->size());
}

} // namespace vk
} // namespace kuu
