
#include "src/packedmaterialdata.h"


namespace rt {


PackedMaterialData::PackedMaterialData(int materialCount, int textureSize)
    : m_materialCount(materialCount), m_textureSize(textureSize) {

    Image temp = GenImageColor(textureSize, textureSize, BLANK);
    m_materialsData = LoadTextureFromImage(temp);
    UnloadImage(temp);
}


PackedMaterialData::~PackedMaterialData() {
    UnloadTexture(m_materialsData);
}


void PackedMaterialData::setMaterial(Material& material, int index) {
    const float h = m_textureSize / m_materialCount;

    Image materialData = material.make(m_textureSize, h);
    UpdateTextureRec(m_materialsData, {0, h * index, (float) m_textureSize, h}, materialData.data);
    UnloadImage(materialData);
}


} // namespace rt
