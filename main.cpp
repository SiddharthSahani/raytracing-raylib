
#include "src/renderer.h"


int main() {
    const int windowWidth = 1280;
    const int windowHeight = 720;
    const float scale = 2.0;
    const int imageWidth = windowWidth / scale;
    const int imageHeight = windowHeight / scale;

    Renderer renderer({windowWidth, windowHeight}, {imageWidth, imageHeight});

    renderer.setCurrentCamera({
        .position = {0, 0, 6},
        .direction = {0, 0, -1},
    });
    renderer.setCurrentConfig({
        .bounceLimit = 5,
        .numSamples = 16,
    });

    renderer.setCurrentScene({
        .spheres = {
            {
                .position = {0, 0, 0},
                .radius = 1.0,
                .color = {50, 230, 205, 255},
            },
            {
                .position = {0, -6, 0},
                .radius = 5.0,
                .color = {255, 0, 255, 255},
            }
        },
        .backgroundColor = {210, 210, 215, 255},
    });

    while (!WindowShouldClose()) {
        renderer.runComputeShader();
        renderer.draw();
    }
}
