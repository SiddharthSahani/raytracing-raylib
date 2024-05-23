
#pragma once


namespace rl {
#include <raylib/raylib.h>
}


class Renderer {

public:
    Renderer(rl::Vector2 windowSize, rl::Vector2 imageSize);
    ~Renderer();
    void loop();

private:
    void runShader();
    void updateShaderCamera();
    void updateShaderSpheres();

private:
    rl::Vector2 m_windowSize;
    rl::Vector2 m_imageSize;
    rl::Shader m_raytracingShader;
    rl::RenderTexture m_renderTexture;
};
