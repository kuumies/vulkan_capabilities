/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Mesh struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace kuu
{

/* -------------------------------------------------------------------------- *
   A vertex.
 * -------------------------------------------------------------------------- */

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

/* -------------------------------------------------------------------------- *
   A mesh.
 * -------------------------------------------------------------------------- */
struct Mesh
{
    void addVertex(const Vertex& v);
    void addTriangle(
            const Vertex& a,
            const Vertex& b,
            const Vertex& c);
    void addQuad(
            const Vertex& a,
            const Vertex& b,
            const Vertex& c,
            const Vertex& d);
    void generateTangents();

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

std::shared_ptr<Mesh> createBox(float width, float height, float depth);

} // namespace kuu
