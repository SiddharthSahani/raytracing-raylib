
#version 430 core


uniform sampler2D texture0;
uniform vec2 windowSize;
uniform float gamma;


void main() {
    vec2 uv = gl_FragCoord.xy / windowSize;
    uv.y = 1.0f - uv.y;

    vec4 color = texture(texture0, uv);
    color = pow(color, vec4(gamma));
    gl_FragColor = vec4(color.rgb, 1.0);
}
