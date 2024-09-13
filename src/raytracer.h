
#pragma once

#include "src/structs/camera.h"
#include "src/compiledscene.h"
#include "src/structs/config.h"


enum class SceneStorageType {
    UBO,
    SSBO,
};


struct ComputeShaderParams {
    uint32_t workgroupSize;
    SceneStorageType storageType;
    uint32_t maxSphereCount;
    uint32_t maxTriangleCount;
};


class Raytracer {

public:
    Raytracer(Vector2 textureSize, const ComputeShaderParams& shaderParams);
    ~Raytracer();
    const Vector2& getTextureSize() const { return m_textureSize; }
    int getFrameIndex() const { return m_frameIndex; }
    void setCamera(const rt::Camera& camera);
    void setScene(const rt::CompiledScene& scene);
    void setConfig(const rt::Config& config);
    bool saveImage(const char* fileName) const;
    void reset();

private:
    void makeTexture();
    void makeBuffers();
    char* loadComputeShaderContents();
    void compileComputeShader();
    Texture getOutTexture() const { return m_outTexture; }
    void runComputeShader();
    void setScene_materials(const rt::CompiledScene& scene);
    void setScene_spheres(const rt::CompiledScene& scene);
    void setScene_triangles(const rt::CompiledScene& scene);

private:
    Vector2 m_textureSize;
    // shader will write to this texture
    Texture m_outTexture;
    // used to average frames over time
    int m_frameIndex = 0;

    ComputeShaderParams m_shaderParams;

    uint32_t m_computeShaderProgram = 0;
    uint32_t m_sceneSpheresBuffer = 0;
    uint32_t m_sceneTrianglesBuffer = 0;


    friend class Renderer;

};
