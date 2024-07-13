
#pragma once

#include "src/packedmaterialdata.h"
#include "src/scene.h"
#include "src/structs/objects.h"
#include <string>


class Renderer;


namespace rt {


class CompiledScene {

public:
    CompiledScene(const std::string& name, const Scene& scene, Vector2 packedMatTexSize);
    ~CompiledScene();
    const std::string& getname() const { return m_name; }

private:
    std::string m_name;
    Vector3 m_backgroundColor;
    std::vector<internal::Sphere> m_spheres;
    std::vector<internal::Triangle> m_triangles;
    PackedMaterialData* m_materialData = nullptr;

    friend class ::Renderer;
};


} // namespace rt
