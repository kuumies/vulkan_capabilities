/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::ShaderModule class
 * -------------------------------------------------------------------------- */

#include "vk_shader_module.h"
#include <fstream>
#include <iostream>
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{
namespace
{

/* -------------------------------------------------------------------------- */

std::vector<char> readShaderSourceFile(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << __FUNCTION__
                  << std::string(": failed to open shader file ")
                  << path
                  << std::endl;
        return std::vector<char>();
    }

    size_t fileSize = (size_t) file.tellg();
    if (fileSize == 0)
    {
        std::cerr << __FUNCTION__
                  << std::string(": shader source is zero-sized ")
                  << path
                  << std::endl;
        return std::vector<char>();
    }

    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct ShaderModule::Impl
{
    Impl(const VkDevice& device)
        : logicalDevice(device)
    {}

    ~Impl()
    {
        destroy();
    }

    bool create()
    {
        VkShaderModuleCreateInfo info;
        info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.pNext    = NULL;
        info.flags    = 0;
        if (source.size() > 0)
        {
            info.codeSize = source.size();
            info.pCode    = reinterpret_cast<const uint32_t*>(source.data());
        }

        const VkResult result =
            vkCreateShaderModule(
                logicalDevice,
                &info,
                NULL,
                &shaderModule);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": shader module creation failed as "
                      << vk::stringify::result(result)
                      << std::endl;
            return false;
        }

        return true;
    }

    void destroy()
    {
        vkDestroyShaderModule(
            logicalDevice,
            shaderModule,
            NULL);

        shaderModule = VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice = VK_NULL_HANDLE;

    // Child
    VkShaderModule shaderModule = VK_NULL_HANDLE;

    // From User
    VkShaderStageFlagBits stage;
    std::string stageName = "main";
    std::vector<char> source;
};

/* -------------------------------------------------------------------------- */

ShaderModule::ShaderModule(const VkDevice& device)
    : impl(std::make_shared<Impl>(device))
{}

ShaderModule::ShaderModule(const VkDevice& device, const std::string& filePath)
    : impl(std::make_shared<Impl>(device))
{
    impl->source = readShaderSourceFile(filePath);
}

ShaderModule& ShaderModule::setStage(VkShaderStageFlagBits stage)
{
    impl->stage = stage;
    return *this;
}

VkShaderStageFlagBits ShaderModule::stage() const
{ return impl->stage; }

ShaderModule& ShaderModule::setStageName(const std::string& name)
{
    impl->stageName = name;
    return* this;
}

std::string ShaderModule::stageName() const
{ return impl->stageName; }

ShaderModule& ShaderModule::setSourceCode(const std::vector<char>& source)
{
    impl->source = source;
    return *this;
}

std::vector<char> ShaderModule::sourceCode() const
{ return impl->source; }

bool ShaderModule::create()
{
    if (!isValid())
        return impl->create();
    return true;
}

void ShaderModule::destroy()
{
    if (isValid())
        impl->destroy();
}

bool ShaderModule::isValid() const
{ return impl->shaderModule != VK_NULL_HANDLE; }

VkPipelineShaderStageCreateInfo ShaderModule::createInfo() const
{
    VkPipelineShaderStageCreateInfo vshStageInfo = {};
    vshStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vshStageInfo.stage  = impl->stage;
    vshStageInfo.module = impl->shaderModule;
    vshStageInfo.pName  = impl->stageName.c_str();
    return vshStageInfo;
}

} // namespace vk
} // namespace kuu
