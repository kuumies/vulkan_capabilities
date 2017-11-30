/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Mesh class.
 * -------------------------------------------------------------------------- */

#include "vk_mesh.h"
#include <iostream>
#include "vk_buffer.h"
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct Mesh::Impl
{
    Impl(const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice)
        : logicalDevice(logicalDevice)
        , vertexBuffer(physicalDevice, logicalDevice)
        , indexBuffer(physicalDevice, logicalDevice)
    {}

    ~Impl()
    {
        destroy();
    }

    bool create()
    {
        vertexBuffer.setSize(vertices.size() * sizeof(float));
        vertexBuffer.setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        vertexBuffer.setMemoryProperties(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vertexBuffer.create();
        if (!vertexBuffer.isValid())
            return false;

        void* vertexDst = vertexBuffer.map();
        memcpy(vertexDst, vertices.data(), size_t(vertexBuffer.size()));
        vertexBuffer.unmap();

        indexBuffer.setSize(indices.size() * sizeof(uint32_t));
        indexBuffer.setUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        indexBuffer.setMemoryProperties(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        indexBuffer.create();
        if (!indexBuffer.isValid())
            return false;

        void* indexDst = indexBuffer.map();
        memcpy(indexDst, indices.data(), size_t(indexBuffer.size()));
        indexBuffer.unmap();
        return true;
    }

    void destroy()
    {
        indexBuffer.destroy();
        vertexBuffer.destroy();
    }

    // Parent
    VkDevice logicalDevice;

    // Childs
    Buffer vertexBuffer;
    Buffer indexBuffer;

    // From user.
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
    VkVertexInputBindingDescription vertexBindingDescription;
};

/* -------------------------------------------------------------------------- */

Mesh::Mesh(const VkPhysicalDevice& physicalDevice,
           const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>(physicalDevice, logicalDevice))
{}

Mesh& Mesh::setVertices(const std::vector<float>& vertices)
{
    impl->vertices = vertices;
    return *this;
}

std::vector<float> Mesh::vertices() const
{ return impl->vertices; }

Mesh& Mesh::setIndices(const std::vector<uint32_t>& indices)
{
    impl->indices = indices;
    return *this;
}

std::vector<uint32_t> Mesh::indices() const
{ return impl->indices; }

Mesh& Mesh::addVertexAttributeDescription(const VkVertexInputAttributeDescription& desc)
{
    impl->vertexAttributeDescriptions.push_back(desc);
    return *this;
}

Mesh& Mesh::addVertexAttributeDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
{
    return addVertexAttributeDescription( { location, binding, format, offset } );
}

std::vector<VkVertexInputAttributeDescription> Mesh::vertexAttributeDescriptions() const
{ return impl->vertexAttributeDescriptions; }

Mesh& Mesh::setVertexBindingDescription(const VkVertexInputBindingDescription& desc)
{
    impl->vertexBindingDescription = desc;
    return *this;
}

Mesh& Mesh::setVertexBindingDescription(
    uint32_t binding,
    uint32_t stride,
    VkVertexInputRate inputRate)
{
    return setVertexBindingDescription( { binding, stride, inputRate } );
}

VkVertexInputBindingDescription Mesh::vertexBindingDescription() const
{ return impl->vertexBindingDescription; }

bool Mesh::create()
{
    if (!isValid())
        return impl->create();
    return true;
}

void Mesh::destroy()
{
    if (isValid())
        impl->destroy();
}

bool Mesh::isValid() const
{ return impl->vertexBuffer.isValid() && impl->indexBuffer.isValid(); }

VkBuffer Mesh::vertexBufferHandle() const
{ return impl->vertexBuffer.handle(); }

VkBuffer Mesh::indexBufferHandle() const
{ return impl->indexBuffer.handle(); }

} // namespace vk
} // namespace kuu
