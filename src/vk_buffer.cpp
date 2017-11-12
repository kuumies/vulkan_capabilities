/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Buffer class
 * ---------------------------------------------------------------- */

#include "vk_buffer.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

// Returns the memory type index based on the memory type and
// needed memory properties.
uint32_t memoryTypeIndex(
    VkPhysicalDevice physicalDevice,
    VkMemoryRequirements memRequirements,
    uint32_t propertyFlags)
{
    // Get physical device memory properties
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    // Go trought the memory types.
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        // Check that the type bit is enabled.
        const uint32_t memoryTypeBits = (i << i);
        if (!(memRequirements.memoryTypeBits & memoryTypeBits))
            continue;

        // Check that the memory supports required properties.
        if (!(memProperties.memoryTypes[i].propertyFlags & propertyFlags))
            continue;

        return i;
    }

    throw std::runtime_error(
        __FUNCTION__ +
        std::string(": failed to find correct type of memory "
                    "for vertex buffer"));
}

/* ---------------------------------------------------------------- */

struct Buffer::Data
{
    // From user
    VkDevice device;
    VkPhysicalDevice physicalDevice;

    // Vulkan stuff vertex buffer
    VkBufferCreateInfo bufferInfo;
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    // Creation status.
    bool created = false;
};

/* ---------------------------------------------------------------- */

Buffer::Buffer(VkDevice device, VkPhysicalDevice physicalDevice)
    : d(std::make_shared<Data>())
{
    d->device         = device;
    d->physicalDevice = physicalDevice;
}

/* ---------------------------------------------------------------- */

VkBuffer Buffer::handle() const
{ return d->buffer; }

/* ---------------------------------------------------------------- */

void Buffer::create(VkDeviceSize size, VkBufferUsageFlagBits usage, void* bufData)
{
    // Destroys old buffer if it exists.
    destroy();

    // Create the vertex buffer.
    //  * buffer is exclusive to graphics queue
    d->bufferInfo = VkBufferCreateInfo{};
    d->bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    d->bufferInfo.size        = size;
    d->bufferInfo.usage       = usage;
    d->bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(d->device, &d->bufferInfo,
                                     nullptr, &d->buffer);
    if (result != VK_SUCCESS)
        throw std::runtime_error(
            __FUNCTION__ +
            std::string(": failed to create vertex buffer"));

    // Get the memory requirements for the vertex buffer.
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(d->device, d->buffer,
                                  &memRequirements);

    // Allocate memory for the vertex buffer.
    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize  = memRequirements.size;
    memoryAllocInfo.memoryTypeIndex =
        memoryTypeIndex(d->physicalDevice,
                        memRequirements,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    result = vkAllocateMemory(d->device, &memoryAllocInfo,
                              nullptr, &d->bufferMemory);
    if (result != VK_SUCCESS)
        throw std::runtime_error(
            __FUNCTION__ +
            std::string(": failed to alloc memory for vertex buffer"));

    // Associate the mememory to buffer.
    vkBindBufferMemory(d->device, d->buffer, d->bufferMemory, 0);


    // Map the GPU buffer memory into CPU memory.
    void* data;
    vkMapMemory(d->device, d->bufferMemory, 0,
                d->bufferInfo.size, 0, &data);

    // Copy vertex data into buffer memory
    memcpy(data, bufData, (size_t) d->bufferInfo.size);

    // Unmap the GPU memory. The vertex data is immeadetly copies as
    // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT flag was used when finding
    // the fitting memory for vertex buffer.
    vkUnmapMemory(d->device, d->bufferMemory);

    // Update created status.
    d->created = true;
}

/* ---------------------------------------------------------------- */

void Buffer::destroy()
{
    if (!d->created)
        return;

    vkDestroyBuffer(d->device, d->buffer, nullptr);
    vkFreeMemory(d->device, d->bufferMemory, nullptr);
}

} // namespace vk
} // namespace kuu
