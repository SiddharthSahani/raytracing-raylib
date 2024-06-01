
#pragma once

#include "src/structs/camera.h"
#include "src/structs/config.h"
#include "src/structs/scene.h"


class Renderer {

public:
    Renderer(Vector2 windowSize, Vector2 imageSize);
    ~Renderer();
    void draw() const;
    void compileComputeShader(unsigned computeLocalSize, unsigned maxSphereCount,
                              unsigned maxPlaneCount, unsigned maxTriangleCount, bool useBuffers);
    void runComputeShader();

    bool canRender() const {
        return m_computeShaderProgram && m_hasCamera && m_hasScene && m_hasConfig;
    }
    unsigned getComputeShaderId() const { return m_computeShaderProgram; }

    void setCurrentCamera(const rt::Camera& camera);
    void setCurrentScene(const rt::Scene& scene);
    void setCurrentConfig(const rt::Config& config);

private:
    int getUniformLoc(const char*) const;
    void makeOutImage();
    void makeBufferObjects();

private:
    Vector2 m_windowSize;
    Vector2 m_imageSize;
    Texture m_outImage;

    bool m_hasCamera = false;
    bool m_hasScene = false;
    bool m_hasConfig = false;

    unsigned m_computeLocalSize;
    unsigned m_computeShaderProgram = 0;
    bool m_usingBuffers;

    int m_maxSphereCount;
    int m_maxPlaneCount;
    int m_maxTriangleCount;

    unsigned m_sceneMaterialsBuffer = 0;
    unsigned m_sceneSpheresBuffer = 0;
    unsigned m_scenePlanesBuffer = 0;
    unsigned m_sceneTrianglesBuffer = 0;
};
