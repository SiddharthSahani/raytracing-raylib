
#include "src/compiledscene.h"
#include <algorithm>


namespace rt {


CompiledScene::CompiledScene(const std::string& sceneName, const Scene& scene,
                             Vector2 packedMatTexSize)
    : m_sceneName(sceneName) {

    // normalizing background color
    m_backgroundColor = {
        scene.backgroundColor.r / 255.0f,
        scene.backgroundColor.g / 255.0f,
        scene.backgroundColor.b / 255.0f,
    };

    // vec to hold all the unique mats
    std::vector<const Material*> materials;

    // gets the mat's index from the vec
    // if vec doesnt have mat then pushes the mat onto the vec
    auto findMat = [&](const Material* mat) {
        auto it = std::find(materials.begin(), materials.end(), mat);
        if (it == materials.end()) {
            materials.push_back(mat);
            return (int)(materials.size() - 1);
        }
        return (int)(it - materials.begin());
    };

    // converts and sets the mat index of the spheres
    for (const Sphere& obj : scene.spheres) {
        internal::Sphere iObj = obj.convert();
        iObj.materialIndex = findMat(obj.material.get());
        m_spheres.push_back(iObj);
    }

    // converts and sets the mat index of the triangles
    for (const Triangle& obj : scene.triangles) {
        internal::Triangle iObj = obj.convert();
        iObj.materialIndex = findMat(obj.material.get());
        m_triangles.push_back(iObj);
    }

    // creating the material data
    m_materialData = new PackedMaterialData(materials.size(), packedMatTexSize);
    for (int i = 0; i < materials.size(); i++) {
        m_materialData->setMaterial(i, *materials[i]);
    }
}


CompiledScene::~CompiledScene() { delete m_materialData; }


} // namespace rt
