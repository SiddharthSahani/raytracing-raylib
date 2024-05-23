
#version 330 core

#define FLT_MAX 3.402823466e+38F
#define MAX_SPHERES 10


struct Camera {
    mat4 invProjMat;
    mat4 invViewMat;
    vec3 position;
};


struct Sphere {
    vec3 position;
    float radius;
};


uniform vec2 uImageSize;
uniform Camera camera;
uniform Sphere spheres[MAX_SPHERES];
uniform int numSpheres;


struct Ray {
    vec3 origin;
    vec3 direction;
};


struct HitRecord {
    vec3 worldPosition;
    vec3 worldNormal;
    float hitDistance;
};


Ray genRay() {
    vec2 coord = gl_FragCoord.xy / uImageSize * 2.0 - 1.0;
    vec4 target = camera.invProjMat * vec4(coord, 1.0, 1.0);
    vec4 rayDirection = camera.invViewMat * vec4(normalize(target.xyz / target.w), 0.0);

    return Ray(camera.position, rayDirection.xyz);
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

    HitRecord record;
    record.hitDistance = FLT_MAX;

    for (int i = 0; i < numSpheres; i++) {
        hitSphere(spheres[i], ray, record);
    }

    if (record.hitDistance == FLT_MAX) {
        gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);
    } else {
        gl_FragColor = vec4(record.worldNormal, 1.0);
    }
}
