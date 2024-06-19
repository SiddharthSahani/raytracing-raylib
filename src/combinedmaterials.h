
#include <raylib/raylib.h>


namespace rt {


Image createAlbedoImage(Color color, int width, int height);


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
    int getTextureId() const { return m_texture.id; }
    int getNumMaterials() const { return m_numMaterials; }

private:
    void createTexture();

private:
    int m_numMaterials = 0;
    int m_size = 0;
    Texture m_texture;
};


} // namespace rt
