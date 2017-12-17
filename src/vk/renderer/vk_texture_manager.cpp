/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::TextureManager class
 * -------------------------------------------------------------------------- */

#include "vk_texture_manager.h"

/* -------------------------------------------------------------------------- */

#include <map>

/* -------------------------------------------------------------------------- */

namespace kuu
{
namespace vk
{

struct TextureManager::Impl
{
    std::map<std::string, std::shared_ptr<Texture2D>> textureMap;
};

/* -------------------------------------------------------------------------- */

TextureManager::TextureManager()
{

}
/* -------------------------------------------------------------------------- */

void TextureManager::addTexture2D(
        const std::string& filepath,
        std::shared_ptr<Texture2D> texture)
{
    impl->textureMap[filepath] = texture;
}

/* -------------------------------------------------------------------------- */

std::shared_ptr<Texture2D>
    TextureManager::texture2D(
        const std::string& filepath) const
{
    return impl->textureMap.at(filepath);
}

} // namespace vk
} // namespace kuu
