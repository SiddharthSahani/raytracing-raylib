
#include "src/packedmaterialdata.h"
#include "src/logger.h"


namespace rt {


static unsigned currentId = 0;


PackedMaterialData::PackedMaterialData(int materialCount, Vector2 textureSize)
    : m_id(++currentId), m_materialCount(materialCount), m_textureSize(textureSize) {
    INFO("Creating materialData with %d materials and of size = %d x %d [ID: %u]", materialCount, (int) m_textureSize.x, (int) m_textureSize.y, m_id);

    createFrameBuffer();
    createShader();
}


PackedMaterialData::~PackedMaterialData() {
    TRACE("Unloading materialData [ID: %u]", m_id);
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
        const Vector2 uCurrent = {(float) i, (float) index};

        BeginShaderMode(m_shader);
        SetShaderValue(m_shader, uCurrent_uniLoc, &uCurrent, SHADER_UNIFORM_VEC2);
        SetShaderValue(m_shader, mean_uniLoc, &info.mean, SHADER_UNIFORM_VEC4);
        SetShaderValue(m_shader, deviation_uniLoc, &info.deviation, SHADER_UNIFORM_VEC2);
        SetShaderValue(m_shader, useTextures_uniLoc, &info.useTextures, SHADER_UNIFORM_VEC2);
        SetShaderValueTexture(m_shader, uTextureRGB_uniLoc, info.textures[0]);
        SetShaderValueTexture(m_shader, uTextureA_uniLoc, info.textures[1]);

        TRACE("    Setting materialIndex = %d with Material[ID: %u]", index, material.getId());
        TRACE("        Uniform vec2 set [index = %d | uCurrent = (%f %f)]", uCurrent_uniLoc, uCurrent.x, uCurrent.y);
        TRACE("        Uniform vec4 set [index = %d | mean = (%f %f %f %f)]", mean_uniLoc, info.mean.x, info.mean.y, info.mean.z, info.mean.w);
        TRACE("        Uniform vec2 set [index = %d | deviation = (%f %f)]", deviation_uniLoc, info.deviation.x, info.deviation.y);
        TRACE("        Uniform vec2 set [index = %d | useTextures = (%f %f)]", useTextures_uniLoc, info.useTextures.x, info.useTextures.y);

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
    TRACE("    Created texture for materialData [ID: %u]", m_id);
}


void PackedMaterialData::createShader() {
    m_shader = LoadShader(nullptr, "shaders/packedmaterialgen.glsl");
}


} // namespace rt
