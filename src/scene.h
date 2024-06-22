
#pragma once

#include "src/structs/objects.h"
#include "src/combinedmaterials.h"
#include <vector>


namespace rt {


struct Scene {

    // `Scene` owns the pointer now
    Scene(CombinedMaterial* materials);
    ~Scene();

    void addObject(const Sphere& obj);
    void addObject(const Triangle& obj);

    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;
    Color backgroundColor;
    CombinedMaterial* materials = nullptr;

};


} // namespace rt
