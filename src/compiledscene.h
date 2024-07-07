
#include "src/packedmaterialdata.h"
#include "src/scene.h"
#include "src/structs/objects.h"


class Renderer;


namespace rt {


class CompiledScene {

public:
    CompiledScene(const Scene& scene, Vector2 packedMatTexSize);
    ~CompiledScene();

private:
    Vector3 m_backgroundColor;
    std::vector<internal::Sphere> m_spheres;
    std::vector<internal::Triangle> m_triangles;
    PackedMaterialData* m_materialData = nullptr;

    friend class ::Renderer;
};


} // namespace rt
