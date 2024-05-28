
#pragma once

#include "src/structs/camera.h"
#include "src/structs/config.h"
#include "src/structs/scene.h"


class Renderer {

public:
    Renderer(Vector2 windowSize, Vector2 imageSize, unsigned computeLocalSize = 8,
             int maxSphereCount = 32);
    ~Renderer();
    void draw() const;
    void runComputeShader();

    bool canRender() const { return m_hasCamera && m_hasScene && m_hasConfig; }

    void setCurrentCamera(const rt::Camera& camera);
    void setCurrentScene(const rt::Scene& scene);
    void setCurrentConfig(const rt::Config& config);

private:
    int getUniformLoc(const char*) const;
    void makeImage();
    void makeBufferObjects();
    void compileComputeShader();

private:
    Vector2 m_windowSize;
    Vector2 m_imageSize;
    Texture m_outImage;

    bool m_hasCamera = false;
    bool m_hasScene = false;
    bool m_hasConfig = false;

    int m_maxSphereCount;
    unsigned m_computeLocalSize;
    unsigned m_computeShaderProgram;
    unsigned m_sceneObjectsBuffer = -1;
};
