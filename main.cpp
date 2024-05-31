
#include "src/renderer.h"


rt::Scene createRandomScene(int numSpheres) {
    SetRandomSeed(0);

    rt::Scene scene;

    for (int i = 0; i < numSpheres; i++) {
        Vector3 pos = {
            GetRandomValue(-22000, 22000) / 10000.0f,
            GetRandomValue(-22000, 22000) / 10000.0f,
            GetRandomValue(-22000, 22000) / 10000.0f,
        };
        float rad = GetRandomValue(3000, 8000) / 10000.0f;

        scene.spheres.push_back(rt::Sphere{pos, rad, i});

        Color col = {
            GetRandomValue(0, 255),
            GetRandomValue(0, 255),
            GetRandomValue(0, 255),
            255,
        };
        float roughness = GetRandomValue(3, 8) / 10.0f;

        scene.materials.push_back(rt::Material{col, roughness});
    }

    scene.backgroundColor = {210, 210, 240, 255};
    return scene;
}


rt::Scene createScene_1() {
    rt::Scene scene;

    {
        rt::Material mat = rt::Material({50, 230, 200, 255}, 1.0);
        scene.materials.push_back(mat);
        rt::Sphere centerSphere = rt::Sphere({0, 0, 0}, 1.0, 0);
        scene.spheres.push_back(centerSphere);
    }

    {
        rt::Material mat = rt::Material({200, 180, 190, 255}, 0.2);
        scene.materials.push_back(mat);
        rt::Sphere groundSphere = rt::Sphere({0, -6, 0}, 5.0, 1);
        scene.spheres.push_back(groundSphere);
    }

    {
        rt::Material mat = rt::Material({220, 220, 220, 255}, 0.0);
        scene.materials.push_back(mat);
        rt::Plane plane = rt::Plane({1.3, 0, -1}, {0.7, 0, 1}, {0, 1, 0}, {1.2, 1.2}, 2);
        scene.planes.push_back(plane);
    }

    scene.backgroundColor = {210, 210, 210, 255};
    return scene;
}


int main() {
    const int windowWidth = 1280;
    const int windowHeight = 720;
    const float scale = 2.0;
    const int imageWidth = windowWidth / scale;
    const int imageHeight = windowHeight / scale;

    Renderer renderer({windowWidth, windowHeight}, {imageWidth, imageHeight});

    const int workgroupSize = 8;
    const int maxSphereCount = 32; // Max number of spheres allowed
    const int maxPlaneCount = 5; // Max number of planes allowed
    const bool useBuffers = false; // Do not use uniforms, use buffers (large memory size)
    renderer.compileComputeShader(workgroupSize, maxSphereCount, maxPlaneCount, useBuffers);

    renderer.setCurrentCamera({
        .position = {0, 0, 6},
        .direction = {0, 0, -1},
    });
    renderer.setCurrentConfig({
        .bounceLimit = 5,
        .numSamples = 16,
    });

    // renderer.setCurrentScene(createRandomScene(16));
    renderer.setCurrentScene(createScene_1());

    while (!WindowShouldClose()) {
        renderer.runComputeShader();
        renderer.draw();
    }
}
