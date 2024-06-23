
#include "src/material.h"
#include <raylib/raylib.h>


namespace rt {


class PackedMaterialData {

public:
    PackedMaterialData(int materialCount, int textureSize);
    ~PackedMaterialData();
    void setMaterial(Material& material, int index);
    int getTextureId() const { return m_materialsData.id; }
    int getNumMaterials() const { return m_materialCount; }

private:
    int m_materialCount = 0;
    int m_textureSize = 0;
    Texture m_materialsData;
};


} // namespace rt
