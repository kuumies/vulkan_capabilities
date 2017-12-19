/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::MeshManager class
 * -------------------------------------------------------------------------- */

#include "vk_mesh_manager.h"

/* -------------------------------------------------------------------------- */

#include <map>
#include "../../common/mesh.h"
#include "../vk_mesh.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct MeshManager::Impl
{
    Impl(const VkPhysicalDevice& physicalDevice,
         const VkDevice& device,
         const uint32_t& queueFamilyIndex)
        : physicalDevice(physicalDevice)
        , device(device)
        ,  queueFamilyIndex(queueFamilyIndex)
    {}

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t queueFamilyIndex;

    std::map<std::shared_ptr<kuu::Mesh>, std::shared_ptr<vk::Mesh>> meshes;
};

/* -------------------------------------------------------------------------- */

MeshManager::MeshManager(
    const VkPhysicalDevice& physicalDevice,
    const VkDevice& device,
    const uint32_t& queueFamilyIndex)
    : impl(std::make_shared<Impl>(physicalDevice, device, queueFamilyIndex))
{}

/* -------------------------------------------------------------------------- */

void MeshManager::addPbrMesh(std::shared_ptr<kuu::Mesh> m)
{
    std::vector<float> vertexVector;
    for (const Vertex& v : m->vertices)
    {
        vertexVector.push_back(v.pos.x);
        vertexVector.push_back(v.pos.y);
        vertexVector.push_back(v.pos.z);
        vertexVector.push_back(v.texCoord.x);
        vertexVector.push_back(v.texCoord.y);
        vertexVector.push_back(v.normal.x);
        vertexVector.push_back(v.normal.y);
        vertexVector.push_back(v.normal.z);
        vertexVector.push_back(v.tangent.x);
        vertexVector.push_back(v.tangent.y);
        vertexVector.push_back(v.tangent.z);
        vertexVector.push_back(v.bitangent.x);
        vertexVector.push_back(v.bitangent.y);
        vertexVector.push_back(v.bitangent.z);
    }

    std::shared_ptr<vk::Mesh> vkMesh = std::make_shared<vk::Mesh>(impl->physicalDevice, impl->device);
    vkMesh->setVertices(vertexVector);
    vkMesh->setIndices(m->indices);
    vkMesh->addVertexAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
    vkMesh->addVertexAttributeDescription(1, 0, VK_FORMAT_R32G32_SFLOAT,    3 * sizeof(float));
    vkMesh->addVertexAttributeDescription(2, 0, VK_FORMAT_R32G32B32_SFLOAT, 5 * sizeof(float));
    vkMesh->addVertexAttributeDescription(3, 0, VK_FORMAT_R32G32B32_SFLOAT, 8 * sizeof(float));
    vkMesh->addVertexAttributeDescription(4, 0, VK_FORMAT_R32G32B32_SFLOAT, 11 * sizeof(float));
    vkMesh->setVertexBindingDescription(0, 14 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX);
    vkMesh->create();

    impl->meshes[m] = vkMesh;
}

/* -------------------------------------------------------------------------- */

std::shared_ptr<vk::Mesh> MeshManager::mesh(std::shared_ptr<kuu::Mesh> mesh) const
{
    if (!impl->meshes.count(mesh))
        return nullptr;
    return impl->meshes.at(mesh);
}

/* -------------------------------------------------------------------------- */

std::vector<std::shared_ptr<Mesh> > MeshManager::meshes() const
{
    std::vector<std::shared_ptr<vk::Mesh>> meshes;
    for(auto m: impl->meshes)
        meshes.push_back(m.second);
    return meshes;
}

} // namespace vk
} // namespace kuu
