
#include "src/renderer.h"


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
    rl::SetShaderValue(m_raytracingShader, rl::GetShaderLocation(m_raytracingShader, "uImageSize"),
                       &m_imageSize, rl::SHADER_UNIFORM_VEC2);

    while (!rl::WindowShouldClose()) {
        runShader();

        rl::BeginDrawing();

        rl::DrawTexturePro(m_renderTexture.texture, {0, 0, m_imageSize.x, m_imageSize.y},
                           {0, 0, m_windowSize.x, m_windowSize.y}, {0, 0}, 0, rl::WHITE);

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
