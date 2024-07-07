
#pragma once

#include <raylib/raylib.h>
#include <raylib/raymath.h> // Vector3Normalize


namespace rt::internal {


struct Sphere {
    // 16 bytes
    Vector3 position;
    float radius;
    // 16 bytes
    float materialIndex;
    float _padding_1[3];
};


struct Triangle {
    // 16 bytes
    Vector3 v0;
    float _padding_1;
    // 16 bytes
    Vector3 v1;
    float _padding_2;
    // 16 bytes
    Vector3 v2;
    float _padding_3;
    // 16 bytes
    Vector2 uv0;
    Vector2 uv1;
    // 16 bytes
    Vector2 uv2;
    float materialIndex;
    float _padding_4;
};


} // namespace rt::internal
