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
    // Creates a null texture
    Texture2D(const VkDevice& device);
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
    // Creates an empty texture with undefined layout
    Texture2D(const VkPhysicalDevice& physicalDevice,
              const VkDevice& device,
              const VkExtent2D& extent,
              const VkFormat& format,
              VkFilter magFilter,
              VkFilter minFilter,
              VkSamplerAddressMode addressModeU,
              VkSamplerAddressMode addressModeV);

    VkFormat format;
    VkExtent2D extent;
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

// Transitions texture to given in layout.
bool transitionTexture(Queue& queue,
                       CommandPool& commandPool,
                       std::shared_ptr<Texture2D> texture,
                       VkImageLayout from,
                       VkImageLayout to);

//bool transitionTexture(const VkCommandBuffer& cmdBuf,
//                       const VkImage& image,
//                       VkImageLayout from,
//                       VkImageLayout to);

// A scoped texture cube texture.
struct TextureCube
{
    // Loads a RGBA or grayscale texture cube images from disk and
    // creates a texture cube out of them.
    TextureCube(const VkPhysicalDevice& physicalDevice,
                const VkDevice& device,
                Queue& queue,
                CommandPool& commandPool,
                const std::vector<std::string>& filePaths,
                VkFilter magFilter,
                VkFilter minFilter,
                VkSamplerAddressMode addressModeU,
                VkSamplerAddressMode addressModeV,
                VkSamplerAddressMode addressModeW);

    // Texture cube of given in extent. Pixels content is undefined.
    TextureCube(const VkPhysicalDevice& physicalDevice,
                const VkDevice& device,
                const VkExtent3D& extent,
                const VkFormat& format,
                VkFilter magFilter,
                VkFilter minFilter,
                VkSamplerAddressMode addressModeU,
                VkSamplerAddressMode addressModeV,
                VkSamplerAddressMode addressModeW);

    VkFormat format;
    VkImage image;
    VkImageView imageView;
    VkSampler sampler;
    VkDeviceMemory memory;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
