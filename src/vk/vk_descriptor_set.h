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

    // Sets and gets the type
    DescriptorPool& setType(VkDescriptorType type);
    VkDescriptorType type() const;

    // Sets and gets the descriptor count. The default is 1.
    DescriptorPool& setDescriptorCount(uint32_t count);
    uint32_t descriptorCount() const;

    // Sets and gets the max descriptor set count that can be allocated from
    // the pool. The default is 1.
    DescriptorPool& setMaxSetCount(uint32_t count);
    uint32_t maxSetCount() const;

    // Creates and destroys the pool.
    void create();
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
   A vulkan descriptor set wrapper class
 * -------------------------------------------------------------------------- */
class DescriptorSet
{
public:
    // Constructs the descriptor set.
    DescriptorSet(const VkDevice& logicalDevice);

    // Sets and gets the layout binding.
    DescriptorSet& setLayoutBinding(const VkDescriptorSetLayoutBinding& layoutBinding);
    VkDescriptorSetLayoutBinding layoutBinding() const;

    // Sets and gets the pool.
    DescriptorSet& setPool(const VkDescriptorPool& pool);
    VkDescriptorPool pool() const;

    // Sets the buffer info. The descriptor set is set to be a buffer
    // descriptor set.
    DescriptorSet& setBufferInfo(const VkDescriptorBufferInfo& info);
    // Sets the image info. The descriptor set is set to be a image
    // descriptor set.
    DescriptorSet& setImageInfo(const VkDescriptorImageInfo& info);

    // Sets and gets the binding point of descriptor. The default binding
    // point is 0.
    DescriptorSet& setBindingPoint(uint32_t point);
    uint32_t bindingPoint() const;

    // Creates and destroys the descriptor set.
    void create();
    void destroy();

    // Returns true if the descriptor set handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the descriptor set handle.
    VkDescriptorSet handle() const;
    // Returns the  descriptor set layout handles
    VkDescriptorSetLayout layoutHandle() const;

    // Returns write description set.
    VkWriteDescriptorSet writeDescriptorSet() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
