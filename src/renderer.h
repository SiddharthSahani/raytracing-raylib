
#pragma once

#include "src/raytracer.h"


class Renderer {

public:
    Renderer(Vector2 windowSize);
    void setRaytracer(std::weak_ptr<Raytracer> raytracer);
    ~Renderer();
    void render();
    void draw();

private:
    Vector2 m_windowSize;
    std::weak_ptr<Raytracer> m_raytracer;

    Texture m_blankTexture;
    Shader m_texFragShader;
};
