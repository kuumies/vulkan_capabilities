/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::IblBrdfLutRenderer class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

struct Texture2D;

class IblBrdfLutRenderer
{
public:
    IblBrdfLutRenderer(
        const VkPhysicalDevice& physicalDevice,
        const VkDevice& device,
        const uint32_t& graphicsQueueFamilyIndex);

    void render();

    std::shared_ptr<Texture2D> texture() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
