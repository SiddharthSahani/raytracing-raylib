
#include "src/packedmaterialdata.h"


namespace rt {


PackedMaterialData::PackedMaterialData(int materialCount, Vector2 textureSize)
    : m_materialCount(materialCount), m_textureSize(textureSize) {

    createFrameBuffer();
    createShader();
}


PackedMaterialData::~PackedMaterialData() {
    UnloadRenderTexture(m_renderTexture);
    UnloadShader(m_shader);
}


void PackedMaterialData::setMaterial(int index, const Material& material) {
    const Vector2 uGridSize = {1.0, (float)m_materialCount};

    BeginShaderMode(m_shader);

    static int uResolution_uniLoc = GetShaderLocation(m_shader, "uResolution");
    static int uGridSize_uniLoc = GetShaderLocation(m_shader, "uGridSize");
    static int uCurrent_uniLoc = GetShaderLocation(m_shader, "uCurrent");
    static int mean_uniLoc = GetShaderLocation(m_shader, "mean");
    static int deviation_uniLoc = GetShaderLocation(m_shader, "deviation");
    static int useTextures_uniLoc = GetShaderLocation(m_shader, "useTextures");
    static int uTextureRGB_uniLoc = GetShaderLocation(m_shader, "uTextureRGB");
    static int uTextureA_uniLoc = GetShaderLocation(m_shader, "uTextureA");

    SetShaderValue(m_shader, uResolution_uniLoc, &m_textureSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(m_shader, uGridSize_uniLoc, &uGridSize, SHADER_UNIFORM_VEC2);
    EndShaderMode();

    BeginTextureMode(m_renderTexture);
    BeginBlendMode(BLEND_ADD_COLORS);

    for (int i = 0; i < 1; i++) {
        const Material::BlockInfo info = material.getBlockInfo(i);
        const Vector2 uCurrent = {(float)i, (float)index};

        BeginShaderMode(m_shader);
        SetShaderValue(m_shader, uCurrent_uniLoc, &uCurrent, SHADER_UNIFORM_VEC2);
        SetShaderValue(m_shader, mean_uniLoc, &info.mean, SHADER_UNIFORM_VEC4);
        SetShaderValue(m_shader, deviation_uniLoc, &info.deviation, SHADER_UNIFORM_VEC2);
        SetShaderValue(m_shader, useTextures_uniLoc, &info.useTextures, SHADER_UNIFORM_VEC2);
        SetShaderValueTexture(m_shader, uTextureRGB_uniLoc, info.textures[0]);
        SetShaderValueTexture(m_shader, uTextureA_uniLoc, info.textures[1]);

        DrawRectangle(0, 0, m_textureSize.x, m_textureSize.y, RED);
        EndShaderMode();

        if (info.useTextures.x == 1.0) {
            UnloadTexture(info.textures[0]);
        }
        if (info.useTextures.y == 1.0) {
            UnloadTexture(info.textures[1]);
        }
    }

    EndBlendMode();
    EndTextureMode();
}


void PackedMaterialData::createFrameBuffer() {
    m_renderTexture = LoadRenderTexture(m_textureSize.x, m_textureSize.y);
}


void PackedMaterialData::createShader() {
    m_shader = LoadShader(nullptr, "shaders/packedmaterialgen.glsl");
}


} // namespace rt
