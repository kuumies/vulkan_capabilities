/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::Mesh struct.
 * -------------------------------------------------------------------------- */

#include "mesh.h"
#include <glm/geometric.hpp>

namespace kuu
{

void Mesh::addVertex(const Vertex &v)
{
    indices.push_back(vertices.size());
    vertices.push_back(v);
}

void Mesh::addTriangle(const Vertex &a, const Vertex &b, const Vertex &c)
{
    addVertex(a);
    addVertex(b);
    addVertex(c);
}

void Mesh::addQuad(const Vertex &a, const Vertex &b, const Vertex &c, const Vertex &d)
{
    addTriangle(a, d, c);
    addTriangle(c, b, a);
}

void Mesh::generateTangents()
{
    bool triangles = (indices.size() % 3) == 0;
    if (!triangles)
        return;

    for (int i = 0; i < indices.size(); i += 3)
    {
        Vertex& v1 = vertices[indices[i+0]];
        Vertex& v2 = vertices[indices[i+1]];
        Vertex& v3 = vertices[indices[i+2]];

        glm::vec3 edge1 = v2.pos - v1.pos;
        glm::vec3 edge2 = v3.pos - v1.pos;
        glm::vec2 deltaUV1 = v2.texCoord - v1.texCoord;
        glm::vec2 deltaUV2 = v3.texCoord - v1.texCoord;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);

        glm::vec3 bitangent;
        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent = glm::normalize(bitangent);

        v1.tangent = tangent;
        v2.tangent = tangent;
        v3.tangent = tangent;

        v1.bitangent = bitangent;
        v2.bitangent = bitangent;
        v3.bitangent = bitangent;
    }
}

} // namespace kuu
