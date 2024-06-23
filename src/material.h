
#pragma once


#include <raylib/raylib.h>
#include <variant>


namespace rt {


struct AlbedoInfo {
    Color color;
};


class Material {

public:
    Material();
    ~Material();
    void setAlbedo(AlbedoInfo info);
    void setAlbedo(const char* filepath);
    void setAlbedo(Image image);
    Image make(int width, int height);

private:
    std::variant<AlbedoInfo, Image> m_albedoData;
};


} // namespace rt
