/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Scene struct.
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <vector>
#include "camera.h"
#include "light.h"
#include "model.h"

namespace kuu
{

/* -------------------------------------------------------------------------- *
   A scene to render.
 * -------------------------------------------------------------------------- */
struct Scene
{
    std::string name;
    Camera camera;
    Light light;
    std::vector<Model> models;
};

} // namespace kuu
