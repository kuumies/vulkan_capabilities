/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Mesh class
 * ---------------------------------------------------------------- */

#include "vk_mesh.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct Mesh::Data
{
    Data(VkDevice device, VkPhysicalDevice physicalDevice,
         const uint32_t bindingNumber)
        : device(device)
        , physicalDevice(physicalDevice)
        , bindingNumber(bindingNumber)
    {}

    ~Data()
    {
        destroy();
    }

    void create(VkDeviceSize size)
    {
        // Create the binding description
        bindingDescription = VkVertexInputBindingDescription{};
        bindingDescription.binding   = bindingNumber;
        bindingDescription.stride    = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        // Create the vertex input attribute descriptions for each
        // vertex parameter.
        attributeDescriptions.clear();
        attributeDescriptions.resize(2);

        attributeDescriptions[0].binding  = bindingNumber;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset   = offsetof(Vertex, pos);

        attributeDescriptions[1].binding  = bindingNumber;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset   = offsetof(Vertex, color);

        // Create the vertex buffer.
        //  * buffer is exclusive to graphics queue
        bufferInfo = VkBufferCreateInfo{};
        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size        = size;
        bufferInfo.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(device, &bufferInfo,
                                         nullptr, &vertexBuffer);
        if (result != VK_SUCCESS)
            throw std::runtime_error(
                __FUNCTION__ +
                std::string(": failed to create vertex buffer"));

        created = true;
    }

    void alloc()
    {
        // Get the memory requirements for the vertex buffer.
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, vertexBuffer,
                                      &memRequirements);

        // Allocate memory for the vertex buffer.
        VkMemoryAllocateInfo memoryAllocInfo = {};
        memoryAllocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocInfo.allocationSize  = memRequirements.size;
        memoryAllocInfo.memoryTypeIndex =
            memoryTypeIndex(memRequirements,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkResult result = vkAllocateMemory(device, &memoryAllocInfo,
                                           nullptr, &vertexBufferMemory);
        if (result != VK_SUCCESS)
            throw std::runtime_error(
                __FUNCTION__ +
                std::string(": failed to alloc memory for vertex buffer"));

        // Associate the mememory to buffer.
        vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

    }

    void write(const std::vector<Vertex>& vertices)
    {
        // Map the GPU buffer memory into CPU memory.
        void* data;
        vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);

        // Copy vertex data into buffer memory
        memcpy(data, vertices.data(), (size_t) bufferInfo.size);

        // Unmap the GPU memory. The vertex data is immeadetly copies as
        // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT flag was used when finding
        // the fitting memory for vertex buffer.
        vkUnmapMemory(device, vertexBufferMemory);

        // Take the vertex count.
        vertexCount = uint32_t(vertices.size());
    }

    void destroy()
    {
        if (!created)
            return;

        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);
    }

    // Returns the memory type index based on the memory type and
    // needed memory properties.
    uint32_t memoryTypeIndex(
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

    // From user
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    uint32_t bindingNumber;

    // Vulkan stuff for vertex buffer, binding and rendering
    VkVertexInputBindingDescription bindingDescription;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    VkBufferCreateInfo bufferInfo;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    // Vertex count.
    uint32_t vertexCount = 0;
    // Creation status.
    bool created = false;
};

/* ---------------------------------------------------------------- */

Mesh::Mesh(VkDevice device,
           VkPhysicalDevice physicalDevice,
           const uint32_t bindingNumber)
    : d(std::make_shared<Data>(device, physicalDevice, bindingNumber))
{}

/* ---------------------------------------------------------------- */

VkVertexInputBindingDescription Mesh::bindingDescription()
{ return d->bindingDescription; }

/* ---------------------------------------------------------------- */

std::vector<VkVertexInputAttributeDescription> Mesh::attributeDescriptions() const
{ return d->attributeDescriptions; }

/* ---------------------------------------------------------------- */

void Mesh::write(const std::vector<Vertex>& vertices)
{
    d->destroy();
    d->create(sizeof(vertices[0]) * vertices.size());
    d->alloc();
    d->write(vertices);
}

/* ---------------------------------------------------------------- */

void Mesh::draw(VkCommandBuffer commandBuffer)
{
    // Bind the vertex buffer.
    VkBuffer vertexBuffers[] = { d->vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    // Draw the vertex buffer
    vkCmdDraw(commandBuffer, d->vertexCount, 1, 0, 0);
}

} // namespace vk
} // namespace kuu
