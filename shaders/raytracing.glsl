
#version 330 core


uniform vec2 uImageSize;


void main() {
    gl_FragColor = vec4(gl_FragCoord.xy / uImageSize, 0.0, 1.0);
}
