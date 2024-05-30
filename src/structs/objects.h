
#pragma once

#include <raylib/raylib.h>
#include <raylib/raymath.h> // Vector3Normalize


namespace rt {


struct Sphere {

    // 16 bytes
    Vector3 position;
    float radius;
    // 16 bytes
    Vector3 color;
    float roughness;

    Sphere(Vector3 _position, float _radius, Color _color, float _roughness) {
        position = _position;
        radius = _radius;
        color = {_color.r / 255.0f, _color.g / 255.0f, _color.b / 255.0f};
        roughness = _roughness;
    }
};


struct Plane {

    // 16 bytes
    Vector3 center;
    float _padding_1;
    // 16 bytes
    Vector3 uDirection;
    float uSize;
    // 16 bytes
    Vector3 vDirection;
    float vSize;
    // 16 bytes
    Vector3 color;
    float roughness;

    Plane(Vector3 _center, Vector3 _uDir, Vector3 _vDir, Vector2 _size, Color _color,
          float _roughness) {
        center = _center;
        uDirection = Vector3Normalize(_uDir);
        vDirection = Vector3Normalize(_vDir);
        uSize = _size.x;
        vSize = _size.y;
        color = {_color.r / 255.0f, _color.g / 255.0f, _color.b / 255.0f};
        roughness = _roughness;
    }
};


} // namespace rt
