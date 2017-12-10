/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Material struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <glm/vec3.hpp>
#include <string>

namespace kuu
{

/* -------------------------------------------------------------------------- *
   A material
 * -------------------------------------------------------------------------- */
struct Material
{
    enum class Type
    {
        Diffuse,
        Pbr,
        Skybox
    };

    struct Diffuse
    {
        std::string map;
    } diffuse;

    struct Pbr
    {
        std::string ambientOcclusion;
        std::string baseColor;
        std::string height;
        std::string metallic;
        std::string normal;
        std::string roughness;
    } pbr;

    Type type = Type::Diffuse;
};

} // namespace kuu
