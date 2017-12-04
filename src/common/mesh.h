/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Mesh struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <vector>

namespace kuu
{

/* -------------------------------------------------------------------------- *
   A mesh.
 * -------------------------------------------------------------------------- */
struct Mesh
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

} // namespace kuu
