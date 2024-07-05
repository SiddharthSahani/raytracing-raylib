
#include "src/material.h"
#include <random>


namespace rt {


Material::Material() {
    m_albedoData = AlbedoInfo{
        .color = RAYWHITE,
        .deviation = 0.1,
    };

    m_roughnessData = RoughnessInfo{
        .value = 1.0,
        .deviation = 0.01,
    };
}


Material::~Material() {
    Image* image;

    image = std::get_if<Image>(&m_albedoData);
    if (image) {
        UnloadImage(*image);
        image = nullptr;
    }

    image = std::get_if<Image>(&m_roughnessData);
    if (image) {
        UnloadImage(*image);
        image = nullptr;
    }
}


void Material::setAlbedo(AlbedoInfo info) { m_albedoData = info; }


void Material::setAlbedo(const char* filepath) { setAlbedo(LoadImage(filepath)); }


void Material::setAlbedo(Image image) {
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    m_albedoData = image;
}


void Material::setRoughness(RoughnessInfo info) { m_roughnessData = info; }


void Material::setRoughness(const char* filepath) { setRoughness(LoadImage(filepath)); }


void Material::setRoughness(Image image) {
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
    m_roughnessData = image;
}


} // namespace rt
