
#version 330 core

#define FLT_MAX 3.402823466e+38F


uniform vec2 uImageSize;
uniform mat4 uInvViewMat;
uniform mat4 uInvProjMat;
uniform vec3 uCameraPos;


struct Ray {
    vec3 origin;
    vec3 direction;
};


struct Sphere {
    vec3 position;
    float radius;
};


struct HitRecord {
    vec3 worldPosition;
    vec3 worldNormal;
    float hitDistance;
};


Ray genRay() {
    vec2 coord = gl_FragCoord.xy / uImageSize * 2.0 - 1.0;
    vec4 target = uInvProjMat * vec4(coord, 1.0, 1.0);
    vec4 rayDirection = uInvViewMat * vec4(normalize(target.xyz / target.w), 0.0);

    return Ray(uCameraPos, rayDirection.xyz);
}


void hitSphere(Sphere sphere, Ray ray, out HitRecord record) {
    vec3 oc = ray.origin - sphere.position;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float d = b * b - 4 * a * c;

    if (d < 0.0f) {
        return;
    }

    float t = (-b - sqrt(d)) / (2.0 * a);

    if (t < record.hitDistance) {
        record.worldPosition = ray.origin + ray.direction * t;
        record.worldNormal = normalize(record.worldPosition - sphere.position);
        record.hitDistance = t;
    }
}


void main() {
    Ray ray = genRay();
    Sphere spheres[] = {
        Sphere(vec3(0.0, 0.0, 0.0), 1.0),
        Sphere(vec3(0.0, -4.0, 0.0), 3.0)
    };

    HitRecord record;
    record.hitDistance = FLT_MAX;

    for (int i = 0; i < 2; i++) {
        hitSphere(spheres[i], ray, record);
    }

    if (record.hitDistance == FLT_MAX) {
        gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);
    } else {
        gl_FragColor = vec4(record.worldNormal, 1.0);
    }
}
