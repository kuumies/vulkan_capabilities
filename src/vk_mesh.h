/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Mesh class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- *
   A mesh vertex.
 * ---------------------------------------------------------------- */
struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
};

/* ---------------------------------------------------------------- *
   A vulkan mesh buffer.
 * ---------------------------------------------------------------- */
class Mesh
{
public:
    // Constructs the mesh.
    Mesh(VkDevice device,
         VkPhysicalDevice physicalDevice,
         const uint32_t bindingNumber);

    // Returns the binding description. This is valid only after
    // writing vertices into mesh.
    VkVertexInputBindingDescription bindingDescription();
    // Returns the attribute descriptions. This is valid only after
    // writing vertices into mesh.
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions() const;

    // Creates the mesh buffer.
    void create(const std::vector<Vertex>& vertices);
    // Destroys the mesh buffer.
    void destroy();

    // Draws the mesh.
    void draw(VkCommandBuffer commandBuffer);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
