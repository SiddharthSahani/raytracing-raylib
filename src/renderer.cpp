
#include "src/renderer.h"

using namespace rl;
#include <raylib/rlgl.h>


Renderer::Renderer(rl::Vector2 windowSize, rl::Vector2 imageSize, unsigned computeLocalSize)
    : m_windowSize(windowSize), m_imageSize(imageSize), m_computeLocalSize(computeLocalSize) {
    rl::InitWindow(m_windowSize.x, m_windowSize.y, "Raytracing");
    rl::SetTargetFPS(30);

    makeOutputTexture();
    makeBufferObjects();
    compileComputeShader();
}


Renderer::~Renderer() {
    rlUnloadShaderProgram(m_computeShaderProgram);
    rl::UnloadTexture(m_outputTexture);
    rl::CloseWindow();
}


void Renderer::loop() {
    rlEnableShader(m_computeShaderProgram);
    updateShaderCamera();
    updateShaderSpheres();

    while (!rl::WindowShouldClose()) {
        runComputeShader();

        rl::BeginDrawing();

        rl::DrawTexturePro(m_outputTexture, {0, 0, m_imageSize.x, m_imageSize.y},
                           {0, 0, m_windowSize.x, m_windowSize.y}, {0, 0}, 0, rl::WHITE);

        rl::DrawFPS(10, 10);
        rl::EndDrawing();
    }
}


void Renderer::makeOutputTexture() {
    rl::Image image = rl::GenImageColor(m_imageSize.x, m_imageSize.y, rl::GREEN);
    rl::ImageFormat(&image, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    m_outputTexture = rl::LoadTextureFromImage(image);
    rl::UnloadImage(image);
}


void Renderer::makeBufferObjects() {
    m_buffer = rlLoadShaderBuffer(sizeof(float), nullptr, RL_DYNAMIC_COPY);
}


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
    float time = rl::GetTime();
    rlUpdateShaderBuffer(m_buffer, &time, sizeof(float), 0);

    rlBindImageTexture(m_outputTexture.id, 0, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, false);
    rlBindShaderBuffer(m_buffer, 1);

    rlEnableShader(m_computeShaderProgram);
    const int groupX = m_imageSize.x / m_computeLocalSize;
    const int groupY = m_imageSize.y / m_computeLocalSize;
    rlComputeShaderDispatch(groupX, groupY, 1);
    rlDisableShader();
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
        rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, rl::TextFormat("scene.spheres[%d].position", i)),
                     &pos, RL_SHADER_UNIFORM_VEC3, 1);
        rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, rl::TextFormat("scene.spheres[%d].radius", i)),
                     &rad, RL_SHADER_UNIFORM_FLOAT, 1);
        rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, rl::TextFormat("scene.spheres[%d].color", i)),
                     &col, RL_SHADER_UNIFORM_VEC3, 1);
    }

    // {
    //     rl::Vector3 spherePos = {0, 0, 0};
    //     float sphereRad = 1.0;
    //     rl::Vector3 sphereCol = {0.2, 0.9, 0.8};
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.spheres[0].position"),
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
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.spheres[1].position"),
    //                  &spherePos, RL_SHADER_UNIFORM_VEC3, 1);
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.spheres[1].radius"),
    //                  &sphereRad, RL_SHADER_UNIFORM_FLOAT, 1);
    //     rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.spheres[1].color"),
    //                  &sphereCol, RL_SHADER_UNIFORM_VEC3, 1);
    // }

    // int numSpheres = 2;
    rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.numSpheres"), &numSpheres,
                 RL_SHADER_UNIFORM_INT, 1);

    rl::Vector3 backgroundColor = {210/255.0f, 210/255.0f, 210/255.0f};
    rlSetUniform(rlGetLocationUniform(m_computeShaderProgram, "scene.backgroundColor"),
                 &backgroundColor, RL_SHADER_UNIFORM_VEC3, 1);
}
