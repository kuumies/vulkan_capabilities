/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::Model struct.
 * -------------------------------------------------------------------------- */

#include "model.h"

namespace kuu
{

Model::Model()
    : mesh(std::make_shared<Mesh>())
    , material(std::make_shared<Material>())
{

}

} // namespace kuu
