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

    // Creates and destroys the pipeline.
    void create();
    void destroy();

    // Returns true if the pipeline handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the handle.
    VkPipeline handle() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
