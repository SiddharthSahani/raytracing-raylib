
#include "src/renderer.h"

using namespace rl;
#include <raylib/rlgl.h>


Renderer::Renderer(rl::Vector2 windowSize, rl::Vector2 imageSize, unsigned computeLocalSize)
    : m_windowSize(windowSize), m_imageSize(imageSize), m_computeLocalSize(computeLocalSize) {
    rl::InitWindow(m_windowSize.x, m_windowSize.y, "Raytracing");
    rl::SetTargetFPS(30);

    makeImage();
    makeBufferObjects();
    compileComputeShader();
}


Renderer::~Renderer() {
    rlUnloadShaderProgram(m_computeShaderProgram);
    rl::UnloadTexture(m_outImage);
    rl::CloseWindow();
}


void Renderer::loop() {
    rlEnableShader(m_computeShaderProgram);
    updateShaderCamera();
    updateShaderSpheres();
    updateShaderConfig();

    static int frameIndex = 0;

    unsigned uniformLocation_frameIndex =
        rlGetLocationUniform(m_computeShaderProgram, "frameIndex");

    while (!rl::WindowShouldClose()) {
        frameIndex += 1;

        rlEnableShader(m_computeShaderProgram);
        rlSetUniform(uniformLocation_frameIndex, &frameIndex, RL_SHADER_UNIFORM_INT, 1);
        runComputeShader();
        rlDisableShader();

        rl::BeginDrawing();

        rl::DrawTexturePro(m_outImage, {0, 0, m_imageSize.x, m_imageSize.y},
                           {0, 0, m_windowSize.x, m_windowSize.y}, {0, 0}, 0, rl::WHITE);

        rl::DrawFPS(10, 10);
        rl::DrawText(rl::TextFormat("Frame Time: %.5f", rl::GetFrameTime()), 10, 30, 20, rl::DARKBLUE);
        rl::EndDrawing();
    }
}


void Renderer::makeImage() {
    rl::Image image = rl::GenImageColor(m_imageSize.x, m_imageSize.y, rl::BLUE);
    rl::ImageFormat(&image, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    m_outImage = rl::LoadTextureFromImage(image);
    rl::UnloadImage(image);
}


void Renderer::makeBufferObjects() {}


void Renderer::compileComputeShader() {
    char* fileText = rl::LoadFileText("shaders/raytracer.glsl");
    char* newfileText =
        rl::TextReplace(fileText, "WG_SIZE_PLACEHOLDER", rl::TextFormat("%d", m_computeLocalSize));
    rl::UnloadFileText(fileText);
    unsigned shaderId = rlCompileShader(newfileText, RL_COMPUTE_SHADER);
    m_computeShaderProgram = rlLoadComputeShaderProgram(shaderId);
    rl::UnloadFileText(newfileText);
}


void Renderer::runComputeShader() {
    const int groupX = m_imageSize.x / m_computeLocalSize;
    const int groupY = m_imageSize.y / m_computeLocalSize;

    rlBindImageTexture(m_outImage.id, 0, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, false);
    rlComputeShaderDispatch(groupX, groupY, 1);
}

void Renderer::updateShaderCamera() {
    rl::Vector3 position = {0, 0, 6};
    rl::Vector3 direction = {0, 0, -1};

    unsigned shaderLocation_cameraPosition =
        rlGetLocationUniform(m_computeShaderProgram, "camera.position");
    unsigned shaderLocation_cameraDirection =
        rlGetLocationUniform(m_computeShaderProgram, "camera.direction");

    rlSetUniform(shaderLocation_cameraPosition, &position, RL_SHADER_UNIFORM_VEC3, 1);
    rlSetUniform(shaderLocation_cameraDirection, &direction, RL_SHADER_UNIFORM_VEC3, 1);
}


void Renderer::updateShaderSpheres() {
    int numSpheres = 16;
    rl::SetRandomSeed(1);
    for (int i = 0; i < numSpheres; i++) {
        rl::Vector3 pos = {
            rl::GetRandomValue(-20000, 20000) / 10000.0f,
            rl::GetRandomValue(-20000, 20000) / 10000.0f,
            rl::GetRandomValue(-20000, 20000) / 10000.0f,
        };
        float rad = rl::GetRandomValue(5000, 8000) / 10000.0f;
        rl::Vector3 col = {
            rl::GetRandomValue(0, 10000) / 10000.0f,
            rl::GetRandomValue(0, 10000) / 10000.0f,
            rl::GetRandomValue(0, 10000) / 10000.0f,
        };
        rlSetUniform(rlGetLocationUniform(m_computeShaderProgram,
                                          rl::TextFormat("scene.spheres[%d].position", i)),
                     &pos, RL_SHADER_UNIFORM_VEC3, 1);
        rlSetUniform(rlGetLocationUniform(m_computeShaderProgram,
                                          rl::TextFormat("scene.spheres[%d].radius", i)),
                     &rad, RL_SHADER_UNIFORM_FLOAT, 1);
        rlSetUniform(rlGetLocationUniform(m_computeShaderProgram,
                                          rl::TextFormat("scene.spheres[%d].color", i)),
                     &col, RL_SHADER_UNIFORM_VEC3, 1);
    }

    // {
    //     rl::Vector3 spherePos = {0, 0, 0};
    //     float sphereRad = 1.0;
    //     rl::Vector3 sphereCol = {0.2, 0.9, 0.8};
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram,
    //     "scene.spheres[0].position"),
    //                  &spherePos, RL_SHADER_UNIFORM_VEC3, 1);
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.spheres[0].radius"),
    //                  &sphereRad, RL_SHADER_UNIFORM_FLOAT, 1);
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.spheres[0].color"),
    //                  &sphereCol, RL_SHADER_UNIFORM_VEC3, 1);
    // }
    // {
    //     rl::Vector3 spherePos = {0, -4, 0};
    //     float sphereRad = 3.0;
    //     rl::Vector3 sphereCol = {1, 0, 1};
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

    rl::Vector3 backgroundColor = {210 / 255.0f, 210 / 255.0f, 210 / 255.0f};
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
