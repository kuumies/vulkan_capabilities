/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Renderer class.
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
   A vulkan renderer scene struct.
 * -------------------------------------------------------------------------- */
struct RendererScene
{};

/* -------------------------------------------------------------------------- *
   A vulkan renderer class.
 * -------------------------------------------------------------------------- */
class Renderer
{
public:
    // Constructs the renderer.
    Renderer(const VkPhysicalDevice& physicalDevice,
             const VkSurfaceKHR& surface);

    // Creates and destroys the renderer.
    bool create();
    bool destroy();

    // Returns true if the renderer is created and all Vulkan objects.
    // were created without issues.
    bool isValid() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
