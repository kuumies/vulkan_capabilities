/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Renderer class.
 * -------------------------------------------------------------------------- */

#include "vk_renderer.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <QtGui/QImage>
#include "vk_buffer.h"
#include "vk_command.h"
#include "vk_descriptor_set.h"
#include "vk_helper.h"
#include "vk_image.h"
#include "vk_logical_device.h"
#include "vk_mesh.h"
#include "vk_pipeline.h"
#include "vk_queue.h"
#include "vk_render_pass.h"
#include "vk_shader_module.h"
#include "vk_stringify.h"
#include "vk_surface_properties.h"
#include "vk_sync.h"
#include "vk_swapchain.h"
#include "vk_texture.h"
#include "../common/scene.h"
#include "../common/light.h"

namespace kuu
{
namespace vk
{

namespace
{

std::vector<VkQueueFamilyProperties> getQueueFamilies(
    const VkPhysicalDevice& physicalDevice)
{
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,            // [in]  physical device handle
        &queueFamilyPropertyCount, // [out] queue family property count
        NULL);                     // [in]  properties, NULL to get count

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,                // [in]  physical device handle
        &queueFamilyPropertyCount,     // [in]  queue family property count
        queueFamilyProperties.data()); // [out] queue family properties

    return queueFamilyProperties;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct Matrices
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 normal;
};

/* -------------------------------------------------------------------------- */

struct RendererModel
{
    bool create(
        const VkPhysicalDevice& physicalDevice,
        const VkDevice& device,
        const VkDescriptorPool& descriptorPool,
        const uint32_t& queueFamilyIndex,
        CommandPool& commandPool,
        const Model& m)
    {
        model = m;

        if (!createMesh(physicalDevice, device, m.mesh))
            return false;

        if (!createUniformBuffers(physicalDevice, device))
            return false;

        switch (m.material.type)
        {
            case Material::Type::Diffuse:
                textures.push_back(createTexture(physicalDevice, device, queueFamilyIndex, commandPool, m.material.diffuse.map));
                break;

            case Material::Type::Pbr:
                textures.push_back(createTexture(physicalDevice, device, queueFamilyIndex, commandPool, m.material.pbr.ambientOcclusion));
                textures.push_back(createTexture(physicalDevice, device, queueFamilyIndex, commandPool, m.material.pbr.baseColor));
                textures.push_back(createTexture(physicalDevice, device, queueFamilyIndex, commandPool, m.material.pbr.height   ));
                textures.push_back(createTexture(physicalDevice, device, queueFamilyIndex, commandPool, m.material.pbr.metallic));
                textures.push_back(createTexture(physicalDevice, device, queueFamilyIndex, commandPool, m.material.pbr.normal));
                textures.push_back(createTexture(physicalDevice, device, queueFamilyIndex, commandPool, m.material.pbr.roughness));
                break;
        }

        if (!createDescriptorSets(device, descriptorPool))
            return false;

        return true;
    }

    bool createMesh(const VkPhysicalDevice& physicalDevice,
                    const VkDevice& device,
                    const kuu::Mesh& m)
    {
        indexCount = uint32_t(m.indices.size());

        mesh = std::make_shared<vk::Mesh>(physicalDevice, device);
        mesh->setVertices(m.vertices);
        mesh->setIndices(m.indices);
        mesh->addVertexAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
        mesh->addVertexAttributeDescription(1, 0, VK_FORMAT_R32G32_SFLOAT,    3 * sizeof(float));
        mesh->addVertexAttributeDescription(2, 0, VK_FORMAT_R32G32B32_SFLOAT, 5 * sizeof(float));
        mesh->setVertexBindingDescription(0, 8 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX);
        return mesh->create();
    }

    bool createUniformBuffers(const VkPhysicalDevice& physicalDevice,
                              const VkDevice& device)
    {
        matricesUniformBuffer = std::make_shared<Buffer>(physicalDevice, device);
        matricesUniformBuffer->setSize(sizeof(Matrices));
        matricesUniformBuffer->setUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        matricesUniformBuffer->setMemoryProperties(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (!matricesUniformBuffer->create())
            return false;

        lightUniformBuffer = std::make_shared<Buffer>(physicalDevice, device);
        lightUniformBuffer->setSize(sizeof(Light));
        lightUniformBuffer->setUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        lightUniformBuffer->setMemoryProperties(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (!lightUniformBuffer->create())
            return false;

        return true;
    }

    bool createDescriptorSets(const VkDevice& device,
                              const VkDescriptorPool& descriptorPool)
    {
        descriptorSets = std::make_shared<DescriptorSets>(device, descriptorPool);
        descriptorSets->addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);

        std::vector<std::shared_ptr<Buffer>> uniformBuffers;
        switch (model.material.type)
        {
            case Material::Type::Diffuse:
                descriptorSets->addLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
                uniformBuffers.push_back(matricesUniformBuffer);
                break;

            case Material::Type::Pbr:
                descriptorSets->addLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
                for (int i = 2; i < 8; ++i)
                    descriptorSets->addLayoutBinding(i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
                uniformBuffers.push_back(matricesUniformBuffer);
                uniformBuffers.push_back(lightUniformBuffer);
                break;
        }

        if (!descriptorSets->create())
            return false;

        int binding = 0;
        for (int i = 0; i < uniformBuffers.size(); ++i)
            descriptorSets->writeUniformBuffer(
                binding++,
                uniformBuffers[i]->handle(),
                0,
                uniformBuffers[i]->size());

        for (const auto tex : textures)
            descriptorSets->writeImage(
                binding++,
                tex->sampler,
                tex->imageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        return true;
    }

    std::shared_ptr<Texture2D> createTexture(
            const VkPhysicalDevice& physicalDevice,
            const VkDevice& device,
            const uint32_t& queueFamilyIndex,
            CommandPool& commandPool,
            const std::string& filePath)
    {
        Queue queue(device, queueFamilyIndex, 0);
        queue.create();

        return std::make_shared<Texture2D>(
            physicalDevice,
            device,
            queue,
            commandPool,
            filePath,
            VK_FILTER_LINEAR,
            VK_FILTER_LINEAR,
            VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VK_SAMPLER_ADDRESS_MODE_REPEAT,
            true);
    }

    void destroy()
    {
        mesh->destroy();
        matricesUniformBuffer->destroy();
        lightUniformBuffer->destroy();
        descriptorSets->destroy();
        textures.clear();
    }

    // Mesh
    std::shared_ptr<Mesh> mesh;
    uint32_t indexCount;

    // Uniform buffers
    std::shared_ptr<Buffer> matricesUniformBuffer;
    std::shared_ptr<Buffer> lightUniformBuffer;

    // Descriptor sets
    std::shared_ptr<DescriptorSets> descriptorSets;

    // Texture
    std::vector<std::shared_ptr<Texture2D>> textures;

    // Model
    Model model;
};

struct RendererPipeline
{
    RendererPipeline()
    {}

    bool create(
        const VkDevice& device,
        const VkExtent2D& extent,
        const VkRenderPass& renderPass,
        const VkVertexInputBindingDescription& vertexBinding,
        const std::vector<VkVertexInputAttributeDescription>&  vertexAttributes,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const std::string& vshFilePath,
        const std::string& fshFilePath)
    {
        if (!createShaders(device, vshFilePath, fshFilePath))
            return false;

        return createPipeline(device, extent, renderPass, vertexBinding,
                              vertexAttributes, descriptorSetLayouts);
    }

    bool createShaders(
        const VkDevice& device,
        const std::string& vshFilePath,
        const std::string& fshFilePath)
    {
        vshModule = std::make_shared<ShaderModule>(device, vshFilePath);
        vshModule->setStageName("main");
        vshModule->setStage(VK_SHADER_STAGE_VERTEX_BIT);
        if (!vshModule->create())
            return false;

        fshModule = std::make_shared<ShaderModule>(device, fshFilePath);
        fshModule->setStageName("main");
        fshModule->setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
        if (!fshModule->create())
            return false;

        return true;
    }

    bool createPipeline(
        const VkDevice& device,
        const VkExtent2D& extent,
        const VkRenderPass& renderPass,
        const VkVertexInputBindingDescription& vertexBinding,
        const std::vector<VkVertexInputAttributeDescription>&  vertexAttributes,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPipelineColorBlendAttachmentState colorBlend = {};
        colorBlend.blendEnable    = VK_FALSE;
        colorBlend.colorWriteMask = 0xf;

        float blendConstants[4] = { 0, 0, 0, 0 };

        pipeline = std::make_shared<Pipeline>(device);
        pipeline->addShaderStage(vshModule->createInfo());
        pipeline->addShaderStage(fshModule->createInfo());
        pipeline->setVertexInputState(
            { vertexBinding },
              vertexAttributes );
        pipeline->setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
        pipeline->setViewportState(
            { { 0, 0, float(extent.width), float(extent.height), 0, 1 } },
            { { { 0, 0 }, extent }  } );
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
        return pipeline->create();
    }

    void destroy()
    {
        pipeline->destroy();
        fshModule->destroy();
        vshModule->destroy();
    }

    bool isValid() const
    {
        return pipeline->isValid()  &&
               vshModule->isValid() &&
               fshModule->isValid();
    }

    // Shaders
    std::shared_ptr<ShaderModule> vshModule;
    std::shared_ptr<ShaderModule> fshModule;

    // Pipeline
    std::shared_ptr<Pipeline> pipeline;
};

/* -------------------------------------------------------------------------- */

struct Renderer::Impl
{
    Impl(const VkInstance& instance,
         const VkPhysicalDevice& physicalDevice,
         const VkSurfaceKHR& surface,
         const VkExtent2D& extent,
         const std::shared_ptr<Scene>& scene)
        : instance(instance)
        , physicalDevice(physicalDevice)
        , surface(surface)
        , surfaceInfo(instance, physicalDevice, surface)
        , extent(extent)
        , scene(scene)
    {}

    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {
        if (!setupSurface())
            return false;

        if (!createLogicalDevice())
            return false;

        if (!createSwapchain())
            return false;

        if (!createCommandPool())
            return false;

        if (!createDescriptorPool())
            return false;

        if (!createModels())
            return false;

        if (!createRenderPass())
            return false;

        if (!createPipelines())
            return false;

        if (!createCommandBuffers())
            return false;

        if (!createSync())
            return false;

        return true;
    }

    bool setupSurface()
    {
        surfaceFormat = vk::helper::findSwapchainSurfaceFormat(surfaceInfo.surfaceFormats);
        if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
            return false;

        presentMode = vk::helper::findSwapchainPresentMode(surfaceInfo.presentModes);
        extent      = vk::helper::findSwapchainImageExtent(surfaceInfo.surfaceCapabilities, extent);
        return true;
    }

    bool createLogicalDevice()
    {
        // Get the queue family indices for graphics and presentation queues
        auto queueFamilies = getQueueFamilies(physicalDevice);
        const int graphics     = helper::findQueueFamilyIndex(
            VK_QUEUE_GRAPHICS_BIT,
            queueFamilies);

        if (graphics == -1)
            return false;

        const int presentation = helper::findPresentationQueueFamilyIndex(
            physicalDevice,
            surface,
            queueFamilies,
            { graphics });

        if (presentation == -1)
            return false;

        graphicsFamilyIndex     = graphics;
        presentationFamilyIndex = presentation;

        device = std::make_shared<LogicalDevice>(physicalDevice);
        device->setExtensions( { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
        device->addQueueFamily(graphicsFamilyIndex,     1, 1.0f);
        device->addQueueFamily(presentationFamilyIndex, 1, 1.0f);
        return device->create();
    }

    bool createRenderPass()
    {
        // Colorbuffer attachment description
        VkAttachmentDescription colorAttachment;
        colorAttachment.flags          = 0;
        colorAttachment.format         = surfaceFormat.format;
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Depth/stencil buffer attachment description
        VkAttachmentDescription depthStencilAttachment;
        depthStencilAttachment.flags          = 0;
        depthStencilAttachment.format         = VK_FORMAT_D32_SFLOAT_S8_UINT;
        depthStencilAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthStencilAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthStencilAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Reference to first attachment (color)
        VkAttachmentReference colorAttachmentRef;
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Reference to second attachment (depth & stencil)
        VkAttachmentReference depthStencilAttachmentRef;
        depthStencilAttachmentRef.attachment = 1;
        depthStencilAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Subpass
        VkSubpassDescription subpass;
        subpass.flags                   = 0;
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount    = 0;
        subpass.pInputAttachments       = NULL;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pResolveAttachments     = NULL;
        subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;
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

        renderPass = std::make_shared<RenderPass>(physicalDevice, device->handle());
        renderPass->addAttachmentDescription(colorAttachment);
        renderPass->addAttachmentDescription(depthStencilAttachment);
        renderPass->addSubpassDescription(subpass);
        renderPass->addSubpassDependency(dependency);
        renderPass->setSwapchainImageViews(swapchain->imageViews(), extent);
        return renderPass->create();

        // Attachment usage dependency. Only this subpass is using the attachment...
    //    srcSubpass – Index of a first (previous) subpass or VK_SUBPASS_EXTERNAL if we want to indicate dependency between subpass and operations outside of a render pass.
    //    dstSubpass – Index of a second (later) subpass (or VK_SUBPASS_EXTERNAL).
    //    srcStageMask – Pipeline stage during which a given attachment was used before (in a src subpass).
    //    dstStageMask – Pipeline stage during which a given attachment will be used later (in a dst subpass).
    //    srcAccessMask – Types of memory operations that occurred in a src subpass or before a render pass.
    //    dstAccessMask – Types of memory operations that occurred in a dst subpass or after a render pass.
    //    dependencyFlags – Flag describing the type (region) of dependency.

    }

    bool createSwapchain()
    {
        swapchainImageCount = vk::helper::findSwapchainImageCount(surfaceInfo.surfaceCapabilities);

        swapchain = std::make_shared<vk::Swapchain>(device->handle(), surface);
        swapchain->setSurfaceFormat(surfaceFormat);
        swapchain->setPresentMode(presentMode);
        swapchain->setImageExtent(extent);
        swapchain->setImageCount(swapchainImageCount);
        swapchain->setPreTransform(surfaceInfo.surfaceCapabilities.currentTransform);
        swapchain->setQueueIndicies( { graphicsFamilyIndex, presentationFamilyIndex } );
        return swapchain->create();
    }

    bool createDescriptorPool()
    {
        uint32_t uniformBufferCount = 0;
        uint32_t imageSamplerCount  = 0;
        for (const Model& model : scene->models)
        {
            switch(model.material.type)
            {
                case Material::Type::Diffuse:
                    uniformBufferCount += 1;
                    imageSamplerCount  += 1;
                    break;

                case Material::Type::Pbr:
                    uniformBufferCount += 2;
                    imageSamplerCount  += 6;
                    break;
            }
        }

        descriptorPool = std::make_shared<DescriptorPool>(device->handle());
        descriptorPool->addTypeSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          uniformBufferCount);
        descriptorPool->addTypeSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  imageSamplerCount);
        descriptorPool->setMaxCount(uniformBufferCount + imageSamplerCount);
        return descriptorPool->create();
    }

    bool createModels()
    {
        for (const Model& model : scene->models)
        {
            RendererModel m;
            if (!m.create(physicalDevice,
                          device->handle(),
                          descriptorPool->handle(),
                          graphicsFamilyIndex,
                          *commandPool,
                          model))
            {
                return false;
            }

            vulkanModels.push_back(m);
        }
        return true;
    }

    bool createPipelines()
    {
        if (vulkanModels.size() == 0)
            return false;

        std::vector<VkDescriptorSetLayout> diffuseDescriptorSetLayouts;
        for (const RendererModel& m : vulkanModels)
            if (m.model.material.type == Material::Type::Diffuse)
                diffuseDescriptorSetLayouts.push_back(m.descriptorSets->layoutHandle());

        diffusePipeline = std::make_shared<RendererPipeline>();
        if (!diffusePipeline->create(
                device->handle(),
                extent,
                renderPass->handle(),
                vulkanModels[0].mesh->vertexBindingDescription(),
                vulkanModels[0].mesh->vertexAttributeDescriptions(),
                diffuseDescriptorSetLayouts,
                "shaders/diffuse.vert.spv",
                "shaders/diffuse.frag.spv"))
        {
            return false;
        }

        std::vector<VkDescriptorSetLayout> pbrDescriptorSetLayouts;
        for (const RendererModel& m : vulkanModels)
            if (m.model.material.type == Material::Type::Pbr)
                pbrDescriptorSetLayouts.push_back(m.descriptorSets->layoutHandle());

        pbrPipeline = std::make_shared<RendererPipeline>();
        return pbrPipeline->create(
               device->handle(),
               extent,
               renderPass->handle(),
               vulkanModels[0].mesh->vertexBindingDescription(),
               vulkanModels[0].mesh->vertexAttributeDescriptions(),
               pbrDescriptorSetLayouts,
               "shaders/pbr.vert.spv",
               "shaders/pbr.frag.spv");
    }

    bool createCommandPool()
    {
        commandPool = std::make_shared<CommandPool>(device->handle());
        commandPool->setQueueFamilyIndex(graphicsFamilyIndex);
        return commandPool->create();
    }

    bool createCommandBuffers()
    {
        commandBuffers =
            commandPool->allocateBuffers(
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                swapchainImageCount);

        for (size_t i = 0; i < commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo;
            beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext            = NULL;
            beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            beginInfo.pInheritanceInfo = NULL;

            vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

            std::vector<VkClearValue> clearValues(2);
            clearValues[0].color        = { 0.1f, 0.1f, 0.1f, 1.0f };
            clearValues[1].depthStencil = { 1.0f, 0 };

            VkRenderPassBeginInfo renderPassInfo;
            renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.pNext             = NULL;
            renderPassInfo.renderPass        = renderPass->handle();
            renderPassInfo.framebuffer       = renderPass->framebuffer(uint32_t(i));
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = extent;
            renderPassInfo.clearValueCount   = uint32_t(clearValues.size());
            renderPassInfo.pClearValues      = clearValues.data();

            vkCmdBeginRenderPass(
                commandBuffers[i],
                &renderPassInfo,
                VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport;
            viewport.x        = 0;
            viewport.y        = 0;
            viewport.width    = float(extent.width);
            viewport.height   = float(extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

            VkRect2D scissor;
            scissor.offset = {};
            scissor.extent = extent;
            vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

            for (const RendererModel& m : vulkanModels)
            {
                VkDescriptorSet descriptorHandle = m.descriptorSets->handle();

                VkPipeline pipeline = VK_NULL_HANDLE;
                VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
                switch(m.model.material.type)
                {
                    case Material::Type::Diffuse:
                        pipeline       = diffusePipeline->pipeline->handle();
                        pipelineLayout = diffusePipeline->pipeline->pipelineLayoutHandle();
                        break;

                    case Material::Type::Pbr:
                        pipeline       = pbrPipeline->pipeline->handle();
                        pipelineLayout = pbrPipeline->pipeline->pipelineLayoutHandle();
                        break;
                }

                vkCmdBindDescriptorSets(
                    commandBuffers[i],
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 0, 1,
                    &descriptorHandle, 0, NULL);

                vkCmdBindPipeline(
                    commandBuffers[i],
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline);

                const VkBuffer vertexBuffer = m.mesh->vertexBufferHandle();
                const VkDeviceSize offsets[1] = { 0 };
                vkCmdBindVertexBuffers(
                    commandBuffers[i], 0, 1,
                    &vertexBuffer,
                    offsets);

                const VkBuffer indexBuffer = m.mesh->indexBufferHandle();
                vkCmdBindIndexBuffer(
                    commandBuffers[i],
                    indexBuffer,
                    0, VK_INDEX_TYPE_UINT32);

                vkCmdDrawIndexed(
                    commandBuffers[i],
                    m.indexCount,
                    1, 0, 0, 1);
            }

            vkCmdEndRenderPass(commandBuffers[i]);
            const VkResult result = vkEndCommandBuffer(commandBuffers[i]);
            if (result != VK_SUCCESS)
            {
                std::cerr << __FUNCTION__
                          << ": render commands failed as "
                          << vk::stringify::resultDesc(result)
                          << std::endl;
                return false;
            }
        }

        return true;
    }

    bool createSync()
    {
        renderingFinished = std::make_shared<Semaphore>(device->handle());
        if (!renderingFinished->create())
            return false;

        imageAvailable = std::make_shared<Semaphore>(device->handle());
        if (!imageAvailable->create())
            return false;

        return true;
    }

    bool renderFrame()
    {
        for (size_t i = 0; i < vulkanModels.size(); ++i)
        {
            RendererModel& m = vulkanModels[i];
            const float aspect = extent.width / float(extent.height);
            scene->camera.aspectRatio = aspect;
            Matrices matrices;
            matrices.view       = scene->camera.viewMatrix();
            matrices.projection = scene->camera.projectionMatrix();
            matrices.model      = scene->models[i].worldTransform;
            matrices.normal     = glm::inverseTranspose(matrices.view * matrices.model);

            m.matricesUniformBuffer->copyHostVisible(&matrices, m.matricesUniformBuffer->size());
            if (m.model.material.type == Material::Type::Pbr)
                m.lightUniformBuffer->copyHostVisible(&scene->light, m.lightUniformBuffer->size());
        }

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            device->handle(),
            swapchain->handle(),
            std::numeric_limits<uint64_t>::max(),
            imageAvailable->handle(),
            VK_NULL_HANDLE,
            &imageIndex);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": next image acquire from swapchain failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;
            return false;
        }

        // Render
        Queue graphicsQueue(device->handle(), graphicsFamilyIndex, 0);
        graphicsQueue.create();
        graphicsQueue.submit(commandBuffers[imageIndex],
                             renderingFinished->handle(),
                             imageAvailable->handle(),
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        // Present
        Queue presentQueue(device->handle(), presentationFamilyIndex, 0);
        presentQueue.create();
        presentQueue.present(swapchain->handle(),
                             renderingFinished->handle(),
                             imageIndex);

        return true;
    }

    bool resized(const VkExtent2D& extent)
    {
        vkDeviceWaitIdle(device->handle());
        this->extent = extent;

        commandPool.reset();
        commandBuffers.clear();
        diffusePipeline.reset();
        pbrPipeline.reset();
        renderPass.reset();
        swapchain.reset();

        if (!createSwapchain())
            return false;
        if (!createRenderPass())
            return false;
        if (!createCommandPool())
            return false;
        if (!createPipelines())
            return false;
        if (!createCommandBuffers())
            return false;

        return true;
    }

    void destroy()
    {
        if (device->handle() == VK_NULL_HANDLE)
            return;

        vkDeviceWaitIdle(device->handle());

        commandPool->destroy();
        commandBuffers.clear();
        renderingFinished->destroy();
        imageAvailable->destroy();
        diffusePipeline->destroy();
        pbrPipeline->destroy();
        descriptorPool->destroy();

        for (RendererModel& m : vulkanModels)
            m.destroy();
        vulkanModels.clear();

        swapchain->destroy();
        renderPass->destroy();
        device->destroy();
    }

    bool isValid() const
    {
        return commandBuffers.size() > 0                         &&
               commandPool       && commandPool->isValid()       &&
               renderingFinished && renderingFinished->isValid() &&
               imageAvailable    && imageAvailable->isValid()    &&
               diffusePipeline   && diffusePipeline->isValid()   &&
               pbrPipeline       && pbrPipeline->isValid()       &&
               descriptorPool    && descriptorPool->isValid()    &&
               swapchain         && swapchain->isValid()         &&
               renderPass        && renderPass->isValid()        &&
               device            && device->isValid();
    }

    // From user.
    VkInstance instance             = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR surface            = VK_NULL_HANDLE;

    // Surface
    SurfaceProperties surfaceInfo;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;

    // Queue family indices.
    uint32_t graphicsFamilyIndex;
    uint32_t presentationFamilyIndex;

    // Device.
    std::shared_ptr<LogicalDevice> device;

    // Render pass.
    std::shared_ptr<RenderPass> renderPass;

    // Pipelines
    std::shared_ptr<RendererPipeline> diffusePipeline;
    std::shared_ptr<RendererPipeline> pbrPipeline;

    // Swapchain
    std::shared_ptr<Swapchain> swapchain;
    uint32_t swapchainImageCount;

    // Descriptor sets
    std::shared_ptr<DescriptorPool> descriptorPool;

    // Commands
    std::shared_ptr<CommandPool> commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    // Sync
    std::shared_ptr<Semaphore> renderingFinished;
    std::shared_ptr<Semaphore> imageAvailable;

    // Scene
    const std::shared_ptr<Scene> scene;

    // Vulkan models.
    std::vector<RendererModel> vulkanModels;
};

/* -------------------------------------------------------------------------- */

Renderer::Renderer(const VkInstance& instance,
                   const VkPhysicalDevice& physicalDevice,
                   const VkSurfaceKHR& surface,
                   const VkExtent2D& extent,
                   const std::shared_ptr<Scene>& scene)
    : impl(std::make_shared<Impl>(instance,
                                  physicalDevice,
                                  surface,
                                  extent,
                                  scene))
{}

bool Renderer::create()
{
    if (!isValid())
        return impl->create();
    return isValid();
}

void Renderer::destroy()
{
    if (isValid())
        impl->destroy();
}

bool Renderer::isValid() const
{ return impl->isValid(); }

bool Renderer::resized(const VkExtent2D& extent)
{ return impl->resized(extent); }

bool Renderer::renderFrame()
{ return impl->renderFrame(); }

} // namespace vk
} // namespace kuu
