
#include "src/hittable.h"


namespace rt {


internal::Sphere Sphere::convert() const {
    internal::Sphere out;

    out.position = position;
    out.radius = radius;

    return out;
}


internal::Triangle Triangle::convert() const {
    internal::Triangle out;

    out.v0 = v0;
    out.v1 = v1;
    out.v2 = v2;
    out.uv0 = uv0;
    out.uv1 = uv1;
    out.uv2 = uv2;

    return out;
}


} // namespace rt
