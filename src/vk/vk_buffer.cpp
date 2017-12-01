/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Buffer class
 * -------------------------------------------------------------------------- */

#include "vk_buffer.h"
#include <iostream>
#include "vk_helper.h"
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct Buffer::Impl
{
    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {
        VkBufferCreateInfo bufferInfo;
        bufferInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext                 = NULL;
        bufferInfo.flags                 = 0;
        bufferInfo.size                  = size;
        bufferInfo.usage                 = usage;
        bufferInfo.sharingMode           = sharingMode;
        bufferInfo.queueFamilyIndexCount = uint32_t(queueFamilyIndices.size());
        bufferInfo.pQueueFamilyIndices   = queueFamilyIndices.data();

        VkResult result =
            vkCreateBuffer(
                logicalDevice,
                &bufferInfo,
                NULL,
                &buffer);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create buffer - "
                      << vk::stringify::result(result)
                      << std::endl;
            return false;
        }

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(
            physicalDevice,     // [in]  physical device handle
            &memoryProperties); // [out] memory properties

        VkMemoryRequirements mememoryRequirements;
        vkGetBufferMemoryRequirements(
            logicalDevice,
            buffer,
            &mememoryRequirements);

        VkMemoryAllocateInfo memoryAllocInfo;
        memoryAllocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocInfo.pNext           = NULL;
        memoryAllocInfo.allocationSize  = mememoryRequirements.size;
        memoryAllocInfo.memoryTypeIndex =
            helper::findMemoryTypeIndex(
                memoryProperties,
                mememoryRequirements,
                memoryFlags);

        result = vkAllocateMemory(
            logicalDevice,
            &memoryAllocInfo,
            NULL,
            &bufferMemory);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to allocate memory for buffer - "
                      << vk::stringify::result(result)
                      << std::endl;
            return false;
        }

        result = vkBindBufferMemory(
            logicalDevice,
            buffer,
            bufferMemory,
            0);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to bind memory for buffer - "
                      << vk::stringify::result(result)
                      << std::endl;
            return false;
        }

        return true;
    }

    void destroy()
    {
        vkDestroyBuffer(
            logicalDevice,
            buffer,
            NULL);

        vkFreeMemory(
            logicalDevice,
            bufferMemory,
            NULL);

        buffer       = VK_NULL_HANDLE;
        bufferMemory = VK_NULL_HANDLE;
    }

    bool isValid() const
    {
        return buffer       != VK_NULL_HANDLE &&
               bufferMemory != VK_NULL_HANDLE;
    }

    // Parent
    VkPhysicalDevice physicalDevice;
    // Child
    VkDevice logicalDevice;

    // Data from user
    VkDeviceSize size;
    VkBufferUsageFlagBits usage;
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    std::vector<uint32_t> queueFamilyIndices;
    VkMemoryPropertyFlags memoryFlags;

    // Vulkan stuff vertex buffer
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
};

/* -------------------------------------------------------------------------- */

Buffer::Buffer(const VkPhysicalDevice& physicalDevice,
               const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->physicalDevice = physicalDevice;
    impl->logicalDevice  = logicalDevice;
}

Buffer& Buffer::setSize(VkDeviceSize size)
{
    impl->size = size;
    return *this;
}

VkDeviceSize Buffer::size() const
{ return impl->size; }

Buffer& Buffer::setUsage(VkBufferUsageFlagBits usage)
{
    impl->usage = usage;
    return *this;
}

VkBufferUsageFlagBits Buffer::usage() const
{ return impl->usage; }

Buffer& Buffer::setSharingMode(VkSharingMode mode)
{
    impl->sharingMode = mode;
    return *this;
}

VkSharingMode Buffer::shardingMode() const
{ return impl->sharingMode; }

Buffer& Buffer::setQueueFamilyIndices(const std::vector<uint32_t>& indices)
{
    impl->queueFamilyIndices = indices;
    return *this;
}

std::vector<uint32_t> Buffer::queueFamilyIndices() const
{ return impl->queueFamilyIndices; }

Buffer& Buffer::setMemoryProperties(VkMemoryPropertyFlags properties)
{
    impl->memoryFlags = properties;
    return *this;
}

VkMemoryPropertyFlags Buffer::memoryProperties() const
{ return impl->memoryFlags; }

VkBuffer Buffer::handle() const
{ return impl->buffer; }

bool Buffer::create()
{
    if (!isValid())
        return impl->create();
    return true;
}

void Buffer::destroy()
{
    if (isValid())
        impl->destroy();
}

bool Buffer::isValid() const
{ return impl->isValid(); }

void* Buffer::map()
{
    return map(0, impl->size, 0);
}

void* Buffer::map(VkDeviceSize offset,
                  VkDeviceSize size,
                  VkMemoryMapFlags flags)
{
    void* data;
    const VkResult result = vkMapMemory(
        impl->logicalDevice,
        impl->bufferMemory,
        offset,
        size,
        flags,
        &data);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to map buffer - "
                  << vk::stringify::result(result)
                  << std::endl;
        return nullptr;
    }

    return data;
}

void Buffer::unmap()
{
    vkUnmapMemory(
        impl->logicalDevice,
        impl->bufferMemory);
}

} // namespace vk
} // namespace kuu
