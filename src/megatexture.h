
#pragma once

#include <raylib/raylib.h>
#include <vector>


struct MaterialImage {
    MaterialImage(const char* dirPath) {
        FilePathList imagePaths = LoadDirectoryFilesEx(dirPath, ".h;.cpp", false);
        // TraceLog(LOG_WARNING, "%d %d", imagePaths.capacity, imagePaths.count);
        for (int i = 0; i < imagePaths.count; i++) {
            TraceLog(LOG_WARNING, "%s", imagePaths.paths[i]);
        }
        UnloadDirectoryFiles(imagePaths);
    }

    ~MaterialImage() {
        // UnloadImage(albedo);
        // UnloadImage(ao);
        // UnloadImage(normal);
        // UnloadImage(specular);
        // UnloadImage(emissive);
    }

    Image albedo;
    Image ao;
    Image normal;
    Image specular;
    Image emissive;
};


class MegaTexture {

public:
    MegaTexture(const std::vector<MaterialImage>&);

private:
    Texture m_texture;
};
