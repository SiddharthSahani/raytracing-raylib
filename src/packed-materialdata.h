
#include "src/material.h"
#include <raylib/raylib.h>


namespace rt {


class PackedMaterialData {

public:
    PackedMaterialData(int materialCount, Vector2 textureSize);
    ~PackedMaterialData();
    unsigned getId() const { return m_id; }
    void setMaterial(int index, const Material& material);
    int getTextureId() const { return m_renderTexture.texture.id; }
    int getMaterialCount() const { return m_materialCount; }

private:
    void createFrameBuffer();
    void createShader();

private:
    unsigned m_id;
    int m_materialCount;
    Vector2 m_textureSize;
    RenderTexture m_renderTexture;
    Shader m_shader;
};


} // namespace rt
