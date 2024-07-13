
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
    void render(bool compute = true, bool draw = true);
    void compileComputeShader(CompileShaderParams params);
    void resetImage();
    bool saveImage(const char* filepath) const;

    void setCamera(const SceneCamera& camera) const;
    void setScene(const rt::CompiledScene& scene) const;
    void setConfig(const rt::Config& config) const;

    unsigned getComputeShaderId() const { return m_computeShaderProgram; }

private:
    template <class... Args> int getUniformLoc(const char* fmt, Args... args) const;
    void runComputeShader();
    void drawOutImage() const;
    void makeOutImage();
    void makeBufferObjects();
    void setScene_spheres(const rt::CompiledScene& scene) const;
    void setScene_triangles(const rt::CompiledScene& scene) const;

private:
    Vector2 m_windowSize;
    Vector2 m_imageSize;
    Texture m_outImage;

    int m_frameIndex = 0;
    unsigned m_computeShaderProgram = 0;
    CompileShaderParams m_compileParams;
    bool m_compiled = false;

    unsigned m_sceneSpheresBuffer = 0;
    unsigned m_sceneTrianglesBuffer = 0;
};
