
#pragma once


#include <raylib/raylib.h>
#include <variant>
#include <string>


namespace rt {


struct RGB_ChannelInfo {
    Vector3 value;
    float deviation;
};


struct A_ChannelInfo {
    float value;
    float deviation;
};


class Material {

public:
    Material(const std::string& name);
    ~Material();
    const std::string& getName() const { return m_name; }
    void setAlbedo(RGB_ChannelInfo info);
    void setAlbedo(const char* fileName);
    void setAlbedo(Image image);
    void setRoughness(A_ChannelInfo info);
    void setRoughness(const char* fileName);
    void setRoughness(Image image);

private:
    struct BlockInfo {
        Vector4 mean;
        Vector2 deviation;
        Vector2 useTextures;
        Texture textures[2];
    };

private:
    BlockInfo getBlockInfo(int blockIndex) const;

private:
    std::string m_name;
    std::variant<RGB_ChannelInfo, Image> m_albedoData;
    std::variant<A_ChannelInfo, Image> m_roughnessData;

    friend class PackedMaterialData;
};


} // namespace rt
