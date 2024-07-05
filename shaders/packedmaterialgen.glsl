
#version 430 core


uniform vec2 uResolution;
uniform vec2 uGridSize;
uniform vec2 uCurrent;

uniform vec4 mean;
uniform vec2 deviation;
uniform vec2 useTextures;

uniform sampler2D uTextureRGB;
uniform sampler2D uTextureA;


uint nextRandom(inout uint state) {
    state = state * 747796405u + 2891336453u;
    uint result = ((state >> ((state >> 28) + 4u)) ^ state) * 277803737u;
    result = (result >> 22) ^ result;
    return result;
}


float randomFloat(inout uint state) {
    return nextRandom(state) / 4294967295.0;
}


float randomNormalFloat(inout uint seed, float mean, float deviation) {
    float theta = 2.0f * 3.1415926 * randomFloat(seed);
    float rho = sqrt(-2.0f * log(randomFloat(seed)));
    float mult = rho * cos(theta);

    return mult * deviation + mean;
}


void main() {
    vec2 offset = 1.0 / uGridSize;
    vec2 outUv = gl_FragCoord.xy / uResolution;

    if (
        outUv.x < (uCurrent.x * offset.x) ||
        outUv.x > ((uCurrent.x + 1.0) * offset.x) ||
        outUv.y < (uCurrent.y * offset.y) ||
        outUv.y > ((uCurrent.y + 1.0) * offset.y)
    ) {
        discard;
    }

    vec2 uv = fract(outUv / offset);
    uint seed = uint(gl_FragCoord.x * gl_FragCoord.y + dot(deviation, vec2(100.0)));

    vec4 color;

    if (useTextures.x == 1.0) {
        color.rgb = texture(uTextureRGB, uv).rgb;
    } else {
        color.rgb = vec3(
            randomNormalFloat(seed, mean.r, deviation.x),
            randomNormalFloat(seed, mean.g, deviation.x),
            randomNormalFloat(seed, mean.b, deviation.x)
        );
    }

    if (useTextures.y == 1.0) {
        color.a = texture(uTextureA, uv).a;
    } else {
        color.a = randomNormalFloat(seed, mean.a, deviation.y);
    }

    color = clamp(color, 0.0, 1.0);
    gl_FragColor = color;
}
