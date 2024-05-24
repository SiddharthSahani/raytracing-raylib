
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


struct Scene {
    Sphere spheres[MAX_SPHERES];
    vec3 backgroundColor;
    int numSpheres;
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
uniform Scene scene;


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

    if (t > 0.0 && t < record.hitDistance) {
        record.worldPosition = ray.origin + ray.direction * t;
        record.worldNormal = normalize(record.worldPosition - sphere.position);
        record.hitDistance = t;
        return true;
    }

    return false;
}


// Single ray path
HitRecord traceRay(Ray ray) {
    HitRecord record;
    record.objectIndex = -1;
    record.hitDistance = FLT_MAX;

    for (int i = 0; i < scene.numSpheres; i++) {
        bool closer = hitSphere(scene.spheres[i], ray, record);
        if (closer) {
            record.objectIndex = i;
        }
    }

    return record;
}


// Traces the pixel ray (bounces included)
vec3 getPixelColor() {
    Ray ray = genRay();
    vec3 light = vec3(0.0, 0.0, 0.0);
    vec3 contribution = vec3(1.0, 1.0, 1.0);

    for (float i = 0; i < 5; i++) {
        HitRecord record = traceRay(ray);

        if (record.hitDistance == FLT_MAX) {
            light += scene.backgroundColor * contribution;
            break;
        }

        contribution *= scene.spheres[record.objectIndex].color;

        ray.origin = record.worldPosition + record.worldNormal * 0.001;
        ray.direction = reflect(ray.direction, record.worldNormal);
    }

    return light;
}


void main() {
    vec3 color = getPixelColor();
    gl_FragColor = vec4(color, 1.0);
}
