
#pragma once

#include "src/packedmaterialdata.h"
#include "src/scene.h"
#include "src/structs/objects.h"
#include <string>


// forward declaration
class Raytracer;


namespace rt {


class CompiledScene {

public:
    CompiledScene(const Scene& scene, Vector2 packedMatTexSize);
    ~CompiledScene();
    unsigned getId() const { return m_id; }

private:
    unsigned m_id;
    Vector3 m_backgroundColor;
    std::vector<internal::Sphere> m_spheres;
    std::vector<internal::Triangle> m_triangles;
    PackedMaterialData* m_materialData = nullptr;

    friend class ::Raytracer;
};


} // namespace rt
