/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::AtmosphereRenderer class
 * -------------------------------------------------------------------------- */

#pragma once

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

struct TextureCube;

class AtmosphereRenderer
{
public:
    struct Params
    {
        glm::vec4 viewport;
        glm::mat4 inv_proj;
        glm::mat4 inv_view_rot;
        glm::vec4 lightdir = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
        glm::vec4 Kr = glm::vec4(0.18867780436772762,
                                 0.4978442963618773,
                                 0.6616065586417131,
                                 0.0f);
        float rayleighBrightness      = 0.8f;
        float mieBrightness           = 0.1f;
        float spotBrightness          = 1000.0f;
        float scatterStrength         = 0.5f;
        float rayleighStrength        = 0.8f;
        float mieStrength             = 0.1f;
        float rayleighCollectionPower = 0.1f;
        float mieCollectionPower      = 0.39f;
        float mieDistribution         = 0.63f;
    };

    AtmosphereRenderer(
        const VkPhysicalDevice& physicalDevice,
        const VkDevice& device,
        const uint32_t& graphicsQueueFamilyIndex);

    void render();

    std::shared_ptr<TextureCube> textureCube() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
