/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::SkyRenderer class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <glm/vec3.hpp>
#include <memory>
#include <vulkan/vulkan.h>

namespace kuu
{

/* -------------------------------------------------------------------------- */

struct Camera;
struct Scene;

namespace vk
{

/* -------------------------------------------------------------------------- */

struct TextureCube;

/* -------------------------------------------------------------------------- *
   A skybox renderer.
 * -------------------------------------------------------------------------- */
class SkyRenderer
{
public:
    // Constructs the sky renderer.
    SkyRenderer(const VkPhysicalDevice& physicalDevice,
                const VkDevice& device,
                const uint32_t queueFamilyIndex,
                const VkExtent2D& extent,
                const VkRenderPass& renderPass,
                std::shared_ptr<TextureCube> environment);

    // Sets and returns the scene to renderer.
    void setScene(std::shared_ptr<Scene> scene);
    std::shared_ptr<Scene> scene() const;

    // Viewport has been resized.
    void resized(const VkExtent2D& extent, const VkRenderPass& renderPass);

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
