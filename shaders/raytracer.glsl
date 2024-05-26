
#version 430 core
layout (local_size_x = WG_SIZE_PLACEHOLDER, local_size_y = WG_SIZE_PLACEHOLDER, local_size_z = 1) in;

#define FLT_MAX 3.402823466e+38F
#define MAX_SPHERES 32


// ----- STRUCT DEFINITIONS -----

struct Camera {
    vec3 position;
    vec3 direction;
};


struct Ray {
    vec3 origin;
    vec3 direction;
};


struct Sphere {
    vec3 position;
    float radius;
    vec3 color;
};


struct HitRecord {
    vec3 worldPosition;
    vec3 worldNormal;
    float hitDistance;
    int objectIndex;
};


struct Scene {
    Sphere spheres[MAX_SPHERES];
    vec3 backgroundColor;
    int numSpheres;
};


struct Config {
    float bounceLimit;
    float numSamples;
};


// ----- UNIFORMS AND BUFFERS -----

layout (rgba32f, binding = 0) uniform image2D outputImage;
uniform Camera camera;
uniform Scene scene;
uniform Config config;


// ----- RNG FUNCTIONS -----

// PCG www.shadertoy.com/view/XlGcRh
uint nextRandom(inout uint state) {
    state = state * 747796405u + 2891336453u;
    uint result = ((state >> ((state >> 28) + 4u)) ^ state) * 277803737u;
    result = (result >> 22) ^ result;
    return result;
}


float randomValue(inout uint state) {
    return nextRandom(state) / 4294967295.0;
}


vec3 randomDirection(inout uint state) {
    return normalize(vec3(
        randomValue(state),
        randomValue(state),
        randomValue(state)
    ) * 2 - 1);
}


// ----- INTERSECTION FUNCTIONS -----

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


// ----- MAIN FUNCTIONS -----

Ray genRay() {
    vec2 imgSize = imageSize(outputImage);
    vec2 coefficients = (gl_GlobalInvocationID.xy * 2.0 - imgSize) / imgSize.x;

    vec3 upDirection = vec3(0.0, 1.0, 0.0);
    vec3 rightDirection = cross(camera.direction, upDirection);

    Ray ray;
    ray.origin = camera.position;
    ray.direction = camera.direction + coefficients.x * rightDirection - coefficients.y * upDirection;
    return ray;
}


HitRecord traceRay(Ray ray) {
    HitRecord record;
    record.hitDistance = FLT_MAX;

    for (int i = 0; i < scene.numSpheres; i++) {
        bool closer = hitSphere(scene.spheres[i], ray, record);
        if (closer) {
            record.objectIndex = i;
        }
    }

    return record;
}


vec3 perPixel(inout uint rngState) {
    Ray ray = genRay();
    vec3 light = vec3(0.0, 0.0, 0.0);
    vec3 contribution = vec3(1.0, 1.0, 1.0);

    for (float i = 0; i < config.bounceLimit; i++) {
        HitRecord record = traceRay(ray);

        if (record.hitDistance == FLT_MAX) {
            light += scene.backgroundColor * contribution;
            break;
        }

        contribution *= scene.spheres[record.objectIndex].color;

        ray.origin = record.worldPosition + record.worldNormal * 0.001;
        // ray.direction = reflect(ray.direction, record.worldNormal);
        ray.direction = normalize(record.worldNormal + randomDirection(rngState));
    }

    return light;
}


void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    uint rngState = pixelCoord.x * pixelCoord.y;

    vec3 color = vec3(0.0, 0.0, 0.0);
    for (float i = 0; i < config.numSamples; i++) {
        color += perPixel(rngState);
    }
    imageStore(outputImage, pixelCoord, vec4(color/config.numSamples, 1.0));
}
