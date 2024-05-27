
#pragma once

#include <raylib/raylib.h>


int pass(const float& value, void* dst, bool singular = true) {
    if (singular) {
        *((float*)dst) = value;
        return 4;
    }

    int offset = 0;

    offset += pass(value, dst + offset);
    offset += pass(0.0f, dst + offset);
    offset += pass(0.0f, dst + offset);
    offset += pass(0.0f, dst + offset);

    return offset;
}


int pass(const int& value, void* dst, bool singular = true) {
    if (singular) {
        *((int*)dst) = value;
        return 4;
    }

    int offset = 0;

    offset += pass(value, dst + offset);
    offset += pass(0.0f, dst + offset);
    offset += pass(0.0f, dst + offset);
    offset += pass(0.0f, dst + offset);

    return offset;
}


int pass(const Vector3& vec, void* dst) {
    int offset = 0;

    offset += pass(vec.x, dst + offset);
    offset += pass(vec.y, dst + offset);
    offset += pass(vec.z, dst + offset);
    offset += pass(0.0f, dst + offset);

    return offset;
}


int pass(const Color& color, void* dst) {
    Vector3 vec = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f};
    return pass(vec, dst);
}
