
#pragma once

#include "src/structs/objects.h"
#include "src/combinedmaterials.h"
#include <vector>


namespace rt {


struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;
    rt::CombinedMaterial* material;
    Color backgroundColor;

    void addObject(Sphere obj) { spheres.push_back(obj); }

    void addObject(Triangle obj) { triangles.push_back(obj); }
};


} // namespace rt
