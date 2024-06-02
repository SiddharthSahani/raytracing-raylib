
#pragma once

#include "src/structs/camera.h"
#include "src/structs/config.h"
#include "src/structs/scene.h"


enum class SceneStorageType {
    Uniforms,
    Buffers,
};


struct CompileShaderParams {
    unsigned workgroupSize;
    SceneStorageType storageType;
    unsigned maxSphereCount;
    unsigned maxPlaneCount;
    unsigned maxTriangleCount;
};


class Renderer {

public:
    Renderer(Vector2 windowSize, Vector2 imageSize);
    ~Renderer();
    void draw() const;
    void compileComputeShader(CompileShaderParams params);
    void runComputeShader();

    bool canRender() const;
    unsigned getComputeShaderId() const { return m_computeShaderProgram; }

    void setCurrentCamera(const rt::Camera& camera);
    void setCurrentScene(const rt::Scene& scene);
    void setCurrentConfig(const rt::Config& config);

private:
    template <class... Args> int getUniformLoc(const char* fmt, Args... args) const;
    void makeOutImage();
    void makeBufferObjects();
    void setScene_spheres(const rt::Scene& scene);
    void setScene_planes(const rt::Scene& scene);
    void setScene_triangles(const rt::Scene& scene);

private:
    Vector2 m_windowSize;
    Vector2 m_imageSize;
    Texture m_outImage;

    bool m_hasCamera = false;
    bool m_hasScene = false;
    bool m_hasConfig = false;

    unsigned m_computeShaderProgram = 0;
    CompileShaderParams m_compileParams;

    unsigned m_sceneMaterialsBuffer = 0;
    unsigned m_sceneSpheresBuffer = 0;
    unsigned m_scenePlanesBuffer = 0;
    unsigned m_sceneTrianglesBuffer = 0;
};
