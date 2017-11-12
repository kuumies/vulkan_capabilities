/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Mesh class
 * ---------------------------------------------------------------- */

#include "vk_mesh.h"

/* ---------------------------------------------------------------- */

#include "vk_buffer.h"

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
        , buffer(device, physicalDevice)
    {}

    // From user
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    uint32_t bindingNumber;

    // Vertex buffer
    Buffer buffer;

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

VkVertexInputBindingDescription Mesh::bindingDescription() const
{
    VkVertexInputBindingDescription bindingDescription;
    bindingDescription.binding   = d->bindingNumber;
    bindingDescription.stride    = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

/* ---------------------------------------------------------------- */

std::vector<VkVertexInputAttributeDescription>
    Mesh::attributeDescriptions() const
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.clear();
    attributeDescriptions.resize(2);

    attributeDescriptions[0].binding  = d->bindingNumber;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset   = offsetof(Vertex, pos);

    attributeDescriptions[1].binding  = d->bindingNumber;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset   = offsetof(Vertex, color);

    return attributeDescriptions;
}

/* ---------------------------------------------------------------- */

void Mesh::create(std::vector<Vertex> vertices)
{
    d->vertexCount = uint32_t(vertices.size());
    d->buffer.destroy();
    d->buffer.create(sizeof(vertices[0]) * vertices.size(),
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     vertices.data());
}

/* ---------------------------------------------------------------- */

void Mesh::destroy()
{
    d->buffer.destroy();
}

/* ---------------------------------------------------------------- */

void Mesh::draw(VkCommandBuffer commandBuffer)
{
    // Bind the vertex buffer.
    VkBuffer vertexBuffers[] = { d->buffer.handle() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1,
                           vertexBuffers, offsets);

    // Draw the vertex buffer
    vkCmdDraw(commandBuffer, d->vertexCount, 1, 0, 0);
}

} // namespace vk
} // namespace kuu
