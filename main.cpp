
#include "src/camera.h"
#include "src/renderer.h"


// rt::Scene createRandomScene(int numSpheres) {
//     SetRandomSeed(0);

//     rt::Scene scene;

//     for (int i = 0; i < 5; i++) {
//         Color col = {
//             GetRandomValue(0, 255),
//             GetRandomValue(0, 255),
//             GetRandomValue(0, 255),
//             255,
//         };
//         float roughness = i / 4.0;

//         rt::Material material = rt::Material(col, roughness, 0.0);
//         scene.setMaterial(material);
//     }

//     for (int i = 0; i < numSpheres; i++) {
//         Vector3 pos = {
//             GetRandomValue(-40000, 40000) / 10000.0f,
//             GetRandomValue(-40000, 40000) / 10000.0f,
//             GetRandomValue(-40000, 40000) / 10000.0f,
//         };
//         float rad = GetRandomValue(5000, 10000) / 10000.0f;

//         rt::Sphere sph = rt::Sphere{pos, rad, GetRandomValue(0, 4)};
//         scene.addObject(sph);
//     }

//     scene.backgroundColor = {210, 210, 240, 255};
//     return scene;
// }


rt::Scene createScene_1() {

    rt::Scene scene = rt::Scene(new rt::PackedMaterialData(4, 512));

    {
        rt::Material mat;
        mat.setAlbedo({.color = {50, 230, 200, 255}});
        scene.materials->setMaterial(mat, 0);
    }
    {
        rt::Material mat;
        mat.setAlbedo({.color = {200, 180, 190, 255}});
        scene.materials->setMaterial(mat, 1);
    }
    {
        rt::Material mat;
        mat.setAlbedo({.color = {220, 220, 220, 255}});
        scene.materials->setMaterial(mat, 2);
    }
    {
        rt::Material mat;
        mat.setAlbedo({.color = {210, 75, 75, 255}});
        scene.materials->setMaterial(mat, 3);
    }

    {
        // rt::Material mat = rt::Material({50, 230, 200, 255}, 0.0, 0.0);
        // scene.setMaterial(mat);
        rt::Sphere centerSphere = rt::Sphere({0, 0, 0}, 1.0, 0);
        scene.addObject(centerSphere);
    }

    {
        // rt::Material mat = rt::Material({200, 180, 190, 255}, 0.8, 0.0);
        // scene.setMaterial(mat);
        rt::Sphere groundSphere = rt::Sphere({0, -6, 0}, 5.0, 1);
        scene.addObject(groundSphere);
    }

    {
        // rt::Material mat = rt::Material({220, 220, 220, 255}, 1.0, 0.0);
        // scene.setMaterial(mat);
        rt::Triangle triangle = rt::Triangle({-1.3, 0, -1.2}, {-2, 1.1, 1}, {-2, -1.1, 1}, 2);
        scene.addObject(triangle);
    }

    {
        // rt::Material mat = rt::Material({210, 75, 70, 255}, 1.0, 50.0);
        // scene.setMaterial(mat);
        rt::Sphere glowingSphere = rt::Sphere({-1.3, 0, -1.2}, 0.1, 3);
        scene.addObject(glowingSphere);
    }

    scene.backgroundColor = {200, 200, 200, 255};
    return scene;
}


// rt::Scene createScene_2() {
//     rt::Scene scene;

//     {
//         rt::Material mat = rt::Material({255, 0, 255, 255}, 0.0, 0.0);
//         scene.setMaterial(mat);
//         rt::Sphere sphere = rt::Sphere({0.5, 0.0, 0.0}, 1.0, 0);
//         scene.addObject(sphere);
//     }

//     {
//         rt::Material mat = rt::Material({50, 75, 255, 255}, 0.0, 0.0);
//         scene.setMaterial(mat);
//         rt::Sphere sphere = rt::Sphere({0.0, -101.0, 0.0}, 100.0, 1);
//         scene.addObject(sphere);
//     }

//     {
//         rt::Material mat = rt::Material({205, 128, 50, 255}, 0.0, 100.0);
//         scene.setMaterial(mat);
//         rt::Sphere sphere = rt::Sphere({32.0, 4.0, -32.0}, 20.0, 2);
//         scene.addObject(sphere);
//     }

//     {
//         rt::Material mat = rt::Material({205, 205, 205, 255}, 1.0, 0.0);
//         scene.setMaterial(mat);
//         rt::Triangle tri = rt::Triangle({0.0, 2.0, -3.0}, {-2.0, 2.0, -2.0}, {0.0, -1.0, -3.0},
//         3); scene.addObject(tri);
//     }

//     {
//         rt::Triangle tr = rt::Triangle({-2.0, 2.0, -2.0}, {0.0, -1.0, -3.0}, {-2.0, -1.0, -2.0},
//         3); scene.addObject(tr);
//     }

//     scene.backgroundColor = {200, 200, 200, 255};
//     return scene;
// }


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

    const rt::Scene scenes[] = {
        createScene_1(),
        // createScene_2(),
        // createRandomScene(8),
        // createRandomScene(16),
    };
    const int numScenes = sizeof(scenes) / sizeof(rt::Scene);

    const rt::Config configs[] = {
        {.bounceLimit = 5, .numSamples = 1},
        {.bounceLimit = 5, .numSamples = 4},
        {.bounceLimit = 5, .numSamples = 16},
        {.bounceLimit = 5, .numSamples = 32},
    };
    const int numConfigs = sizeof(configs) / sizeof(rt::Config);

    unsigned sceneIdx = 0;
    unsigned configIdx = 2;
    bool benchmarkMode = false;
    bool raytrace = true;

    // SetTargetFPS(0);
    while (!WindowShouldClose()) {
        bool forceCameraUpdate = false;

        if (camera.update(GetFrameTime())) {
            renderer.resetImage();
            forceCameraUpdate = true;
        }

        // if (IsKeyPressed(KEY_SPACE)) {
        //     raytrace = !raytrace;
        //     forceCameraUpdate = true;
        // }

        if (IsKeyPressed(KEY_B)) {
            benchmarkMode = !benchmarkMode;
            if (benchmarkMode) {
                SetTargetFPS(0);
            } else {
                SetTargetFPS(30);
            }
        }

        if (IsKeyDown(KEY_C)) {
            if (IsKeyPressed(KEY_LEFT)) {
                configIdx -= 1;
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                configIdx += 1;
            }
        }
        if (IsKeyDown(KEY_S)) {
            if (IsKeyPressed(KEY_LEFT)) {
                sceneIdx -= 1;
                renderer.resetImage();
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                sceneIdx += 1;
                renderer.resetImage();
            }
        }

        renderer.render(camera, scenes[sceneIdx % numScenes], configs[configIdx % numConfigs],
                        forceCameraUpdate, raytrace);
    }
}
