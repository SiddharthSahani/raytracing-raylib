
// #include "src/renderer.h"


// int main() {
//     const int windowWidth = 1280;
//     const int windowHeight = 720;
//     const float scale = 2.0;
//     const int imageWidth = windowWidth / scale;
//     const int imageHeight = windowHeight / scale;

//     Renderer renderer({windowWidth, windowHeight}, {imageWidth, imageHeight});
//     renderer.loop();
// }


#include "src/builder/scene.h"
#include <iostream>


int main() {
    char* memory[200 * 1024];

    Scene scene;
    scene.spheres.push_back({
        .position = {0, 0, 0},
        .radius = 1.0,
        .color = MAGENTA,
    });

    std::cout << pass(scene, memory, 32) << '\n';
}
