
#pragma once

#include <raylib/raylib.h>


namespace rt {


struct Material {

    // 16 bytes
    Vector3 albedo;
    float roughness;
    // 16 bytes
    float emissionPower;
    float _padding_1[3];

    Material(Color _albedo, float _roughness, float _emPower) {
        albedo = {_albedo.r / 255.0f, _albedo.g / 255.0f, _albedo.b / 255.0f};
        roughness = _roughness;
        emissionPower = _emPower;
    }
};


} // namespace rt
