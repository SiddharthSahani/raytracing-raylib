
#pragma once


#include <raylib/raylib.h>
#include <variant>


namespace rt {


struct AlbedoInfo {
    Color color;
    float deviation;
};


struct RoughnessInfo {
    float value;
    float deviation;
};


class Material {

public:
    Material();
    ~Material();
    void setAlbedo(AlbedoInfo info);
    void setAlbedo(const char* filepath);
    void setAlbedo(Image image);
    void setRoughness(RoughnessInfo info);
    void setRoughness(const char* filepath);
    void setRoughness(Image image);

private:
    std::variant<AlbedoInfo, Image> m_albedoData;
    std::variant<RoughnessInfo, Image> m_roughnessData;

    friend class PackedMaterialData;
};


} // namespace rt
