
#pragma once

#include "src/structs/objects.h"
#include <vector>


namespace rt {


struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Plane> planes;
    Color backgroundColor;
};


} // namespace rt
