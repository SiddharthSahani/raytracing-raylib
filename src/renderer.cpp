
#include "src/renderer.h"

using namespace rl;
#include <raylib/raymath.h>


Renderer::Renderer(rl::Vector2 windowSize, rl::Vector2 imageSize)
    : m_windowSize(windowSize), m_imageSize(imageSize) {
    rl::InitWindow(m_windowSize.x, m_windowSize.y, "Raytracing");
    rl::SetTargetFPS(30);

    m_renderTexture = rl::LoadRenderTexture(m_imageSize.x, m_imageSize.y);
    m_raytracingShader = rl::LoadShader(0, "shaders/raytracing.glsl");
}


Renderer::~Renderer() {
    rl::UnloadShader(m_raytracingShader);
    rl::UnloadRenderTexture(m_renderTexture);
    rl::CloseWindow();
}


void Renderer::loop() {
    updateShaderCamera();

    while (!rl::WindowShouldClose()) {
        runShader();

        rl::BeginDrawing();

        rl::DrawTexturePro(m_renderTexture.texture, {0, 0, m_imageSize.x, m_imageSize.y},
                           {m_windowSize.x, m_windowSize.y, m_windowSize.x, m_windowSize.y}, {0, 0},
                           180, rl::WHITE);

        rl::DrawFPS(10, 10);
        rl::EndDrawing();
    }
}


void Renderer::runShader() {
    rl::BeginTextureMode(m_renderTexture);

    rl::BeginShaderMode(m_raytracingShader);
    rl::DrawRectangleV({0, 0}, m_imageSize, rl::WHITE);
    rl::EndShaderMode();

    rl::EndTextureMode();
}


void Renderer::updateShaderCamera() {
    rl::Vector3 position = {0, 0, 6};
    rl::Vector3 direction = {0, 0, -1};

    rl::Matrix viewMat = MatrixLookAt(position, Vector3Add(position, direction), {0, 1, 0});
    rl::Matrix invViewMat = MatrixInvert(viewMat);

    rl::Matrix projMat =
        MatrixPerspective(60.0 * DEG2RAD, m_imageSize.x / m_imageSize.y, 0.1, 100.0);
    rl::Matrix invProjMat = MatrixInvert(projMat);

    rl::SetShaderValue(m_raytracingShader, rl::GetShaderLocation(m_raytracingShader, "uImageSize"),
                       &m_imageSize, rl::SHADER_UNIFORM_VEC2);
    rl::SetShaderValueMatrix(m_raytracingShader,
                             rl::GetShaderLocation(m_raytracingShader, "camera.invViewMat"),
                             invViewMat);
    rl::SetShaderValueMatrix(m_raytracingShader,
                             rl::GetShaderLocation(m_raytracingShader, "camera.invProjMat"),
                             invProjMat);
    rl::SetShaderValue(m_raytracingShader,
                       rl::GetShaderLocation(m_raytracingShader, "camera.position"), &position,
                       rl::SHADER_UNIFORM_VEC3);
}
