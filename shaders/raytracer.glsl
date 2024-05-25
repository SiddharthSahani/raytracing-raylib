
#version 430 core
layout (local_size_x = WG_SIZE_PLACEHOLDER, local_size_y = WG_SIZE_PLACEHOLDER, local_size_z = 1) in;


layout (rgba32f, binding = 0) uniform image2D outputImage;

layout (std430, binding = 1) readonly buffer uniforms {
    float iTime;
};


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
};


void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 imgSize = imageSize(outputImage);
    float horizontalCoeff = (pixelCoord.x * 2.0 - imgSize.x) / imgSize.x;
    float verticalCoeff = (pixelCoord.y * 2.0 - imgSize.y) / imgSize.x;

    Camera camera;
    camera.position = vec3(0.0, 0.0, 6.0);
    camera.direction = vec3(0.0, 0.0, -1.0);

    vec3 upDirection = vec3(0.0, 1.0, 0.0);
    vec3 rightDirection = cross(camera.direction, upDirection);
    Ray ray;
    ray.origin = camera.position;
    ray.direction = camera.direction + horizontalCoeff * rightDirection + verticalCoeff * upDirection;

    Sphere sphere;
    sphere.position = vec3(0.0, 0.0, 0.0);
    sphere.radius = 1.0;

    vec3 oc = ray.origin - sphere.position;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float d = b * b - 4 * a * c;

    if (d > 0.0) {
        imageStore(outputImage, pixelCoord, vec4(1.0, 0.0, 0.0, 1.0));
    } else {
        imageStore(outputImage, pixelCoord, vec4(0.0, 0.0, 1.0, 1.0));
    }
}
