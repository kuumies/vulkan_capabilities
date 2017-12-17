/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::TextureManager class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct Texture2D;

/* -------------------------------------------------------------------------- *
   A texture manager.
 * -------------------------------------------------------------------------- */
class TextureManager
{
public:
    // Constructs the texture manager.
    TextureManager();

    // Adds a two-dimensional texture.
    void addTexture2D(const std::string& filepath,
                      std::shared_ptr<Texture2D> texture);

    // Returns a two-dimensional texture.
    std::shared_ptr<Texture2D> texture2D(const std::string& filepath) const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
