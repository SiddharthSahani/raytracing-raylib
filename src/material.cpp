
#include "src/material.h"


namespace rt {


static unsigned currentId = 0;


Material::Material()
    : m_id(++currentId) {
    m_albedoData = RGB_ChannelInfo{
        .value = {0.9, 0.9, 0.9},
        .deviation = 0.1,
    };
    m_roughnessData = A_ChannelInfo{
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


void Material::setAlbedo(RGB_ChannelInfo info) {
    if (Image* img = std::get_if<Image>(&m_albedoData)) {
        UnloadImage(*img);
        img->data = nullptr;
    }

    m_albedoData = info;
}


void Material::setAlbedo(const char* fileName) {
    if (Image* img = std::get_if<Image>(&m_albedoData)) {
        UnloadImage(*img);
        img->data = nullptr;
    }

    setAlbedo(LoadImage(fileName));
}


void Material::setAlbedo(Image image) {
    if (Image* img = std::get_if<Image>(&m_albedoData)) {
        UnloadImage(*img);
        img->data = nullptr;
    }

    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    m_albedoData = image;
}


void Material::setRoughness(A_ChannelInfo info) {
    if (Image* img = std::get_if<Image>(&m_roughnessData)) {
        UnloadImage(*img);
        img->data = nullptr;
    }

    m_roughnessData = info;
}


void Material::setRoughness(const char* fileName) {
    if (Image* img = std::get_if<Image>(&m_roughnessData)) {
        UnloadImage(*img);
        img->data = nullptr;
    }

    setRoughness(LoadImage(fileName));
}


void Material::setRoughness(Image image) {
    if (Image* img = std::get_if<Image>(&m_roughnessData)) {
        UnloadImage(*img);
        img->data = nullptr;
    }

    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
    m_roughnessData = image;
}


Material::BlockInfo Material::getBlockInfo(int blockIndex) const {
    const std::variant<RGB_ChannelInfo, Image>* variant_RGB = nullptr;
    const std::variant<A_ChannelInfo, Image>* variant_A = nullptr;

    switch (blockIndex) {

        // albedo and roughness
        case 0:
            variant_RGB = &m_albedoData;
            variant_A = &m_roughnessData;
            break;

    }

    Material::BlockInfo res;

    if (auto info = std::get_if<RGB_ChannelInfo>(variant_RGB)) {
        res.mean = {info->value.x, info->value.y, info->value.z, 0.0};
        res.deviation.x = info->deviation;
        res.useTextures.x = 0.0;
    } else if (auto image = std::get_if<Image>(variant_RGB)) {
        res.textures[0] = LoadTextureFromImage(*image);
        res.useTextures.x = 1.0;
    }

    if (auto info = std::get_if<A_ChannelInfo>(variant_A)) {
        res.mean.w = info->value;
        res.deviation.y = info->deviation;
        res.useTextures.y = 0.0;
    } else if (auto image = std::get_if<Image>(variant_A)) {
        res.textures[1] = LoadTextureFromImage(*image);
        res.useTextures.y = 1.0;
    }

    return res;
}


} // namespace rt
