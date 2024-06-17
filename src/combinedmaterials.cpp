
#include "src/combinedmaterials.h"

using namespace rt;


Image createAlbedoImage(Color color, int size) {
    Image res = GenImageColor(size, size, color);
    ImageFormat(&res, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    return res;
}


Material::Material(const Image& albedoImage) : m_albedoImage(albedoImage) {}


Material::~Material() { UnloadImage(m_albedoImage); }


CombinedMaterial::CombinedMaterial(int numMaterials, int size)
    : m_numMaterials(numMaterials), m_size(size) {
    createTexture();
}


CombinedMaterial::~CombinedMaterial() { UnloadTexture(m_texture); }


void CombinedMaterial::addMaterial(const Material& material, int index) {
    const float w = m_size / 1;
    const float h = m_size / m_numMaterials;
    const float y = h * index;

    Image temp = ImageCopy(material.m_albedoImage);
    ImageResizeNN(&temp, w, h);

    UpdateTextureRec(m_texture, {0, y, w, h}, temp.data);

    UnloadImage(temp);
}


void CombinedMaterial::createTexture() {
    Image temp = GenImageColor(m_size, m_size, BLANK);
    ImageFormat(&temp, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    m_texture = LoadTextureFromImage(temp);
    UnloadImage(temp);
}
