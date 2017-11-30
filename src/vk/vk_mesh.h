/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Mesh class.
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
   A vulkan mesh class.
 * -------------------------------------------------------------------------- */
class Mesh
{
public:
    // Constructs the mesh.
    Mesh(const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice);

    // Sets and gets the vertex data.
    Mesh& setVertices(const std::vector<float>& vertices);
    std::vector<float> vertices() const;

    // Sets and gets the index data.
    Mesh& setIndices(const std::vector<uint32_t>& indices);
    std::vector<uint32_t> indices() const;

    // Adds a vertex attribute description.
    Mesh& addVertexAttributeDescription(const VkVertexInputAttributeDescription& desc);
    Mesh& addVertexAttributeDescription(
        uint32_t location,
        uint32_t binding,
        VkFormat format,
        uint32_t offset);
    // Returns the vertex attribute descriptions.
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions() const;

    // Sets the vertex binding description.
    Mesh& setVertexBindingDescription(const VkVertexInputBindingDescription& desc);
    Mesh& setVertexBindingDescription(
        uint32_t binding,
        uint32_t stride,
        VkVertexInputRate inputRate);
    VkVertexInputBindingDescription vertexBindingDescription() const;

    // Creates and destroys the mesh.
    bool create();
    void destroy();

    // Returns true if the vertex and index buffer handles are not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the vertex and index buffer handles.
    VkBuffer vertexBufferHandle() const;
    VkBuffer indexBufferHandle() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
