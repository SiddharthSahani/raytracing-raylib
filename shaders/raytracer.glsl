
#version 430 core
layout (local_size_x = WG_SIZE_PLACEHOLDER, local_size_y = WG_SIZE_PLACEHOLDER, local_size_z = 1) in;


layout (rgba32f, binding = 0) uniform image2D outputImage;

layout (std430, binding = 1) readonly buffer uniforms {
    float iTime;
};


void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    vec3 color = vec3(0.0, 0.0, 0.0);
    color.rg = vec2(coord) / imageSize(outputImage);
    color.b = abs(sin(color.r + iTime));

    imageStore(outputImage, coord, vec4(color, 1.0));
}
