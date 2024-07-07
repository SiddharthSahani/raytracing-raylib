
#pragma once

#include "src/camera.h"
#include "src/compiledscene.h"
#include "src/structs/config.h"


enum class SceneStorageType {
    Uniforms,
    Buffers,
};


struct CompileShaderParams {
    unsigned workgroupSize;
    SceneStorageType storageType;
    unsigned maxSphereCount;
    unsigned maxTriangleCount;
};


class Renderer {

public:
    Renderer(Vector2 windowSize, Vector2 imageSize);
    ~Renderer();
    void render(const SceneCamera& camera, const rt::CompiledScene& scene, const rt::Config& config,
                bool forceCameraUpdate = false);
    void compileComputeShader(CompileShaderParams params);
    void resetImage();

    bool canRender() const;
    unsigned getComputeShaderId() const { return m_computeShaderProgram; }


private:
    template <class... Args> int getUniformLoc(const char* fmt, Args... args) const;
    void runComputeShader();
    void makeOutImage();
    void makeBufferObjects();
    void updateCurrentCamera();
    void updateCurrentScene();
    void updateCurrentConfig();
    void setScene_spheres(const rt::CompiledScene& scene);
    void setScene_triangles(const rt::CompiledScene& scene);

private:
    Vector2 m_windowSize;
    Vector2 m_imageSize;
    Texture m_outImage;

    const SceneCamera* m_camera = nullptr;
    const rt::CompiledScene* m_scene = nullptr;
    const rt::Config* m_config = nullptr;

    int m_frameIndex = 0;
    unsigned m_computeShaderProgram = 0;
    CompileShaderParams m_compileParams;

    unsigned m_sceneMaterialsBuffer = 0;
    unsigned m_sceneSpheresBuffer = 0;
    unsigned m_sceneTrianglesBuffer = 0;
};
