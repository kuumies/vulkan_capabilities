/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::DescriptorSet class
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
   A vulkan descriptor pool wrapper class
 * -------------------------------------------------------------------------- */
class DescriptorPool
{
public:
    // Constructs the pool.
    DescriptorPool(const VkDevice& logicalDevice);

    // Adds a size of descriptor types.
    DescriptorPool& addTypeSize(const VkDescriptorPoolSize& size);
    DescriptorPool& addTypeSize(VkDescriptorType type, uint32_t size);
    // Returns the descriptor type sizes.
    std::vector<VkDescriptorPoolSize> typeSizes() const;

    // Sets and gets the max descriptor set count that can be allocated from
    // the pool. The default is 1.
    DescriptorPool& setMaxCount(uint32_t count);
    uint32_t maxCount() const;

    // Creates and destroys the pool.
    bool create();
    void destroy();

    // Returns true if the descriptor pool handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the handle.
    VkDescriptorPool handle() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

/* -------------------------------------------------------------------------- *
   A vulkan descriptor sets wrapper class
 * -------------------------------------------------------------------------- */
class DescriptorSets
{
public:
    // Constructs the descriptor sets.
    DescriptorSets(const VkDevice& logicalDevice,
                   const VkDescriptorPool& pool);

    // Adds a layout binding.
    DescriptorSets& addLayoutBinding(const VkDescriptorSetLayoutBinding& layoutBinding);
    DescriptorSets& addLayoutBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        uint32_t descriptorCount,
        VkShaderStageFlags stageFlags,
        const VkSampler* immutableSamplers = NULL);
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings() const;

    // Sets the descriptor set layout. If not set then it is created.
    DescriptorSets& setLayout(VkDescriptorSetLayout layout);

    // Sets the buffer info. The descriptor set is set to be a buffer
    // descriptor set.
    DescriptorSets& setBufferInfo(const VkDescriptorBufferInfo& info);
    // Sets the image info. The descriptor set is set to be a image
    // descriptor set.
    DescriptorSets& setImageInfo(const VkDescriptorImageInfo& info);

    // Sets and gets the binding point of descriptor. The default binding
    // point is 0.
    DescriptorSets& setBindingPoint(uint32_t point);
    uint32_t bindingPoint() const;

    // Creates and destroys the descriptor set.
    bool create();
    void destroy();

    // Returns true if the descriptor set and layout handles are not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the descriptor set handle.
    VkDescriptorSet handle() const;
    // Returns the  descriptor set layout handles
    VkDescriptorSetLayout layoutHandle() const;

    // Updates the uniform buffer descriptor set.
    void writeUniformBuffer(
        uint32_t binding,
        VkBuffer buffer,
        VkDeviceSize offset,
        VkDeviceSize range);

    // Updates the combined image sampler descriptor set.
    void writeImage(
        uint32_t binding,
        VkSampler sampler,
        VkImageView imageView,
        VkImageLayout imageLayout);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
