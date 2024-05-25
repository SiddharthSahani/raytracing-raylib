
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
    void makeOutputTexture();
    void makeBufferObjects();
    void compileComputShader();
    void runComputeShader();
    // void updateShaderCamera();
    // void updateShaderSpheres();

private:
    rl::Vector2 m_windowSize;
    rl::Vector2 m_imageSize;
    rl::Texture m_outputTexture;
    unsigned m_computeShaderProgram;
    unsigned m_buffer;
};
