
#version 430 core
layout (local_size_x = WG_SIZE, local_size_y = WG_SIZE, local_size_z = 1) in;

#define FLT_MAX 3.402823466e+38F


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
    int materialIndex;
};


struct Plane {
    vec3 center;
    int materialIndex;
    vec3 uDirection;
    float uSize;
    vec3 vDirection;
    float vSize;
};


struct Material {
    vec3 albedo;
    float roughness;
};


struct HitRecord {
    vec3 worldPosition;
    float hitDistance;
    vec3 worldNormal;
    int materialIndex;
};


struct SceneInfo {
    vec3 backgroundColor;
    int numSpheres;
    int numPlanes;
};


struct Config {
    float bounceLimit;
    float numSamples;
};


// ----- UNIFORMS AND BUFFERS -----

layout (rgba32f, binding = 0) uniform image2D outImage;
layout (std430, binding = 1) readonly buffer materialBlock {
    Material data[];
} materials;

uniform Camera camera;
uniform SceneInfo sceneInfo;
uniform Config config;
uniform int frameIndex;


#if USE_UNIFORM_OBJECTS

    struct SceneSpheres {
        Sphere data[MAX_SPHERE_COUNT];
    };

    struct ScenePlanes {
        Plane data[MAX_PLANE_COUNT];
    };

    uniform SceneSpheres sceneSpheres;
    uniform ScenePlanes scenePlanes;

#else

    layout (std430, binding = 2) readonly buffer sceneSpheresBlock {
        Sphere data[];
    } sceneSpheres;

    layout (std430, binding = 3) readonly buffer scenePlanesBlock {
        Plane data[];
    } scenePlanes;

#endif

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

bool hit(Sphere sphere, Ray ray, out HitRecord record) {
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
        record.materialIndex = sphere.materialIndex;
        return true;
    }

    return false;
}


bool hit(Plane plane, Ray ray, out HitRecord record) {
    vec3 planeNormal = cross(plane.uDirection, plane.vDirection);

    float denom = dot(planeNormal, ray.direction);
    if (denom > 0.00001) {
        return false;
    }

    float t = dot(plane.center - ray.origin, planeNormal) / denom;

    if (t > 0.0 && t < record.hitDistance) {
        vec3 p = ray.origin + t * ray.direction;
        vec3 dir = p - plane.center;

        float u = dot(dir, plane.uDirection);
        float v = dot(dir, plane.vDirection);

        if (u > -plane.uSize && u < plane.uSize && v > -plane.vSize && v < plane.vSize) {
            // uv = (vec2(u, v) + vec2(plane.uSize, plane.vSize)) / (2 * vec2(plane.uSize, plane.vSize));
            record.worldPosition = p;
            record.worldNormal = planeNormal;
            record.hitDistance = t;
            record.materialIndex = plane.materialIndex;
            return true;
        }
    }

    return false;
}


// ----- MAIN FUNCTIONS -----

Ray genRay() {
    vec2 imgSize = imageSize(outImage);
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

    for (int i = 0; i < sceneInfo.numSpheres; i++) {
        hit(sceneSpheres.data[i], ray, record);
    }

    for (int i = 0; i < sceneInfo.numPlanes; i++) {
        hit(scenePlanes.data[i], ray, record);
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
            light += sceneInfo.backgroundColor * contribution;
            break;
        }

        contribution *= materials.data[record.materialIndex].albedo;

        vec3 diffuseDir = normalize(record.worldNormal + randomDirection(rngState));
        vec3 specularDir = reflect(ray.direction, record.worldNormal);

        ray.origin = record.worldPosition + record.worldNormal * 0.001;
        ray.direction = normalize(mix(specularDir, diffuseDir, materials.data[record.materialIndex].roughness));
    }

    return light;
}


void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    uint rngState = pixelCoord.x * pixelCoord.y + uint(frameIndex) * 32421u;

    vec3 frameColor = vec3(0.0, 0.0, 0.0);
    for (float i = 0; i < config.numSamples; i++) {
        frameColor += perPixel(rngState);
    }
    frameColor /= config.numSamples;

    vec3 accumColor = imageLoad(outImage, pixelCoord).rgb;

    vec3 avgColor = (accumColor * (frameIndex-1) + frameColor) / frameIndex;
    imageStore(outImage, pixelCoord, vec4(avgColor, 1.0));
}
