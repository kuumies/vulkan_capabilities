/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Texture2D struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <map>
#include <memory>
#include <string>
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
    Texture2D(const VkDevice& device);
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
    VkDeviceMemory memory;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

std::map<std::string, std::shared_ptr<Texture2D>>
    loadtextures(std::vector<std::string> filepaths,
                 const VkPhysicalDevice& physicalDevice,
                 const VkDevice& device,
                 Queue& queue,
                 CommandPool& commandPool,
                 VkFilter magFilter,
                 VkFilter minFilter,
                 VkSamplerAddressMode addressModeU,
                 VkSamplerAddressMode addressModeV,
                 bool generateMipmaps);

} // namespace vk
} // namespace kuu
