
#pragma once

#include "src/structs/scene.h"
#include "src/builder/objects.h"


/*
Scene struct defined in shader
struct Scene {
    Sphere spheres[MAX_SPHERE_COUNT]; // 48 * MAX_SPHERE_COUNT
    vec3 backgroundColor;             // 12 -> 16
    int numSpheres;                   //  4 -> 16
};

48 * MAX_SPHERE_COUNT + 32 bytes in total

for MAX_SPHERE_COUNT = 32, size of struct is 1568 bytes
*/


int pass(const rt::Scene& scene, void* dst, int maxSphereCount) {
    if (scene.spheres.size() > maxSphereCount) {
        return 0;
    }

    int offset = 0;

    for (const rt::Sphere& sphere: scene.spheres) {
        offset += pass(sphere, dst + offset);
    }
    for (int i = scene.spheres.size(); i < maxSphereCount; i++) {
        offset += pass(rt::Sphere{}, dst + offset);
    }
    offset += pass(scene.backgroundColor, dst + offset);
    offset += pass((int) scene.spheres.size(), dst + offset, false);

    return offset;
}
