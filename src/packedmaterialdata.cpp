
#include "src/packedmaterialdata.h"


namespace rt {


PackedMaterialData::PackedMaterialData(int materialCount, Vector2 textureSize)
    : m_materialCount(materialCount), m_textureSize(textureSize) {

    createTexture();
    createShader();
}


PackedMaterialData::~PackedMaterialData() {
    UnloadRenderTexture(m_renderTexture);
    UnloadShader(m_shader);
}


void PackedMaterialData::setMaterial(int index, const Material& material) {
    const Vector2 uGridSize = {1, m_materialCount};

    Vector2 uCurrent;

    Vector4 meanRGBA;
    Vector2 stddevRGBA;
    Vector2 useTextureRGBA;
    Texture textureRGB;
    Texture textureA;

    BeginShaderMode(m_shader);

    static int uResolution_uniLoc = GetShaderLocation(m_shader, "uResolution");
    static int uGridSize_uniLoc = GetShaderLocation(m_shader, "uGridSize");
    static int uCurrent_uniLoc = GetShaderLocation(m_shader, "uCurrent");
    static int meanRGBA_uniLoc = GetShaderLocation(m_shader, "meanRGBA");
    static int stddevRGBA_uniLoc = GetShaderLocation(m_shader, "stddevRGBA");
    static int useTextureRGBA_uniLoc = GetShaderLocation(m_shader, "useTextureRGBA");
    static int uTextureRGB_uniLoc = GetShaderLocation(m_shader, "uTextureRGB");
    static int uTextureA_uniLoc = GetShaderLocation(m_shader, "uTextureA");

    SetShaderValue(m_shader, uResolution_uniLoc, &m_textureSize, SHADER_UNIFORM_VEC2);
    EndShaderMode();

    // SetShaderValueTexture(m_shader, GetShaderLocation(m_shader, "uTextureRGB"), textureRGB);
    // SetShaderValueTexture(m_shader, GetShaderLocation(m_shader, "uTextureA"), textureA);


    BeginTextureMode(m_renderTexture);
    BeginBlendMode(BLEND_ADD_COLORS);
    // BeginBlendMode(BLEND_SUBTRACT_COLORS);
    // BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

    // albedo and roughness
    {
        uCurrent = {0, (float)index};

        if (const auto* image = std::get_if<Image>(&material.m_albedoData)) {
            // useTextureRGB = 1.0;
            useTextureRGBA.x = 1.0;
            textureRGB = LoadTextureFromImage(*image);
        } else if (const auto* info = std::get_if<AlbedoInfo>(&material.m_albedoData)) {
            // useTextureRGB = 0.0;
            // meanRGB = {info->color.r / 255.0f, info->color.g / 255.0f, info->color.b / 255.0f};
            // stddevRGB = info->deviation;
            useTextureRGBA.x = 0.0;
            meanRGBA = {info->color.r / 255.0f, info->color.g / 255.0f, info->color.b / 255.0f,
                        1.0};
            stddevRGBA.x = info->deviation;
        } else {
            // useTextureRGB = 0.0;
            useTextureRGBA.x = 0.0;
            TraceLog(LOG_WARNING, "albedoData wasnt image as well as info [index=%d]", index);
        }

        if (const auto* image = std::get_if<Image>(&material.m_roughnessData)) {
            // useTextureA = 1.0;
            useTextureRGBA.y = 1.0;
            textureA = LoadTextureFromImage(*image);
        } else if (const auto* info = std::get_if<RoughnessInfo>(&material.m_roughnessData)) {
            // useTextureA = 0.0;
            // meanA = info->value;
            // stddevA = info->deviation;
            useTextureRGBA.y = 0.0;
            meanRGBA.w = info->value;
            stddevRGBA.y = info->deviation;
        } else {
            // useTextureA = 0.0;
            useTextureRGBA.y = 0.0;
            TraceLog(LOG_WARNING, "roughnessData wasnt image as well as info [index=%d]", index);
        }

        BeginShaderMode(m_shader);

        SetShaderValue(m_shader, uGridSize_uniLoc, &uGridSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(m_shader, uCurrent_uniLoc, &uCurrent, SHADER_UNIFORM_VEC2);
        SetShaderValue(m_shader, meanRGBA_uniLoc, &meanRGBA, SHADER_UNIFORM_VEC4);
        SetShaderValue(m_shader, stddevRGBA_uniLoc, &stddevRGBA, SHADER_UNIFORM_VEC2);
        SetShaderValue(m_shader, useTextureRGBA_uniLoc, &useTextureRGBA, SHADER_UNIFORM_VEC2);
        SetShaderValueTexture(m_shader, uTextureRGB_uniLoc, textureRGB);
        SetShaderValueTexture(m_shader, uTextureA_uniLoc, textureA);

        DrawRectangle(0, 0, m_textureSize.x, m_textureSize.y, RED);
        EndShaderMode();

        if (useTextureRGBA.x == 1.0) {
            UnloadTexture(textureRGB);
        }
        if (useTextureRGBA.y == 1.0) {
            UnloadTexture(textureA);
        }
    }

    EndBlendMode();
    EndTextureMode();
}


void PackedMaterialData::createTexture() {
    m_renderTexture = LoadRenderTexture(m_textureSize.x, m_textureSize.y);
}


void PackedMaterialData::createShader() {
    m_shader = LoadShader(nullptr, "shaders/packedmaterialgen.glsl");
}


} // namespace rt
