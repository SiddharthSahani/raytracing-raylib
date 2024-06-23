
#include "src/material.h"


namespace rt {


Material::Material() {
    m_albedoData = AlbedoInfo{
        .color = RAYWHITE,
    };
}


Material::~Material() {
    Image* image = std::get_if<Image>(&m_albedoData);
    if (image) {
        UnloadImage(*image);
    }
}


void Material::setAlbedo(AlbedoInfo info) { m_albedoData = info; }


void Material::setAlbedo(const char* filepath) {
    Image image = LoadImage(filepath);
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    m_albedoData = image;
}


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
                    albedoInfo->color.r,
                    albedoInfo->color.g,
                    albedoInfo->color.b,
                    255,
                };
            }
        }
    }

    return res;
}


} // namespace rt
