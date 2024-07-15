
#include "src/camera.h"
#include "src/logger.h"
#include "src/renderer.h"
#include "src/test_scenes.h"


bool changeIndex(unsigned int& index, KeyboardKey key) {
    unsigned int tmp = index;
    index -= (IsKeyDown(key) && IsKeyPressed(KEY_LEFT));
    index += (IsKeyDown(key) && IsKeyPressed(KEY_RIGHT));
    return index != tmp;
}


int main() {
    logger::setLogLevel(logger::LogLevel::TRACE);
    const int windowWidth = 1280;
    const int windowHeight = 720;
    const float scale = 2.0;
    const int imageWidth = windowWidth / scale;
    const int imageHeight = windowHeight / scale;

    Renderer renderer({windowWidth, windowHeight}, {imageWidth, imageHeight});

    CompileShaderParams params = {
        .workgroupSize = 8,
        .storageType = SceneStorageType::Uniforms,
        // .storageType = SceneStorageType::Buffers,

        .maxSphereCount = 16,
        .maxTriangleCount = 5,
    };

    renderer.compileComputeShader(params);

    Vector3 camPosition = {0, 0, 6};
    Vector3 camDirection = {0, 0, -1};
    float camFov = 60.0;
    SceneCameraParams camParams = {
        .speed = 10.0,
    };

    SceneCamera camera(camPosition, camDirection, camFov, {imageWidth, imageHeight}, camParams);

    const rt::CompiledScene scenes[] = {
        createScene_1(),
        createScene_2(),
        createRandomScene(8, 4),
        createRandomScene(16, 4),
    };
    const int numScenes = sizeof(scenes) / sizeof(rt::CompiledScene);

    const rt::Config configs[] = {
        {.numSamples = 1, .bounceLimit = 5},
        {.numSamples = 4, .bounceLimit = 5},
        {.numSamples = 16, .bounceLimit = 5},
        {.numSamples = 32, .bounceLimit = 5},
    };
    const int numConfigs = sizeof(configs) / sizeof(rt::Config);

    unsigned sceneIdx = 0;
    unsigned configIdx = 2;
    bool benchmarkMode = false;
    bool debugDraw = false;

    renderer.setCamera(camera);
    renderer.setScene(scenes[sceneIdx]);
    renderer.setConfig(configs[configIdx]);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_B)) {
            benchmarkMode = !benchmarkMode;
            if (benchmarkMode) {
                SetTargetFPS(0);
            } else {
                SetTargetFPS(30);
            }
        }
        if (IsKeyPressed(KEY_D)) {
            debugDraw = !debugDraw;
        }

        if (camera.update(GetFrameTime())) {
            renderer.setCamera(camera);
            renderer.resetImage();
        }

        if (changeIndex(sceneIdx, KEY_S)) {
            const rt::CompiledScene& scene = scenes[sceneIdx % numScenes];
            renderer.setScene(scene);
            renderer.resetImage();
        }

        if (changeIndex(configIdx, KEY_C)) {
            const rt::Config& config = configs[configIdx % numConfigs];
            renderer.setConfig(config);
        }

        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
            renderer.saveImage("output.png");
        }

        renderer.render(true, true, debugDraw);
    }
}
