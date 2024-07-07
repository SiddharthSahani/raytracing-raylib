
#pragma once

#include "src/material.h"
#include "src/structs/objects.h"
#include <memory>
#include <raylib/raylib.h>


namespace rt {


struct Sphere {
    Vector3 position;
    float radius;
    std::shared_ptr<Material> material;

    internal::Sphere convert() const;
};


struct Triangle {
    Vector3 v0;
    Vector3 v1;
    Vector3 v2;
    Vector2 uv0 = {0, 0};
    Vector2 uv1 = {0, 1};
    Vector2 uv2 = {1, 0};
    std::shared_ptr<Material> material;

    internal::Triangle convert() const;
};


} // namespace rt
