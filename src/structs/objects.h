
#pragma once

#include <raylib/raylib.h>
#include <raylib/raymath.h> // Vector3Normalize


namespace rt {


struct Sphere {

    // 16 bytes
    Vector3 position;
    float radius;
    // 16 bytes
    int materialIndex;
    float _padding_1[3];

    Sphere(Vector3 _position, float _radius, int _materialIndex) {
        position = _position;
        radius = _radius;
        materialIndex = _materialIndex;
    }
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
    int materialIndex;
    float _padding_4;

    Triangle(Vector3 _v0, Vector3 _v1, Vector3 _v2, int _materialIndex, Vector2 _uv0 = {0, 0},
             Vector2 _uv1 = {0, 1}, Vector2 _uv2 = {1, 0}) {
        v0 = _v0;
        v1 = _v1;
        v2 = _v2;
        materialIndex = _materialIndex;

        uv0 = _uv0;
        uv1 = _uv1;
        uv2 = _uv2;
    }
};


} // namespace rt
