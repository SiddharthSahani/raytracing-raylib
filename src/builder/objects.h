
#pragma once

#include "src/builder/utils.h"


struct Sphere {
    Vector3 position;
    float radius;
    Color color;
};


/*
Sphere struct defined in shader
struct Sphere {
    vec3 position; // 12 -> 16
    float radius;  //  4 -> 16
    vec3 color;    // 12 -> 16
};

48 bytes in total
*/


int pass(const Sphere& sphere, void* dst) {
    int offset = 0;
    offset += pass(sphere.position, dst + offset);
    offset += pass(sphere.radius, dst + offset, false);
    offset += pass(sphere.color, dst + offset);
    return offset;
}
