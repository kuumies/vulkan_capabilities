/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::ShadowMapRenderer class
 * -------------------------------------------------------------------------- */

#pragma once

#include <glm/vec3.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{

struct Scene;

namespace vk
{

class MeshManager;
struct Texture2D;

class ShadowMapRenderer
{
public:
    ShadowMapRenderer(
        const VkPhysicalDevice& physicalDevice,
        const VkDevice& device,
        const uint32_t& graphicsQueueFamilyIndex,
        std::shared_ptr<MeshManager> meshManager);

    void setScene(std::shared_ptr<Scene> scene);
    void render();

    std::shared_ptr<Texture2D> texture() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
