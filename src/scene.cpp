
#include "src/scene.h"


namespace rt {


Scene::Scene(CombinedMaterial* _materials) : materials(_materials) {}


Scene::~Scene() {
    delete materials;
}


void Scene::addObject(const Sphere& obj) {
    spheres.push_back(obj);
}


void Scene::addObject(const Triangle& obj) {
    triangles.push_back(obj);
}


} // namespace rt
