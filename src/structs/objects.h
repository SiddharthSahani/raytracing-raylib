
#pragma once

#include <raylib/raylib.h>
#include <raylib/raymath.h> // Vector3Normalize


namespace rt {


struct Sphere {
    Vector3 position;

    float radius;

    Vector3 color;
    float _padding_1;

    Sphere(Vector3 _position, float _radius, Color _color) {
        position = _position;
        radius = _radius;
        color = {_color.r / 255.0f, _color.g / 255.0f, _color.b / 255.0f};
    }
};


struct Plane {
    Vector3 center;
    float _padding_1;

    Vector3 uDirection;

    float uSize;

    Vector3 vDirection;

    float vSize;

    Vector3 color;
    float _padding_2;

    Plane(Vector3 _center, Vector3 _uDir, Vector3 _vDir, Vector2 _size, Color _color) {
        center = _center;
        uDirection = Vector3Normalize(_uDir);
        vDirection = Vector3Normalize(_vDir);
        uSize = _size.x;
        vSize = _size.y;
        color = {_color.r / 255.0f, _color.g / 255.0f, _color.b / 255.0f};
    }
};


} // namespace rt
