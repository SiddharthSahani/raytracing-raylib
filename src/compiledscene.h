
#include "src/packedmaterialdata.h"
#include "src/scene.h"
#include "src/structs/objects.h"
#include <string>


class Renderer;


namespace rt {


class CompiledScene {

public:
    CompiledScene(const std::string& sceneName, const Scene& scene, Vector2 packedMatTexSize);
    ~CompiledScene();
    const std::string& getSceneName() const { return m_sceneName; }

private:
    std::string m_sceneName;
    Vector3 m_backgroundColor;
    std::vector<internal::Sphere> m_spheres;
    std::vector<internal::Triangle> m_triangles;
    PackedMaterialData* m_materialData = nullptr;

    friend class ::Renderer;
};


} // namespace rt
