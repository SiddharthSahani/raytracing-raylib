
#include "src/camera.h"
#include "src/logger.h"
#include "src/renderer.h"
#include "src/test_scenes.h"
#include "src/cli.h"


bool changeIndex(unsigned int& index, KeyboardKey key) {
    unsigned int tmp = index;
    index -= (IsKeyDown(key) && IsKeyPressed(KEY_LEFT));
    index += (IsKeyDown(key) && IsKeyPressed(KEY_RIGHT));
    return index != tmp;
}


ComputeShaderParams getShaderParams() {
    return {
        .workgroupSize = 8,
        .storageType = SceneStorageType::UBO,
        // .storageType = SceneStorageType::SSBO,

        .maxSphereCount = 16,
        .maxTriangleCount = 5,
    };
}


SceneCamera getSceneCamera(Vector2 imageSize) {
    const Vector3 camPosition = {0, 0, 6};
    const Vector3 camDirection = {0, 0, -1};
    const float camFov = 60.0;
    const SceneCameraParams camParams = {
        .speed = 10.0,
    };

    return SceneCamera(camPosition, camDirection, camFov, imageSize, camParams);
}


std::vector<std::unique_ptr<rt::CompiledScene>> createScenes() {
    std::vector<std::unique_ptr<rt::CompiledScene>> out;
    out.push_back(createScene_1());
    out.push_back(createScene_2());
    out.push_back(createRandomScene(8, 4));
    out.push_back(createRandomScene(16, 4));
    return out;
}


std::vector<rt::Config> createConfigs() {
    std::vector<rt::Config> out;
    out.push_back({.numSamples = 1, .bounceLimit = 5});
    out.push_back({.numSamples = 4, .bounceLimit = 5});
    out.push_back({.numSamples = 16, .bounceLimit = 5});
    out.push_back({.numSamples = 32, .bounceLimit = 5});
    return out;
}


int main(int argc, const char* argv[]) {
    CommandLineOptions options(argc, argv);
    logger::setLogLevel(options.verbose ? logger::LogLevel::TRACE : logger::LogLevel::INFO);

    const float imageWidth = options.windowWidth / options.imageScale;
    const float imageHeight = options.windowHeight / options.imageScale;

    Renderer renderer({options.windowWidth, options.windowHeight});

    ComputeShaderParams params = getShaderParams();
    std::shared_ptr raytracer = std::make_shared<Raytracer>(Vector2{imageWidth, imageHeight}, params);
    renderer.setRaytracer(raytracer);

    SceneCamera camera = getSceneCamera({imageWidth, imageHeight});

    const std::vector scenes = createScenes();
    const std::vector configs = createConfigs();

    unsigned sceneIdx = 0;
    unsigned configIdx = 2;
    bool benchmarkMode = false;

    raytracer->setCamera(camera.get());
    raytracer->setScene(*scenes[sceneIdx].get());
    raytracer->setConfig(configs[configIdx]);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_B)) {
            benchmarkMode = !benchmarkMode;
            if (benchmarkMode) {
                SetTargetFPS(0);
            } else {
                SetTargetFPS(30);
            }
        }

        if (camera.update(GetFrameTime())) {
            raytracer->setCamera(camera.get());
            raytracer->reset();
        }

        if (changeIndex(sceneIdx, KEY_S)) {
            const rt::CompiledScene& scene = *scenes[sceneIdx % scenes.size()];
            raytracer->setScene(scene);
            raytracer->reset();
        }

        if (changeIndex(configIdx, KEY_C)) {
            const rt::Config& config = configs[configIdx % configs.size()];
            raytracer->setConfig(config);
        }

        if (IsKeyDown(KEY_SPACE) && GetMouseWheelMove() != 0) {
            static float fov = 60.0f;
            fov += GetMouseWheelMove();
            camera.updateProjMatrix({imageWidth, imageHeight}, fov);
            raytracer->setCamera(camera.get());
            raytracer->reset();
        }

        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
            raytracer->saveImage("output.png");
        }

        if (IsKeyDown(KEY_M)) {
            renderer.setGamma(1.0);
        }

        renderer.render();
        renderer.draw();
    }
}
