
#pragma once

#include "src/hittable.h"
#include <vector>


namespace rt {


struct Scene {

    void addObject(const Sphere& obj) { spheres.push_back(obj); }
    void addObject(const Triangle& obj) { triangles.push_back(obj); }

    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;
    Color backgroundColor;

};


} // namespace rt
