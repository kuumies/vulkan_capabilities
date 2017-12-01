/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Image class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A Vulkan sampler wrapper class.
 * -------------------------------------------------------------------------- */
class Sampler
{
public:
    // Constructs the sampler.
    Sampler(const VkDevice& logicalDevice = VK_NULL_HANDLE);

    // Sets and gets the logical device.
    Sampler& setDevice(const VkDevice& logicalDevice);
    VkDevice device() const;

    // Creates and destroys the sampler.
    bool create();
    void destroy();

    // Returns true if the sampler handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the handle.
    VkSampler handle() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

/* -------------------------------------------------------------------------- *
   A Vulkan image and image view wrapper class. This will also manage the
   memory of the image.
 * -------------------------------------------------------------------------- */
class Image
{
public:
    // Constructs the image.
    Image(const VkPhysicalDevice& physicalDevice,
          const VkDevice& logicalDevice);

    // Sets and gets the image type.
    Image& setType(VkImageType type);
    VkImageType type() const;

    // Sets and gets the image format.
    Image& setFormat(VkFormat format);
    VkFormat format() const;

    // Sets and gets the image format.
    Image& setExtent(VkExtent3D extent);
    VkExtent3D extent() const;

    // Sets and gets the image tiling.
    Image& setTiling(VkImageTiling tiling);
    VkImageTiling tiling() const;

    // Sets and gets the image usage.
    Image& setUsage(VkImageUsageFlags usage);
    VkImageUsageFlags usage() const;

    // Sets and gets the image layout.
    Image& setInitialLayout(VkImageLayout initialLayout);
    VkImageLayout initialLayout() const;

    // Sets and gets the image view aspect.
    Image& setImageViewAspect(VkImageAspectFlags aspect);
    VkImageAspectFlags imageViewAspectlayout() const;

    // Sets and gets the image  memory properties
    Image& setMemoryProperty(VkMemoryPropertyFlags property);
    VkMemoryPropertyFlags memoryProperty() const;

    // Sets and gets the image sampler.
    Image& setSampler(const Sampler& sampler);
    Sampler sampler() const;

    // Creates and destroys the image.
    bool create();
    void destroy();

    // Returns true if the image handles are valid.
    bool isValid() const;

    // Returns the image handle.
    VkImage imageHandle() const;
    // Returns the image view handle.
    VkImageView imageViewHandle() const;

    // Transition image layout. This is a queue operation.
    bool transitionLayout(
        const VkImageLayout& newLayout,
        class Queue& queue,
        class CommandPool& commandPool);

    // Copy image data from buffer. This is a queue operation. If the regions
    // vector is empty then whole image is set as the region.
    bool copyFromBuffer(
        const class Buffer& buffer,
        class Queue& queue,
        class CommandPool& commandPool,
        const std::vector<VkBufferImageCopy>& regions =
            std::vector<VkBufferImageCopy>());

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
