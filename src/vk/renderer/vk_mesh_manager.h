/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::MeshManager class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <glm/vec3.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{

/* -------------------------------------------------------------------------- */

struct Mesh;

namespace vk
{

/* -------------------------------------------------------------------------- */

class Mesh;

/* -------------------------------------------------------------------------- */

class MeshManager
{
public:
    // Constructs the mesh manager.
    MeshManager(const VkPhysicalDevice& physicalDevice,
                const VkDevice& device,
                const uint32_t& queueFamilyIndex);

    // Adds a PBR mesh into manager.
    void addPbrMesh(std::shared_ptr<kuu::Mesh> mesh);
    // Returns a Vulkan mesh of mesh.
    std::shared_ptr<vk::Mesh> mesh(std::shared_ptr<kuu::Mesh> mesh) const;
    // Returns all Vulkan meshes.
    std::vector<std::shared_ptr<vk::Mesh>> meshes() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
