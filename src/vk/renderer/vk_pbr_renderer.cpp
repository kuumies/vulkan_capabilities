/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::PbrRenderer class
 * -------------------------------------------------------------------------- */


#include "vk_pbr_renderer.h"

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
#include "vk_irradiance_renderer.h"
#include "vk_ibl_brdf_lut_renderer.h"
#include "vk_ibl_prefilter_renderer.h"

namespace kuu
{
namespace vk
{
namespace
{

/* -------------------------------------------------------------------------- *
   Uniform structs
 * -------------------------------------------------------------------------- */
struct Matrices
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 normal;
};

struct PbrParams
{
    glm::vec4  albedo;
    float metallic;
    float roughness;
    float ao;
};

/* -------------------------------------------------------------------------- *
   Textures.
 * -------------------------------------------------------------------------- */
struct TextureManager
{
    TextureManager(const VkPhysicalDevice& physicalDevice,
                   const VkDevice& device,
                   const uint32_t& queueFamilyIndex,
                   CommandPool& commandPool)
        : physicalDevice(physicalDevice)
        , device(device)
        , queueFamilyIndex(queueFamilyIndex)
        , commandPool(commandPool)
    {
        VkExtent2D extent = { 1, 1 };

        textures2d["dummy_rgba"] =
            std::make_shared<Texture2D>(
                physicalDevice,
                device,
                extent,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_FILTER_LINEAR,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        textures2d["dummy_r"] =
            std::make_shared<Texture2D>(
                physicalDevice,
                device,
                extent,
                VK_FORMAT_R8_UNORM,
                VK_FILTER_LINEAR,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    }

    void add(const std::string& filepath)
    {
        if (textures2d.count(filepath))
            return;

        Queue queue(device, queueFamilyIndex, 0);
        queue.create();

        textures2d[filepath] =
            std::make_shared<Texture2D>(
                physicalDevice,
                device,
                queue,
                commandPool,
                filepath,
                VK_FILTER_LINEAR,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                true);
    }

    const VkPhysicalDevice physicalDevice;
    const VkDevice device;
    const uint32_t queueFamilyIndex;
    CommandPool& commandPool;

    std::map<std::string, std::shared_ptr<Texture2D>> textures2d;

    std::shared_ptr<TextureCube> environment;
    std::shared_ptr<TextureCube> irradiance;
    std::shared_ptr<TextureCube> prefiltered;
    std::shared_ptr<Texture2D> brdfLut;
};

/* -------------------------------------------------------------------------- *
   A model for physically-based rendering.
 * -------------------------------------------------------------------------- */
struct PbrModel
{
    PbrModel(const VkPhysicalDevice& physicalDevice,
             const VkDevice& device,
             const VkDescriptorSetLayout& descriptorSetLayout,
             const VkDescriptorPool& descriptorPool,
             const Model& model,
             std::shared_ptr<TextureManager> textureManager)
        : model(model)
    {
        // ---------------------------------------------------------------------
        // Params

        pbrParams.albedo    = glm::vec4(model.material.pbr.albedo, 1.0);
        pbrParams.ao        = model.material.pbr.ao;
        pbrParams.metallic  = model.material.pbr.metallic;
        pbrParams.roughness = model.material.pbr.roughness;


        // ---------------------------------------------------------------------
        // Mesh

        std::vector<float> vertexVector;
        for (const Vertex& v : model.mesh.vertices)
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

        mesh = std::make_shared<vk::Mesh>(physicalDevice, device);
        mesh->setVertices(vertexVector);
        mesh->setIndices(model.mesh.indices);
        mesh->addVertexAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
        mesh->addVertexAttributeDescription(1, 0, VK_FORMAT_R32G32_SFLOAT,    3 * sizeof(float));
        mesh->addVertexAttributeDescription(2, 0, VK_FORMAT_R32G32B32_SFLOAT, 5 * sizeof(float));
        mesh->addVertexAttributeDescription(3, 0, VK_FORMAT_R32G32B32_SFLOAT, 8 * sizeof(float));
        mesh->addVertexAttributeDescription(4, 0, VK_FORMAT_R32G32B32_SFLOAT, 11 * sizeof(float));
        mesh->setVertexBindingDescription(0, 14 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX);
        mesh->create();

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

        lightUniformBuffer = std::make_shared<Buffer>(physicalDevice, device);
        lightUniformBuffer->setSize(sizeof(Light));
        lightUniformBuffer->setUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        lightUniformBuffer->setMemoryProperties(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        lightUniformBuffer->create();

        paramsUniformBuffer = std::make_shared<Buffer>(physicalDevice, device);
        paramsUniformBuffer->setSize(sizeof(PbrParams));
        paramsUniformBuffer->setUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        paramsUniformBuffer->setMemoryProperties(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        paramsUniformBuffer->create();
        paramsUniformBuffer->copyHostVisible(&pbrParams, paramsUniformBuffer->size());

        descriptorSets->writeUniformBuffer(
            0, matricesUniformBuffer->handle(),
            0, matricesUniformBuffer->size());

        descriptorSets->writeUniformBuffer(
            1, lightUniformBuffer->handle(),
            0, lightUniformBuffer->size());

        descriptorSets->writeUniformBuffer(
            2, paramsUniformBuffer->handle(),
            0, paramsUniformBuffer->size());

        auto writeTexture = [&](uint32_t binding, std::string filePath, bool grayScale)
        {
            std::shared_ptr<Texture2D> tex;
            if (filePath.size())
            {
                textureManager->add(filePath);
                tex = textureManager->textures2d.at(filePath);

            }
            else
            {
                if (grayScale)
                    tex = textureManager->textures2d["dummy_r"];
                else
                    tex = textureManager->textures2d["dummy_rgba"];
            }

            descriptorSets->writeImage(
                    binding,
                    tex->sampler,
                    tex->imageView,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        };

        // ---------------------------------------------------------------------
        // Texture maps.

        writeTexture(3, model.material.pbr.ambientOcclusionMap, true);
        writeTexture(4, model.material.pbr.baseColorMap,        false);
        writeTexture(5, model.material.pbr.heightMap,           true);
        writeTexture(6, model.material.pbr.metallicMap,         true);
        writeTexture(7, model.material.pbr.normalMap,           false);
        writeTexture(8, model.material.pbr.roughnessMap,        true);

        descriptorSets->writeImage(
                9,
                textureManager->irradiance->sampler,
                textureManager->irradiance->imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        descriptorSets->writeImage(
                10,
                textureManager->prefiltered->sampler,
                textureManager->prefiltered->imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        descriptorSets->writeImage(
                11,
                textureManager->brdfLut->sampler,
                textureManager->brdfLut->imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // Model
    Model model;

    // Mesh
    std::shared_ptr<Mesh> mesh;

    // Uniform buffers
    std::shared_ptr<Buffer> matricesUniformBuffer;
    std::shared_ptr<Buffer> lightUniformBuffer;
    std::shared_ptr<Buffer> paramsUniformBuffer;

    // Descriptor sets
    std::shared_ptr<DescriptorSets> descriptorSets;

    // Material parameters, used when map is not set.
    PbrParams pbrParams;

    // Material maps.
    std::shared_ptr<Texture2D> albedoMap;
    std::shared_ptr<Texture2D> metallicMap;
    std::shared_ptr<Texture2D> roughnessMap;
    std::shared_ptr<Texture2D> aoMap;

    // Extra maps for parallax and normal mapping.
    std::shared_ptr<Texture2D> height;
    std::shared_ptr<Texture2D> normal;
};

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct PbrRenderer::Impl
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
    {
        createCommandPool(device, queueFamilyIndex);
        createTextureManager(queueFamilyIndex);
        createIblMaps(queueFamilyIndex, environment);
        createShaders();
        createDescriptorSetLayout();
        createPipeline();
    }

    ~Impl()
    {
        vshModule.reset();
        fshModule.reset();
        pipeline.reset();

        models.clear();

        commandPool.reset();
        descriptorPool.reset();

        vkDestroyDescriptorSetLayout(
            device,
            descriptorSetLayout,
            NULL);
    }

    void createCommandPool(const VkDevice& device,
                           const uint32_t queueFamilyIndex)
    {
        commandPool = std::make_shared<CommandPool>(device);
        commandPool->setQueueFamilyIndex(queueFamilyIndex);
        commandPool->create();
    }

    void createTextureManager(const uint32_t queueFamilyIndex)
    {
        textureManager = std::make_shared<TextureManager>(
                physicalDevice,
                device,
                queueFamilyIndex,
                *commandPool);
    }

    void createIblMaps(const uint32_t queueFamilyIndex,
                       std::shared_ptr<TextureCube> environment)
    {
        IrradianceRenderer irradianceRenderer(
            physicalDevice,
            device,
            queueFamilyIndex,
            environment);
        irradianceRenderer.render();

        textureManager->irradiance  = irradianceRenderer.textureCube();

        IblPrefilterRenderer iblPrefilterRenderer(
            physicalDevice,
            device,
            queueFamilyIndex,
            environment);
        iblPrefilterRenderer.render();

        textureManager->prefiltered = iblPrefilterRenderer.textureCube();

        IblBrdfLutRenderer iblBrdfRenderer(
            physicalDevice,
            device,
            queueFamilyIndex);
        iblBrdfRenderer.render();

        textureManager->brdfLut  = iblBrdfRenderer.texture();
    }

    void createShaders()
    {
        vshModule = std::make_shared<ShaderModule>(device, "shaders/pbr.vert.spv");
        vshModule->setStageName("main");
        vshModule->setStage(VK_SHADER_STAGE_VERTEX_BIT);
        if (!vshModule->create())
            return;

        fshModule = std::make_shared<ShaderModule>(device, "shaders/pbr.frag.spv");
        fshModule->setStageName("main");
        fshModule->setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
        if (!fshModule->create())
            return;
    }

    void createDescriptorSetLayout()
    {
        if (descriptorSetLayout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(
                device,
                descriptorSetLayout,
                NULL);

        uint32_t binding = 0;

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings =
        {
            { binding++, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,   NULL },
            { binding++, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, NULL },
            { binding++, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, NULL },
        };

        const uint32_t images = 11;
        for (int i = 0; i < images; ++i)
            layoutBindings.push_back(
                { binding++, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                  1, VK_SHADER_STAGE_FRAGMENT_BIT, NULL } );


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

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        descriptorSetLayouts.push_back(descriptorSetLayout);

        VkPipelineColorBlendAttachmentState colorBlend = {};
        colorBlend.blendEnable    = VK_FALSE;
        colorBlend.colorWriteMask = 0xf;

        float blendConstants[4] = { 0, 0, 0, 0 };
        VkExtent2D extent2d = { extent.width, extent.height };

        if (pipeline)
        {
            pipeline->destroy();
            pipeline.reset();
        }

        pipeline = std::make_shared<Pipeline>(device);
        pipeline->addShaderStage(vshModule->createInfo());
        pipeline->addShaderStage(fshModule->createInfo());
        pipeline->setVertexInputState(
            { vertexBindingDescription },
              vertexAttributes );
        pipeline->setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
        pipeline->setViewportState(
            { { 0, 0, float(extent2d.width), float(extent2d.height), 0, 1 } },
            { { { 0, 0 }, extent2d }  } );
        pipeline->setRasterizerState(
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_NONE,
            VK_FRONT_FACE_COUNTER_CLOCKWISE);
        pipeline->setMultisampleState(VK_FALSE, VK_SAMPLE_COUNT_1_BIT);
        pipeline->setDepthStencilState(VK_TRUE);
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

    std::shared_ptr<ShaderModule> vshModule;
    std::shared_ptr<ShaderModule> fshModule;

    std::shared_ptr<CommandPool> commandPool;

    std::shared_ptr<TextureManager> textureManager;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    std::shared_ptr<DescriptorPool> descriptorPool;

    std::shared_ptr<Pipeline> pipeline;

    std::shared_ptr<Scene> scene;
    std::vector<std::shared_ptr<PbrModel>> models;
};

/* -------------------------------------------------------------------------- */

PbrRenderer::PbrRenderer(const VkPhysicalDevice& physicalDevice,
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

void PbrRenderer::resized(const VkExtent2D& extent,
                          const VkRenderPass& renderPass)
{
    impl->extent     = extent;
    impl->renderPass = renderPass;
    impl->createPipeline();
}

/* -------------------------------------------------------------------------- */

void PbrRenderer::setScene(std::shared_ptr<Scene> scene)
{
    std::vector<Model> pbrModels;
    for (const Model& m :scene->models)
        if (m.material.type == Material::Type::Pbr)
            pbrModels.push_back(m);

    uint32_t uniformBufferCount = 3 * uint32_t(pbrModels.size());
    uint32_t imageSamplerCount  = 11 * uint32_t(pbrModels.size());
    impl->descriptorPool = std::make_shared<DescriptorPool>(impl->device);
    impl->descriptorPool->addTypeSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          uniformBufferCount);
    impl->descriptorPool->addTypeSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  imageSamplerCount);
    impl->descriptorPool->setMaxCount(uniformBufferCount + imageSamplerCount);
    impl->descriptorPool->create();

    impl->scene = scene;
    for (const Model& m :pbrModels)
        impl->models.push_back(
            std::make_shared<PbrModel>(
                impl->physicalDevice,
                impl->device,
                impl->descriptorSetLayout,
                impl->descriptorPool->handle(),
                m,
                impl->textureManager));

   updateUniformBuffers();
}

/* -------------------------------------------------------------------------- */

std::shared_ptr<Scene> PbrRenderer::scene() const
{ return impl->scene; }

/* -------------------------------------------------------------------------- */

void PbrRenderer::recordCommands(const VkCommandBuffer& commandBuffer)
{
    for (std::shared_ptr<PbrModel> model : impl->models)
    {
        VkDescriptorSet descriptorHandle = model->descriptorSets->handle();
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

        const VkBuffer vertexBuffer = model->mesh->vertexBufferHandle();
        const VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(
            commandBuffer, 0, 1,
            &vertexBuffer,
            offsets);

        const VkBuffer indexBuffer = model->mesh->indexBufferHandle();
        vkCmdBindIndexBuffer(
            commandBuffer,
            indexBuffer,
            0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(
            commandBuffer,
            model->mesh->indexCount(),
            1, 0, 0, 1);
        }
}

/* -------------------------------------------------------------------------- */

void PbrRenderer::updateUniformBuffers()
{
    Light eyeLight = impl->scene->light;
    eyeLight.dir   = impl->scene->camera.viewMatrix() * eyeLight.dir;

    for (std::shared_ptr<PbrModel> m : impl->models)
    {
        Matrices matrices;
        matrices.view       = impl->scene->camera.viewMatrix();
        matrices.projection = impl->scene->camera.projectionMatrix();
        matrices.model      = m->model.worldTransform;
        matrices.normal     = glm::inverseTranspose(matrices.view * matrices.model);

        m->matricesUniformBuffer->copyHostVisible(&matrices, m->matricesUniformBuffer->size());
        m->lightUniformBuffer->copyHostVisible(&eyeLight, m->lightUniformBuffer->size());
    }
}

} // namespace vk
} // namespace kuu
