
#pragma once

#include <raylib/raylib.h>


namespace rt {


struct Camera {
    Vector3 position;
    Matrix invViewMat;
    Matrix invProjMat;
};


} // namespace rt
