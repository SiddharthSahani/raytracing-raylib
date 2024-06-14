
#version 430 core
layout (local_size_x = WG_SIZE, local_size_y = WG_SIZE, local_size_z = 1) in;

#define FLT_MAX 3.402823466e+38F


// ----- STRUCT DEFINITIONS -----

struct Camera {
    mat4 invViewMat;
    mat4 invProjMat;
    vec3 position;
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


struct Triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    int materialIndex;
};


struct Material {
    vec3 albedo;
    float roughness;
    float emissionPower;
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
    int numTriangles;
};


struct Config {
    float bounceLimit;
    float numSamples;
};


// ----- UNIFORMS AND BUFFERS -----

layout (rgba16f, binding = 0) uniform image2D outImage;
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

    struct SceneTriangles {
        Triangle data[MAX_TRIANGLE_COUNT];
    };

    uniform SceneSpheres sceneSpheres;
    uniform SceneTriangles sceneTriangles;

#else

    layout (std430, binding = 2) readonly buffer sceneSpheresBlock {
        Sphere data[];
    } sceneSpheres;

    layout (std430, binding = 3) readonly buffer sceneTrianglesBlock {
        Triangle data[];
    } sceneTriangles;

#endif

// ----- RNG FUNCTIONS -----

// PCG https://www.shadertoy.com/view/XlGcRh
uint nextRandom(inout uint state) {
    state = state * 747796405u + 2891336453u;
    uint result = ((state >> ((state >> 28) + 4u)) ^ state) * 277803737u;
    result = (result >> 22) ^ result;
    return result;
}


float randomValue(inout uint state) {
    return nextRandom(state) / 4294967295.0;
}


float randomNormalFloat(inout uint seed) {
    float theta = 2.0f * 3.1415926 * randomValue(seed);
    float rho = sqrt(-2.0f * log(randomValue(seed)));
    return rho * cos(theta);
}


vec3 randomDirection(inout uint state) {
    return normalize(vec3(
        // randomValue(state),
        // randomValue(state),
        // randomValue(state)
        randomNormalFloat(state),
        randomNormalFloat(state),
        randomNormalFloat(state)
    ) * 2 - 1);
}


// ----- INTERSECTION FUNCTIONS -----

bool hit(Sphere sphere, Ray ray, out HitRecord record) {
    vec3 oc = ray.origin - sphere.position;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float d = b * b - 4.0 * a * c;

    if (d < 0.0) {
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


bool hit(Triangle triangle, Ray ray, out HitRecord record) {
    vec3 v0v1 = triangle.v1 - triangle.v0;
    vec3 v0v2 = triangle.v2 - triangle.v0;
    vec3 pvec = cross(ray.direction, v0v2);

    float det = dot(v0v1, pvec);
    if (abs(det) < 0.00001) {
        return false;
    }

    float invDet = 1.0 / det;
    vec3 tvec = ray.origin - triangle.v0;
    float u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0) {
        return false;
    }

    vec3 qvec = cross(tvec, v0v1);
    float v = dot(ray.direction, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }

    float t = dot(v0v2, qvec) * invDet;
    if (t > 0.0 && t < record.hitDistance) {
        record.worldPosition = ray.origin + ray.direction * t;
        record.hitDistance = t;
        record.worldNormal = normalize(cross(v0v1, v0v2));
        record.worldNormal *= dot(record.worldNormal, ray.direction) < 0.0 ? 1 : -1;
        record.materialIndex = triangle.materialIndex;
        return true;
    }

    return false;
}

// ----- MAIN FUNCTIONS -----

Ray genRay() {
    vec2 imgSize = imageSize(outImage);
    vec2 coord = vec2(gl_GlobalInvocationID.x, imgSize.y - gl_GlobalInvocationID.y);
    coord = coord / imgSize * 2.0 - 1.0;

    vec4 target = camera.invProjMat * vec4(coord, 1.0, 1.0);

    Ray ray;
    ray.origin = camera.position;
    ray.direction = (camera.invViewMat * vec4(normalize(target.xyz / target.w), 0.0)).xyz;
    return ray;
}


HitRecord traceRay(Ray ray) {
    HitRecord record;
    record.hitDistance = FLT_MAX;

    for (int i = 0; i < sceneInfo.numSpheres; i++) {
        hit(sceneSpheres.data[i], ray, record);
    }

    for (int i = 0; i < sceneInfo.numTriangles; i++) {
        hit(sceneTriangles.data[i], ray, record);
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

        light += materials.data[record.materialIndex].albedo * materials.data[record.materialIndex].emissionPower * contribution;
        contribution *= materials.data[record.materialIndex].albedo;

        vec3 diffuseDir = normalize(record.worldNormal + randomDirection(rngState));
        vec3 specularDir = reflect(ray.direction, record.worldNormal);

        ray.origin = record.worldPosition + record.worldNormal * 0.001;
        ray.direction = normalize(mix(diffuseDir, specularDir, materials.data[record.materialIndex].roughness));
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
