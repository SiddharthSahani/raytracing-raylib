
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


} // namespace rt
