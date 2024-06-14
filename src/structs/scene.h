
#pragma once

#include "src/structs/material.h"
#include "src/structs/objects.h"
#include <vector>


namespace rt {


struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Plane> planes;
    std::vector<Triangle> triangles;
    std::vector<Material> materials;
    Color backgroundColor;

    void addMaterial(Material mat) { materials.push_back(mat); }

    void addObject(Sphere obj) { spheres.push_back(obj); }

    void addObject(Triangle obj) { triangles.push_back(obj); }

    void addObject(Plane obj) { planes.push_back(obj); }
};


} // namespace rt
