/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::IblPrefilterRenderer class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

struct TextureCube;

class IblPrefilterRenderer
{
public:
    IblPrefilterRenderer(
        const VkPhysicalDevice& physicalDevice,
        const VkDevice& device,
        const uint32_t& graphicsQueueFamilyIndex,
        std::shared_ptr<TextureCube> inputTextureCube);

    void render();

    std::shared_ptr<TextureCube> textureCube() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
