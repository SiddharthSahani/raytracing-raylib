
#include "src/combinedmaterials.h"


namespace rt {


Image createAlbedoImage(Color color, int width, int height) {
    Image res = GenImageColor(width, height, color);
    return res;
}


Material::Material(const Image& albedoImage) : m_albedoImage(albedoImage) {}


Material::~Material() { UnloadImage(m_albedoImage); }


CombinedMaterial::CombinedMaterial(int numMaterials, int size)
    : m_numMaterials(numMaterials), m_size(size) {
    createTexture();
}


CombinedMaterial::~CombinedMaterial() {
    UnloadTexture(m_texture);
}


void CombinedMaterial::addMaterial(const Material& material, int index) {
    const float w = m_size / 1;
    const float h = m_size / m_numMaterials;
    const float y = h * index;

    Image image = material.m_albedoImage;

    if (image.width == w && image.height == h) {
        UpdateTextureRec(m_texture, {0, y, w, h}, image.data);
    } else {
        Image temp = ImageCopy(image);
        ImageResize(&temp, w, h);
        UpdateTextureRec(m_texture, {0, y, w, h}, temp.data);
        UnloadImage(temp);
    }
}


void CombinedMaterial::createTexture() {
    Image temp = GenImageColor(m_size, m_size, BLANK);
    m_texture = LoadTextureFromImage(temp);
    UnloadImage(temp);
}


} // namespace rt
