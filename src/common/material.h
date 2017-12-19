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
        std::string ambientOcclusionMap;
        std::string baseColorMap;
        std::string heightMap;
        std::string metallicMap;
        std::string normalMap;
        std::string roughnessMap;

        glm::vec3  albedo;
        float metallic = 0.0f;
        float roughness = 0.0f;
        float ao = 1.0f;

    } pbr;

    Type type = Type::Pbr;
};

} // namespace kuu
