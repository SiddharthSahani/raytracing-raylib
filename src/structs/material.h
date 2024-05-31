
#pragma once

#include <raylib/raylib.h>


namespace rt {


struct Material {

    // 16 bytes
    Vector3 albedo;
    float roughness;

    Material(Color _albedo, float _roughness) {
        albedo = {_albedo.r / 255.0f, _albedo.g / 255.0f, _albedo.b / 255.0f};
        roughness = _roughness;
    }
};


} // namespace rt
