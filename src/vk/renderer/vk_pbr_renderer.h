/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::PbrRenderer class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <glm/vec3.hpp>
#include <memory>
#include <vulkan/vulkan.h>

namespace kuu
{

/* -------------------------------------------------------------------------- */

struct Scene;

namespace vk
{

/* -------------------------------------------------------------------------- */

class MeshManager;
struct Texture2D;
struct TextureCube;

/* -------------------------------------------------------------------------- *
   A physical-based rendering renderer.
 * -------------------------------------------------------------------------- */
class PbrRenderer
{
public:
    // Constructs the PBR renderer.
    PbrRenderer(const VkPhysicalDevice& physicalDevice,
                const VkDevice& device,
                const uint32_t queueFamilyIndex,
                const VkExtent2D& extent,
                const VkRenderPass& renderPass,
                std::shared_ptr<TextureCube> environment,
                std::shared_ptr<MeshManager> meshManager);

    // Viewport has been resized.
    void resized(const VkExtent2D& extent, const VkRenderPass& renderPass);

    // Sets and returns the scene to renderer.
    void setScene(std::shared_ptr<Scene> scene);
    std::shared_ptr<Scene> scene() const;

    // Sets the shadow map
    void setShadowMap(std::shared_ptr<Texture2D> shadowMap);

    // Records commands to render the added models with PBR renderer.
    void recordCommands(const VkCommandBuffer& cmdBuf);

    // Updates uniform buffers. This needs to be called everytime camera
    // matrices in the scene changes.
    void updateUniformBuffers();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
