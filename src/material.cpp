
#include "src/material.h"
#include <random>


namespace rt {


std::random_device randomDevice;
std::mt19937 rngGenerator(randomDevice());


uint8_t getRandomU8(uint8_t mean, float stddev) {
    std::normal_distribution<float> distribution(mean / 255.0f, stddev);
    float res = distribution(rngGenerator);
    if (res < 0.0f)
        res = 0.0f;
    if (res > 1.0f)
        res = 1.0f;
    return res * 255;
}


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


Image Material::make(int width, int height) {
    const int w = width / 1;
    Image res = GenImageColor(width, height, BLANK);

    // albedo data
    {
        if (Image* image = std::get_if<Image>(&m_albedoData)) {
            ImageResizeNN(image, w, height);
            Color* data = (Color*)image->data;
            for (int i = 0; i < w * height; i++) {
                ((Color*)res.data)[i] = {
                    data[i].r,
                    data[i].g,
                    data[i].b,
                    255,
                };
            }
        } else if (AlbedoInfo* albedoInfo = std::get_if<AlbedoInfo>(&m_albedoData)) {
            for (int i = 0; i < w * height; i++) {
                ((Color*)res.data)[i] = {
                    getRandomU8(albedoInfo->color.r, albedoInfo->deviation),
                    getRandomU8(albedoInfo->color.g, albedoInfo->deviation),
                    getRandomU8(albedoInfo->color.b, albedoInfo->deviation),
                    255,
                };
            }
        }
    }

    // roughness data
    {
        if (Image* image = std::get_if<Image>(&m_roughnessData)) {
            ImageResizeNN(image, w, height);
            uint8_t* data = (uint8_t*)image->data;
            for (int i = 0; i < w * height; i++) {
                ((Color*)res.data)[i].a = data[i];
            }
        } else if (RoughnessInfo* roughnessInfo = std::get_if<RoughnessInfo>(&m_roughnessData)) {
            for (int i = 0; i < w * height; i++) {
                ((Color*)res.data)[i].a =
                    getRandomU8(roughnessInfo->value * 255.0f, roughnessInfo->deviation);
            }
        }
    }

    return res;
}


} // namespace rt
