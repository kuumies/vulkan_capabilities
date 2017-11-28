/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::ShaderModule class
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
   A vulkan shader module wrapper class.
 * -------------------------------------------------------------------------- */
class ShaderModule
{
public:
    // Constructs the shader module.
    ShaderModule(const VkDevice& device);
    ShaderModule(const VkDevice& device, const std::string& filePath);

    // Sets and gets the stage.
    ShaderModule& setStage(VkShaderStageFlagBits stage);
    VkShaderStageFlagBits stage() const;

    // Sets and gets the stage "main" name.
    ShaderModule& setStageName(const std::string& name);
    std::string stageName() const;

    // Sets and gets the SPIR-V source code.
    ShaderModule& setSourceCode(const std::vector<char>& source);
    std::vector<char> sourceCode() const;

    // Creates and destroys the shader module.
    void create();
    void destroy();

    // Returns true if the shader module handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the shader stage create info.
    VkPipelineShaderStageCreateInfo createInfo() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
