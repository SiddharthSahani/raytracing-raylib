
#include "src/renderer.h"
#include <raylib/rlgl.h>


Renderer::Renderer(Vector2 windowSize, Vector2 imageSize, unsigned computeLocalSize)
    : m_windowSize(windowSize), m_imageSize(imageSize), m_computeLocalSize(computeLocalSize) {
    InitWindow(m_windowSize.x, m_windowSize.y, "Raytracing");
    SetTargetFPS(30);

    makeImage();
    makeBufferObjects();
    compileComputeShader();
}


Renderer::~Renderer() {
    rlUnloadShaderProgram(m_computeShaderProgram);
    UnloadTexture(m_outImage);
    CloseWindow();
}


void Renderer::loop() {
    rlEnableShader(m_computeShaderProgram);
    updateShaderCamera();
    updateShaderSpheres();
    updateShaderConfig();

    static int frameIndex = 0;

    unsigned uniformLocation_frameIndex =
        rlGetLocationUniform(m_computeShaderProgram, "frameIndex");

    while (!WindowShouldClose()) {
        frameIndex += 1;

        rlEnableShader(m_computeShaderProgram);
        rlSetUniform(uniformLocation_frameIndex, &frameIndex, RL_SHADER_UNIFORM_INT, 1);
        runComputeShader();
        rlDisableShader();

        BeginDrawing();

        DrawTexturePro(m_outImage, {0, 0, m_imageSize.x, m_imageSize.y},
                       {0, 0, m_windowSize.x, m_windowSize.y}, {0, 0}, 0, WHITE);

        DrawFPS(10, 10);
        DrawText(TextFormat("Frame Time: %.5f", GetFrameTime()), 10, 30, 20, DARKBLUE);
        EndDrawing();
    }
}


void Renderer::makeImage() {
    Image image = GenImageColor(m_imageSize.x, m_imageSize.y, BLUE);
    ImageFormat(&image, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    m_outImage = LoadTextureFromImage(image);
    UnloadImage(image);
}


void Renderer::makeBufferObjects() {}


void Renderer::compileComputeShader() {
    char* fileText = LoadFileText("shaders/raytracer.glsl");
    char* newfileText =
        TextReplace(fileText, "WG_SIZE_PLACEHOLDER", TextFormat("%d", m_computeLocalSize));
    UnloadFileText(fileText);
    unsigned shaderId = rlCompileShader(newfileText, RL_COMPUTE_SHADER);
    m_computeShaderProgram = rlLoadComputeShaderProgram(shaderId);
    UnloadFileText(newfileText);
}


void Renderer::runComputeShader() {
    const int groupX = m_imageSize.x / m_computeLocalSize;
    const int groupY = m_imageSize.y / m_computeLocalSize;

    rlBindImageTexture(m_outImage.id, 0, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, false);
    rlComputeShaderDispatch(groupX, groupY, 1);
}

void Renderer::updateShaderCamera() {
    Vector3 position = {0, 0, 6};
    Vector3 direction = {0, 0, -1};

    unsigned shaderLocation_cameraPosition =
        rlGetLocationUniform(m_computeShaderProgram, "camera.position");
    unsigned shaderLocation_cameraDirection =
        rlGetLocationUniform(m_computeShaderProgram, "camera.direction");

    rlSetUniform(shaderLocation_cameraPosition, &position, RL_SHADER_UNIFORM_VEC3, 1);
    rlSetUniform(shaderLocation_cameraDirection, &direction, RL_SHADER_UNIFORM_VEC3, 1);
}


void Renderer::updateShaderSpheres() {
    int numSpheres = 16;
    SetRandomSeed(1);
    for (int i = 0; i < numSpheres; i++) {
        Vector3 pos = {
            GetRandomValue(-20000, 20000) / 10000.0f,
            GetRandomValue(-20000, 20000) / 10000.0f,
            GetRandomValue(-20000, 20000) / 10000.0f,
        };
        float rad = GetRandomValue(5000, 8000) / 10000.0f;
        Vector3 col = {
            GetRandomValue(0, 10000) / 10000.0f,
            GetRandomValue(0, 10000) / 10000.0f,
            GetRandomValue(0, 10000) / 10000.0f,
        };
        rlSetUniform(rlGetLocationUniform(m_computeShaderProgram,
                                          TextFormat("scene.spheres[%d].position", i)),
                     &pos, RL_SHADER_UNIFORM_VEC3, 1);
        rlSetUniform(
            rlGetLocationUniform(m_computeShaderProgram, TextFormat("scene.spheres[%d].radius", i)),
            &rad, RL_SHADER_UNIFORM_FLOAT, 1);
        rlSetUniform(
            rlGetLocationUniform(m_computeShaderProgram, TextFormat("scene.spheres[%d].color", i)),
            &col, RL_SHADER_UNIFORM_VEC3, 1);
    }

    // {
    //     Vector3 spherePos = {0, 0, 0};
    //     float sphereRad = 1.0;
    //     Vector3 sphereCol = {0.2, 0.9, 0.8};
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram,
    //     "scene.spheres[0].position"),
    //                  &spherePos, RL_SHADER_UNIFORM_VEC3, 1);
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.spheres[0].radius"),
    //                  &sphereRad, RL_SHADER_UNIFORM_FLOAT, 1);
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.spheres[0].color"),
    //                  &sphereCol, RL_SHADER_UNIFORM_VEC3, 1);
    // }
    // {
    //     Vector3 spherePos = {0, -4, 0};
    //     float sphereRad = 3.0;
    //     Vector3 sphereCol = {1, 0, 1};
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram,
    //     "scene.spheres[1].position"),
    //                  &spherePos, RL_SHADER_UNIFORM_VEC3, 1);
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.spheres[1].radius"),
    //                  &sphereRad, RL_SHADER_UNIFORM_FLOAT, 1);
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.spheres[1].color"),
    //                  &sphereCol, RL_SHADER_UNIFORM_VEC3, 1);
    // }

    // int numSpheres = 2;
    rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.numSpheres"), &numSpheres,
                 RL_SHADER_UNIFORM_INT, 1);

    Vector3 backgroundColor = {210 / 255.0f, 210 / 255.0f, 210 / 255.0f};
    rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.backgroundColor"),
                 &backgroundColor, RL_SHADER_UNIFORM_VEC3, 1);
}


void Renderer::updateShaderConfig() {
    float bounceLimit = 5;
    float numSamples = 16;
    rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "config.bounceLimit"), &bounceLimit,
                 RL_SHADER_UNIFORM_FLOAT, 1);
    rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "config.numSamples"), &numSamples,
                 RL_SHADER_UNIFORM_FLOAT, 1);
}
