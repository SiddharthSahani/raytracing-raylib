
#pragma once

#include <raylib/raylib.h>


namespace rt {


struct Camera {
    Vector3 position;
    Vector3 direction;
    float fov;
};


} // namespace rt
