/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::IrradianceRenderer class
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

class IrradianceRenderer
{
public:
    IrradianceRenderer(
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
