
#pragma once

#include "src/structs/objects.h"
#include <vector>


struct Scene {
    std::vector<Sphere> spheres;
    Color backgroundColor;
};
