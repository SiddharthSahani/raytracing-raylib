
#include <raylib/raylib.h>


namespace rt {


Image createAlbedoImage(Color color, int size);


class Material {

public:
    Material(const Image& albedoImage);
    ~Material();

private:
    const Image& m_albedoImage;

friend class CombinedMaterial;
};


class CombinedMaterial {

public:
    CombinedMaterial(int numMaterials, int size);
    ~CombinedMaterial();
    void addMaterial(const Material& material, int index);

private:
    void createTexture();

private:
    int m_numMaterials;
    int m_size;
    Texture m_texture;
};


} // namespace rt
