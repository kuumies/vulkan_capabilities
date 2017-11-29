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
        VkDescriptorPoolCreateInfo poolInfo;
        poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.pNext         = NULL;
        poolInfo.flags         = 0;
        poolInfo.maxSets       = maxCount;
        poolInfo.poolSizeCount = uint32_t(typeSizes.size());
        poolInfo.pPoolSizes    = typeSizes.data();

        const VkResult result =
            vkCreateDescriptorPool(
                logicalDevice,
                &poolInfo,
                NULL,
                &pool);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": descriptor pool creation failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;

            return;
        }
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
    std::vector<VkDescriptorPoolSize> typeSizes;
    uint32_t maxCount = 1;
};

/* -------------------------------------------------------------------------- */

DescriptorPool::DescriptorPool(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
}

DescriptorPool& DescriptorPool::addTypeSize(const VkDescriptorPoolSize& size)
{
    impl->typeSizes.push_back(size);
    return *this;
}

DescriptorPool& DescriptorPool::addTypeSize(
        VkDescriptorType type,
        uint32_t size)
{ return addTypeSize( { type, size } ); }

std::vector<VkDescriptorPoolSize> DescriptorPool::typeSizes() const
{ return impl->typeSizes; }

DescriptorPool& DescriptorPool::setMaxCount(uint32_t count)
{
    impl->maxCount = count;
    return *this;
}

uint32_t DescriptorPool::maxCount() const
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

struct DescriptorSets::Impl
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
        layoutInfo.bindingCount = uint32_t(layoutBindings.size());
        layoutInfo.pBindings    = layoutBindings.data();

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
            &descriptorSets);
    }

    void destroy()
    {
        vkDestroyDescriptorSetLayout(
            logicalDevice,
            layout,
            NULL);

        layout        = VK_NULL_HANDLE;
        descriptorSets = VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkDescriptorSet descriptorSets = VK_NULL_HANDLE;
    VkDescriptorSetLayout layout   = VK_NULL_HANDLE;

    // Data from user
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorBufferInfo bufferInfo;
    VkDescriptorImageInfo imageInfo;
    uint32_t bindingPoint = 0;
};

/* -------------------------------------------------------------------------- */

DescriptorSets::DescriptorSets(const VkDevice& logicalDevice,
                             const VkDescriptorPool& pool)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
    impl->pool          = pool;
}

DescriptorSets& DescriptorSets::setBufferInfo(const VkDescriptorBufferInfo& info)
{
    impl->bufferInfo = info;
    return *this;
}

DescriptorSets& DescriptorSets::setImageInfo(const VkDescriptorImageInfo& info)
{
    impl->imageInfo = info;
    return *this;
}

DescriptorSets& DescriptorSets::setBindingPoint(uint32_t point)
{
    impl->bindingPoint = point;
    return *this;
}

uint32_t DescriptorSets::bindingPoint() const
{ return impl->bindingPoint; }

DescriptorSets& DescriptorSets::addLayoutBinding(
    const VkDescriptorSetLayoutBinding& layoutBinding)
{
    impl->layoutBindings.push_back(layoutBinding);
    return *this;
}

DescriptorSets& DescriptorSets::addLayoutBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        uint32_t descriptorCount,
        VkShaderStageFlags stageFlags,
        const VkSampler* immutableSamplers)
{
    return addLayoutBinding( { binding,
                               descriptorType,
                               descriptorCount,
                               stageFlags,
                               immutableSamplers } );
}

std::vector<VkDescriptorSetLayoutBinding> DescriptorSets::layoutBindings() const
{ return impl->layoutBindings; }

void DescriptorSets::create()
{
    if (!isValid())
        impl->create();
}

void DescriptorSets::destroy()
{
    if (isValid())
        impl->destroy();
}

bool DescriptorSets::isValid() const
{ return impl->descriptorSets != VK_NULL_HANDLE; }

VkDescriptorSet DescriptorSets::handle() const
{ return impl->descriptorSets; }

VkDescriptorSetLayout DescriptorSets::layoutHandle() const
{ return impl->layout; }

void DescriptorSets::writeUniformBuffer(
    const std::vector<VkDescriptorBufferInfo>& bufferInfos)
{
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    for (size_t i = 0; i < bufferInfos.size(); ++i)
    {
        VkDescriptorBufferInfo info = bufferInfos[i];
        VkDescriptorSetLayoutBinding binding = impl->layoutBindings[i];
        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet          = impl->descriptorSets;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.pBufferInfo     = &info;
        writeDescriptorSet.dstBinding      = binding.binding;
        writeDescriptorSets.push_back(writeDescriptorSet);
    }

    vkUpdateDescriptorSets(
        impl->logicalDevice,
        uint32_t(writeDescriptorSets.size()),
        writeDescriptorSets.data(),
        0, NULL);
}
} // namespace vk
} // namespace kuu
