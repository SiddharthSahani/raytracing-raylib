
#pragma once

#include "src/structs/objects.h"
#include "src/structs/material.h"
#include <vector>


namespace rt {


struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Plane> planes;
    std::vector<Material> materials;
    Color backgroundColor;
};


} // namespace rt
