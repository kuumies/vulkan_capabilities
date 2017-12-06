/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Texture2D struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

class CommandPool;
class Queue;

// A scoped two-dimensional texture. Constructs valid texture ready to be
// used as a sampler in shader. Texture is transmitted into device-only visible
// memory.
struct Texture2D
{
    // Loads a RGBA or grayscale image from disk and creates a texture out of it.
    Texture2D(const VkPhysicalDevice& physicalDevice,
              const VkDevice& device,
              Queue& queue,
              CommandPool& commandPool,
              const std::string& filePath,
              VkFilter magFilter,
              VkFilter minFilter,
              VkSamplerAddressMode addressModeU,
              VkSamplerAddressMode addressModeV,
              bool generateMipmaps);

    VkFormat format;
    VkImage image;
    VkImageView imageView;
    VkSampler sampler;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
