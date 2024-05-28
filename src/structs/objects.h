
#pragma once

#include <raylib/raylib.h>


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


} // namespace rt
