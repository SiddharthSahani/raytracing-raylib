
#version 330 core

#define FLT_MAX 3.402823466e+38F
#define MAX_SPHERES 10


// STRUCT DEFINITIONS

struct Camera {
    mat4 invProjMat;
    mat4 invViewMat;
    vec3 position;
};


struct Sphere {
    vec3 position;
    float radius;
    vec3 color;
};


struct Ray {
    vec3 origin;
    vec3 direction;
};


struct HitRecord {
    vec3 worldPosition;
    vec3 worldNormal;
    float hitDistance;
    int objectIndex;
};


// UNIFORM VAIABLES

uniform vec2 uImageSize;
uniform Camera camera;
uniform Sphere spheres[MAX_SPHERES];
uniform int numSpheres;


// FUNCTIONS

// Initializes a Ray based on current fragCoord
Ray genRay() {
    vec2 coord = gl_FragCoord.xy / uImageSize * 2.0 - 1.0;
    vec4 target = camera.invProjMat * vec4(coord, 1.0, 1.0);
    vec4 rayDirection = camera.invViewMat * vec4(normalize(target.xyz / target.w), 0.0);

    return Ray(camera.position, rayDirection.xyz);
}


// Ray-Sphere intersection function
// Modifies HitRecord and returns true if current object is closer
bool hitSphere(Sphere sphere, Ray ray, out HitRecord record) {
    vec3 oc = ray.origin - sphere.position;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float d = b * b - 4 * a * c;

    if (d < 0.0f) {
        return false;
    }

    float t = (-b - sqrt(d)) / (2.0 * a);

    if (t < record.hitDistance) {
        record.worldPosition = ray.origin + ray.direction * t;
        record.worldNormal = normalize(record.worldPosition - sphere.position);
        record.hitDistance = t;
        return true;
    }

    return false;
}


void main() {
    Ray ray = genRay();

    HitRecord record;
    record.objectIndex = -1;
    record.hitDistance = FLT_MAX;

    for (int i = 0; i < numSpheres; i++) {
        bool closer = hitSphere(spheres[i], ray, record);
        if (closer) {
            record.objectIndex = i;
        }
    }

    if (record.objectIndex == -1) {
        gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);
    } else {
        const vec3 lightDirection = normalize(vec3(-1, -1, -1));
        float lightIntensity = dot(record.worldNormal, -lightDirection);
        gl_FragColor = vec4(spheres[record.objectIndex].color * lightIntensity, 1.0);
    }
}
