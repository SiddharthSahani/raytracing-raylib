
#include "src/camera.h"
#include "src/renderer.h"


rt::Scene createRandomScene(int numSpheres) {
    SetRandomSeed(0);

    rt::Scene scene;

    for (int i = 0; i < 5; i++) {
        Color col = {
            GetRandomValue(0, 255),
            GetRandomValue(0, 255),
            GetRandomValue(0, 255),
            255,
        };
        float roughness = i / 4.0;

        rt::Material material = rt::Material(col, roughness, 0.0);
        scene.materials.push_back(material);
    }

    for (int i = 0; i < numSpheres; i++) {
        Vector3 pos = {
            GetRandomValue(-22000, 22000) / 10000.0f,
            GetRandomValue(-22000, 22000) / 10000.0f,
            GetRandomValue(-22000, 22000) / 10000.0f,
        };
        float rad = GetRandomValue(3000, 8000) / 10000.0f;

        scene.spheres.push_back(rt::Sphere{pos, rad, GetRandomValue(0, 4)});
    }

    scene.backgroundColor = {210, 210, 240, 255};
    return scene;
}


rt::Scene createScene_1() {
    rt::Scene scene;

    {
        rt::Material mat = rt::Material({50, 230, 200, 255}, 0.0, 0.0);
        scene.materials.push_back(mat);
        rt::Sphere centerSphere = rt::Sphere({0, 0, 0}, 1.0, 0);
        scene.spheres.push_back(centerSphere);
    }

    {
        rt::Material mat = rt::Material({200, 180, 190, 255}, 0.8, 0.0);
        scene.materials.push_back(mat);
        rt::Sphere groundSphere = rt::Sphere({0, -6, 0}, 5.0, 1);
        scene.spheres.push_back(groundSphere);
    }

    {
        rt::Material mat = rt::Material({220, 220, 220, 255}, 1.0, 0.0);
        scene.materials.push_back(mat);
        rt::Plane plane = rt::Plane({1.3, 0, -1}, {0.7, 0, 1}, {0, 1, 0}, {1.2, 1.2}, 2);
        scene.planes.push_back(plane);
    }

    {
        rt::Triangle triangle = rt::Triangle({-1.3, 0, -1.2}, {-2, 1.1, 1}, {-2, -1.1, 1}, 2);
        scene.triangles.push_back(triangle);
    }

    {
        rt::Material mat = rt::Material({210, 75, 70, 255}, 1.0, 5.0);
        scene.materials.push_back(mat);
        rt::Sphere glowingSphere = rt::Sphere({-1.3, 0, -1.2}, 0.1, 3);
        scene.spheres.push_back(glowingSphere);
    }

    scene.backgroundColor = {200, 200, 200, 255};
    return scene;
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
        .maxPlaneCount = 5,
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
    rt::Scene scene = createScene_1();
    // rt::Scene scene = createRandomScene(16);
    rt::Config config = {
        .bounceLimit = 5,
        .numSamples = 16,
    };

    bool raytrace = true;

    // SetTargetFPS(0);
    while (!WindowShouldClose()) {
        bool forceCameraUpdate = false;
        if (camera.update(GetFrameTime())) {
            renderer.resetImage();
            forceCameraUpdate = true;
        }

        if (IsKeyPressed(KEY_SPACE)) {
            raytrace = !raytrace;
            forceCameraUpdate = true;
        }

        renderer.render(camera, scene, config, forceCameraUpdate, raytrace);
    }
}
