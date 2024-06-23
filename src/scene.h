
#pragma once

#include "src/packedmaterialdata.h"
#include "src/structs/objects.h"
#include <vector>


namespace rt {


struct Scene {

    // `Scene` owns the pointer now
    Scene(PackedMaterialData* materials);
    ~Scene();

    void addObject(const Sphere& obj);
    void addObject(const Triangle& obj);

    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;
    Color backgroundColor;
    PackedMaterialData* materials = nullptr;
};


} // namespace rt
