
#include "src/material.h"
#include <random>


namespace rt {


std::random_device randomDevice;
std::mt19937 rngGenerator(randomDevice());


uint8_t getRandomU8(uint8_t mean, float stddev) {
    std::normal_distribution<float> distribution(mean / 255.0f, stddev);
    float res = distribution(rngGenerator);
    if (res < 0.0f) res = 0.0f;
    if (res > 1.0f) res = 1.0f;
    return res * 255;
}


Material::Material() {
    m_albedoData = AlbedoInfo{
        .color = RAYWHITE,
        .deviation = 1.0,
    };
}


Material::~Material() {
    Image* image = std::get_if<Image>(&m_albedoData);
    if (image) {
        UnloadImage(*image);
    }
}


void Material::setAlbedo(AlbedoInfo info) { m_albedoData = info; }


void Material::setAlbedo(const char* filepath) { setAlbedo(LoadImage(filepath)); }


void Material::setAlbedo(Image image) {
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    m_albedoData = image;
}


Image Material::make(int width, int height) {
    const int w = width / 1;
    Image res = GenImageColor(width, height, BLANK);

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

    return res;
}


} // namespace rt
