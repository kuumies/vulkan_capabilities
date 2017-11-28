/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::DescriptorSet class
 * -------------------------------------------------------------------------- */

#include "vk_descriptor_set.h"
#include <iostream>
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct DescriptorPool::Impl
{
    ~Impl()
    {
        destroy();
    }

    void create()
    {
        VkDescriptorPoolSize poolSize;
        poolSize.type            = type;
        poolSize.descriptorCount = descriptorCount;

        VkDescriptorPoolCreateInfo poolInfo;
        poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.pNext         = NULL;
        poolInfo.flags         = 0;
        poolInfo.maxSets       = maxCount;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes    = &poolSize;

        const VkResult result =
            vkCreateDescriptorPool(
                logicalDevice,
                &poolInfo,
                NULL,
                &pool);
    }

    void destroy()
    {
        vkDestroyDescriptorPool(
            logicalDevice,
            pool,
            NULL);

        pool = VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkDescriptorPool pool = VK_NULL_HANDLE;

    // User set properties.
    VkDescriptorType type;
    uint32_t descriptorCount = 1;
    uint32_t maxCount = 1;
};

/* -------------------------------------------------------------------------- */

DescriptorPool::DescriptorPool(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
}

DescriptorPool& DescriptorPool::setType(VkDescriptorType type)
{
    impl->type = type;
    return *this;
}

VkDescriptorType DescriptorPool::type() const
{ return impl->type; }

DescriptorPool& DescriptorPool::setDescriptorCount(uint32_t count)
{
    impl->descriptorCount = count;
    return *this;
}

uint32_t DescriptorPool::descriptorCount() const
{ return impl->descriptorCount; }

DescriptorPool& DescriptorPool::setMaxSetCount(uint32_t count)
{
    impl->maxCount = count;
    return *this;
}

uint32_t DescriptorPool::maxSetCount() const
{ return impl->maxCount; }

void DescriptorPool::create()
{
    if (!isValid())
        impl->create();
}

void DescriptorPool::destroy()
{
    if (isValid())
        impl->destroy();
}

bool DescriptorPool::isValid() const
{ return impl->pool != VK_NULL_HANDLE; }

VkDescriptorPool DescriptorPool::handle() const
{ return impl->pool; }

/* -------------------------------------------------------------------------- */

struct DescriptorSet::Impl
{
    Impl()
    {
        bufferInfo.buffer   = VK_NULL_HANDLE;
        imageInfo.imageView = VK_NULL_HANDLE;
    }

    ~Impl()
    {
        destroy();
    }

    void create()
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext        = NULL;
        layoutInfo.flags        = 0;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings    = &layoutBinding;

        VkResult result =
            vkCreateDescriptorSetLayout(
                logicalDevice,
                &layoutInfo,
                NULL,
                &layout);

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts        = &layout;

        result = vkAllocateDescriptorSets(
            logicalDevice,
            &allocInfo,
            &descriptorSet);
    }

    void destroy()
    {
        vkDestroyDescriptorSetLayout(
            logicalDevice,
            layout,
            NULL);

        layout        = VK_NULL_HANDLE;
        descriptorSet = VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;

    // Data from user
    VkDescriptorSetLayoutBinding layoutBinding;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorBufferInfo bufferInfo;
    VkDescriptorImageInfo imageInfo;
    uint32_t bindingPoint = 0;
};

/* -------------------------------------------------------------------------- */

DescriptorSet::DescriptorSet(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice  = logicalDevice;
}

DescriptorSet& DescriptorSet::setPool(const VkDescriptorPool& pool)
{
    impl->pool = pool;
    return *this;
}

VkDescriptorPool DescriptorSet::pool() const
{ return impl->pool; }

DescriptorSet& DescriptorSet::setBufferInfo(const VkDescriptorBufferInfo& info)
{
    impl->bufferInfo = info;
    return *this;
}

DescriptorSet& DescriptorSet::setImageInfo(const VkDescriptorImageInfo& info)
{
    impl->imageInfo = info;
    return *this;
}

DescriptorSet& DescriptorSet::setBindingPoint(uint32_t point)
{
    impl->bindingPoint = point;
    return *this;
}

uint32_t DescriptorSet::bindingPoint() const
{ return impl->bindingPoint; }

DescriptorSet& DescriptorSet::setLayoutBinding(
    const VkDescriptorSetLayoutBinding& layoutBinding)
{
    impl->layoutBinding = layoutBinding;
    return *this;
}

VkDescriptorSetLayoutBinding DescriptorSet::layoutBinding() const
{ return impl->layoutBinding; }

void DescriptorSet::create()
{
    if (!isValid())
        impl->create();
}

void DescriptorSet::destroy()
{
    if (isValid())
        impl->destroy();
}

bool DescriptorSet::isValid() const
{ return impl->descriptorSet != VK_NULL_HANDLE; }

VkDescriptorSet DescriptorSet::handle() const
{ return impl->descriptorSet; }

VkDescriptorSetLayout DescriptorSet::layoutHandle() const
{ return impl->layout; }

VkWriteDescriptorSet DescriptorSet::writeDescriptorSet() const
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet          = impl->descriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    if (impl->bufferInfo.buffer != VK_NULL_HANDLE)
        writeDescriptorSet.pBufferInfo     = &impl->bufferInfo;
    if (impl->imageInfo.imageView != VK_NULL_HANDLE)
        writeDescriptorSet.pImageInfo     = &impl->imageInfo;
    writeDescriptorSet.dstBinding      = impl->bindingPoint;
    return writeDescriptorSet;
}

} // namespace vk
} // namespace kuu
