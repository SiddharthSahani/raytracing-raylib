
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


struct Plane {

    // 16 bytes
    Vector3 center;
    int materialIndex;
    // 16 bytes
    Vector3 uDirection;
    float uSize;
    // 16 bytes
    Vector3 vDirection;
    float vSize;

    Plane(Vector3 _center, Vector3 _uDir, Vector3 _vDir, Vector2 _size, int _materialIndex) {
        center = _center;
        uDirection = Vector3Normalize(_uDir);
        vDirection = Vector3Normalize(_vDir);
        uSize = _size.x;
        vSize = _size.y;
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
    int materialIndex;

    Triangle(Vector3 _v0, Vector3 _v1, Vector3 _v2, int _materialIndex) {
        v0 = _v0;
        v1 = _v1;
        v2 = _v2;
        materialIndex = _materialIndex;
    }
};


} // namespace rt
