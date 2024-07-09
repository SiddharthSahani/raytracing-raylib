
#include "src/camera.h"
#include "src/renderer.h"


rt::CompiledScene createRandomScene(int numSpheres, int numMats) {
    SetRandomSeed(0);

    rt::Scene scene;

    std::vector<std::shared_ptr<rt::Material>> materials;
    for (int i = 0; i < numMats; i++) {
        Vector4 col = ColorNormalize({
            .r = (uint8_t)GetRandomValue(0, 255),
            .g = (uint8_t)GetRandomValue(0, 255),
            .b = (uint8_t)GetRandomValue(0, 255),
            .a = 255,
        });
        float roughness = GetRandomValue(0, 5) / 10.0;

        auto mat = std::make_shared<rt::Material>();
        mat->setAlbedo({.value = {col.x, col.y, col.z}, .deviation = 0.05});
        mat->setRoughness({.value = roughness, .deviation = 0.01});
        materials.push_back(mat);
    }

    for (int i = 0; i < numSpheres; i++) {
        Vector3 pos = {
            GetRandomValue(-40000, 40000) / 10000.0f,
            GetRandomValue(-40000, 40000) / 10000.0f,
            GetRandomValue(-40000, 40000) / 10000.0f,
        };
        float rad = GetRandomValue(5000, 10000) / 10000.0f;
        auto mat = materials[GetRandomValue(0, numMats - 1)];

        rt::Sphere sph = {
            .position = pos,
            .radius = rad,
            .material = mat,
        };
        scene.addObject(sph);
    }

    scene.backgroundColor = {210, 210, 240, 255};

    return rt::CompiledScene(scene, {2048, 2048});
}


rt::CompiledScene createScene_1() {

    rt::Scene scene;

    // defining materials
    auto sphereMat = std::make_shared<rt::Material>();
    auto redMat = std::make_shared<rt::Material>();
    auto mirrorMat = std::make_shared<rt::Material>();

    sphereMat->setAlbedo({.value = {0.2, 0.9, 0.8}, .deviation = 0.03});
    // sphereMat->setAlbedo("earthmap1k.png");

    redMat->setAlbedo({.value = {0.8, 0.3, 0.3}, .deviation = 0.1});

    mirrorMat->setAlbedo({.value = {0.8, 0.8, 0.8}, .deviation = 0.02});
    mirrorMat->setRoughness({.value = 0.0, .deviation = 0.01});

    // defining objects
    {
        rt::Sphere centerSphere = {
            .position = {0, 0, 0},
            .radius = 1.0,
            .material = sphereMat,
        };
        scene.addObject(centerSphere);
    }
    {
        rt::Sphere groundSphere = {
            .position = {0, -6, 0},
            .radius = 5.0,
            .material = sphereMat,
        };
        scene.addObject(groundSphere);
    }
    {
        rt::Triangle mirror = {
            .v0 = {-1.3, +0.0, -1.2},
            .v1 = {-2.0, +1.1, +1.0},
            .v2 = {-2.0, -1.1, +1.0},
            .material = mirrorMat,
        };
        scene.addObject(mirror);
    }
    {
        rt::Sphere glowingSphere = {
            .position = {-1.3, 0, -1.2},
            .radius = 0.1,
            .material = redMat,
        };
        scene.addObject(glowingSphere);
    }

    scene.backgroundColor = {200, 200, 200, 255};

    return rt::CompiledScene(scene, {4096, 4096});
}


rt::CompiledScene createScene_2() {

    rt::Scene scene;

    // defining materials
    auto purpleMat = std::make_shared<rt::Material>();
    auto blueMat = std::make_shared<rt::Material>();
    auto emissiveMat = std::make_shared<rt::Material>();
    auto mirrorMat = std::make_shared<rt::Material>();

    purpleMat->setAlbedo({.value = {1.0, 0.0, 1.0}, .deviation = 0.1});

    blueMat->setAlbedo({.value = {0.2, 0.3, 1.0}, .deviation = 0.0});

    emissiveMat->setAlbedo({.value = {0.8, 0.5, 0.2}, .deviation = 0.05});
    emissiveMat->setAlbedo({.value = {1.0, 0.6, 0.2}, .deviation = 0.01});

    mirrorMat->setAlbedo({.value = {0.8, 0.8, 0.8}, .deviation = 0.0});
    mirrorMat->setRoughness({.value = 0.0, .deviation = 0.0});

    // defining objects
    {
        rt::Sphere sphere = {
            .position = {0.5, 0.0, 0.0},
            .radius = 1.0,
            .material = purpleMat,
        };
        scene.addObject(sphere);
    }
    {
        rt::Sphere sphere = {
            .position = {0.0, -31.0, 0.0},
            .radius = 30.0,
            .material = blueMat,
        };
        scene.addObject(sphere);
    }
    {
        rt::Sphere sphere = {
            .position = {10.0, 4.0, -25.0},
            .radius = 15.0,
            .material = emissiveMat,
        };
        scene.addObject(sphere);
    }
    {
        rt::Triangle tr = {
            .v0 = {0.0, 2.0, -3.0},
            .v1 = {-2.0, 2.0, -2.0},
            .v2 = {0.0, -1.0, -3.0},
            .material = mirrorMat,
        };
        scene.addObject(tr);
    }
    {
        rt::Triangle tr = {
            .v0 = {-2.0, 2.0, -2.0},
            .v1 = {0.0, -1.0, -3.0},
            .v2 = {-2.0, -1.0, -2.0},
            .material = mirrorMat,
        };
        scene.addObject(tr);
    }

    scene.backgroundColor = {200, 200, 200, 255};

    return rt::CompiledScene(scene, {4096, 4096});
}


bool changeIndex(unsigned int& index, KeyboardKey key) {
    unsigned int tmp = index;
    index -= (IsKeyDown(key) && IsKeyPressed(KEY_LEFT));
    index += (IsKeyDown(key) && IsKeyPressed(KEY_RIGHT));
    return index != tmp;
}


int main() {
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

        renderer.render();
    }
}
