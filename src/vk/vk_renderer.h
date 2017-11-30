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
    Renderer(const VkInstance& instance,
             const VkPhysicalDevice& physicalDevice,
             const VkSurfaceKHR& surface,
             const VkExtent2D& extent);

    // Creates and destroys the renderer.
    bool create();
    void destroy();

    // Returns true if the renderer is created and all Vulkan objects.
    // were created without issues.
    bool isValid() const;

    // Surface size has changed.
    bool resized(const VkExtent2D& extent);

    // Renders a frame
    bool renderFrame();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
