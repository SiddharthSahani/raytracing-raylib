
#include "src/compiledscene.h"
#include "src/logger.h"
#include <algorithm>
#include <map>


namespace rt {


CompiledScene::CompiledScene(const std::string& name, const Scene& scene, Vector2 packedMatTexSize)
    : m_name(name) {
    INFO("Compiling scene: '%s'", name.c_str());

    // normalizing background color
    m_backgroundColor = {
        scene.backgroundColor.r / 255.0f,
        scene.backgroundColor.g / 255.0f,
        scene.backgroundColor.b / 255.0f,
    };
    TRACE("    BackgroundColor = (%f %f %f)", m_backgroundColor.x, m_backgroundColor.y,
          m_backgroundColor.z);

    // vec to hold all the unique mats
    std::vector<const Material*> materials;
    // map to store how many times a material is being used in scene
    std::map<int, int> materialCounter;

    // gets the mat's index from the vec
    // if vec doesnt have mat then pushes the mat onto the vec
    auto findMat = [&](const Material* mat) {
        auto it = std::find(materials.begin(), materials.end(), mat);
        if (it == materials.end()) {
            materials.push_back(mat);
            int index = (materials.size() - 1);
            materialCounter[index] = 0;
            return index;
        }
        return (int)(it - materials.begin());
    };

    // converts and sets the mat index of the spheres
    for (const Sphere& obj : scene.spheres) {
        internal::Sphere iObj = obj.convert();
        iObj.materialIndex = findMat(obj.material.get());
        m_spheres.push_back(iObj);

        materialCounter[(int)iObj.materialIndex] += 1;
    }

    // converts and sets the mat index of the triangles
    for (const Triangle& obj : scene.triangles) {
        internal::Triangle iObj = obj.convert();
        iObj.materialIndex = findMat(obj.material.get());
        m_triangles.push_back(iObj);

        materialCounter[(int)iObj.materialIndex] += 1;
    }

    for (auto pair : materialCounter) {
        TRACE("    '%s' is referenced by %d objects", materials[pair.first]->getName().c_str(),
              pair.second);
    }

    INFO("    Scene has %d unique materials", materials.size());
    INFO("    Scene has %d spheres", m_spheres.size());
    INFO("    Scene has %d triangles", m_triangles.size());

    // creating the material data
    std::string packedMaterialName = TextFormat("'%s's packedMaterialData", m_name.c_str());
    m_materialData = new PackedMaterialData(packedMaterialName, materials.size(), packedMatTexSize);
    for (int i = 0; i < materials.size(); i++) {
        m_materialData->setMaterial(i, *materials[i]);
    }
}


CompiledScene::~CompiledScene() {
    TRACE("Unloading scene: '%s'", m_name.c_str());
    delete m_materialData;
}


} // namespace rt
