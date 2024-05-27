
#pragma once

#include <raylib/raylib.h>


class Renderer {

public:
    Renderer(Vector2 windowSize, Vector2 imageSize, unsigned computeLocalSize = 8);
    ~Renderer();
    void loop();

private:
    void makeImage();
    void makeBufferObjects();
    void compileComputeShader();
    void runComputeShader();
    void updateShaderCamera();
    void updateShaderSpheres();
    void updateShaderConfig();

private:
    Vector2 m_windowSize;
    Vector2 m_imageSize;
    unsigned m_computeLocalSize;
    Texture m_outImage;
    unsigned m_computeShaderProgram;
};
