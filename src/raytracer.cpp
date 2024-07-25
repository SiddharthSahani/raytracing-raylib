
#include "src/raytracer.h"
#include "src/logger.h"
#include <raylib/rlgl.h>


#define getUniLoc(fmt, ...) \
    rlGetLocationUniform(m_computeShaderProgram, TextFormat(fmt, ##__VA_ARGS__));


Raytracer::Raytracer(Vector2 textureSize, const ComputeShaderParams& shaderParams)
    : m_textureSize(textureSize), m_shaderParams(shaderParams) {

    makeTexture();
    makeBuffers();
    compileComputeShader();
}


Raytracer::~Raytracer() {
    rlUnloadShaderProgram(m_computeShaderProgram);
    TRACE("Unloaded compute shader program [ID: %u]", m_computeShaderProgram);

    UnloadTexture(m_outTexture);
    TRACE("Unloaded out texture [ID: %u]", m_outTexture.id);
}


void Raytracer::setCamera(const rt::Camera& camera) {
    static int invViewMat_uniLoc = getUniLoc("camera.invViewMat");
    static int invProjMat_uniLoc = getUniLoc("camera.invProjMat");
    static int position_uniLoc = getUniLoc("camera.position");

    rlEnableShader(m_computeShaderProgram);

    rlSetUniformMatrix(invViewMat_uniLoc, camera.invViewMat);
    rlSetUniformMatrix(invProjMat_uniLoc, camera.invProjMat);
    rlSetUniform(position_uniLoc, &camera.position, RL_SHADER_UNIFORM_VEC3, 1);
}


void Raytracer::setScene(const rt::CompiledScene& scene) {
    INFO("Setting scene [ID: %u]", scene.getId());
    rlEnableShader(m_computeShaderProgram);

    setScene_materials(scene);
    setScene_spheres(scene);
    setScene_triangles(scene);

    const int backgroundColor_uniLoc = getUniLoc("sceneInfo.backgroundColor");
    rlSetUniform(backgroundColor_uniLoc, &scene.m_backgroundColor, RL_SHADER_UNIFORM_VEC3, 1);
    TRACE("    backgroundColor = (%f %f %f)", scene.m_backgroundColor.x, scene.m_backgroundColor.y, scene.m_backgroundColor.z);
}


void Raytracer::setConfig(const rt::Config& config) {
    INFO("Setting configuration: {numSamples: %d, bounceLimit: %d}", (int) config.numSamples, (int) config.bounceLimit);
    rlEnableShader(m_computeShaderProgram);

    const int numSamples_uniLoc = getUniLoc("config.numSamples");
    rlSetUniform(numSamples_uniLoc, &config.numSamples, RL_SHADER_UNIFORM_FLOAT, 1);

    const int bounceLimit_uniLoc = getUniLoc("config.bounceLimit");
    rlSetUniform(bounceLimit_uniLoc, &config.bounceLimit, RL_SHADER_UNIFORM_FLOAT, 1);
}


bool Raytracer::saveImage(const char* fileName) const {
    TRACE("Saving texture as '%s'", fileName);

    float startTime = GetTime();

    Image img = LoadImageFromTexture(m_outTexture);
    bool saved = ExportImage(img, fileName);
    UnloadImage(img);

    float stopTime = GetTime();

    if (saved) {
        INFO("Saved texture as '%s' in %f seconds", fileName, stopTime - startTime);
    }
    return saved;
}


void Raytracer::makeTexture() {
    // creating a empty texture
    Image temp = GenImageColor(m_textureSize.x, m_textureSize.y, RED);
    ImageFormat(&temp, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16);
    m_outTexture = LoadTextureFromImage(temp);
    UnloadImage(temp);

    if (m_outTexture.id != 0) {
        const int sizeX = m_textureSize.x;
        const int sizeY = m_textureSize.y;
        INFO("Created out texture of size = %d x %d [ID: %u]", sizeX, sizeY, m_outTexture.id);
    }
}


void Raytracer::makeBuffers() {
    if (m_shaderParams.storageType != SceneStorageType::SSBO) {
        return;
    }

    const uint32_t sphereBufferSize = sizeof(rt::internal::Sphere) * m_shaderParams.maxSphereCount;
    const uint32_t triangleBufferSize = sizeof(rt::internal::Triangle) * m_shaderParams.maxTriangleCount;
    m_sceneSpheresBuffer = rlLoadShaderBuffer(sphereBufferSize, nullptr, RL_DYNAMIC_COPY);
    m_sceneTrianglesBuffer = rlLoadShaderBuffer(triangleBufferSize, nullptr, RL_DYNAMIC_COPY);

    if (m_sceneSpheresBuffer != 0) {
        TRACE("Created buffer for scene-spheres of size = %u bytes [ID: %u]", sphereBufferSize, m_sceneSpheresBuffer);
    }
    if (m_sceneTrianglesBuffer != 0) {
        TRACE("Created buffer for scene-triangles of size = %u bytes [ID: %u]", triangleBufferSize, m_sceneTrianglesBuffer);
    }

    if (m_sceneSpheresBuffer && m_sceneTrianglesBuffer) {
        INFO("Created buffers for scene's sphere and triangles successfully [ID: %u %u]", m_sceneSpheresBuffer, m_sceneTrianglesBuffer);
    }
}


char* Raytracer::loadComputeShaderContents() {
    const char* shaderPath = "shaders/raytracer.glsl";
    char* fileContents = LoadFileText(shaderPath);
    if (fileContents == nullptr) {
        TRACE("'%s' loaded successfully", fileContents);
    }

    // find and replace utility function
    auto replaceFn = [&](const char* replaceStr, const char* byStr) {
        char* temp = TextReplace(fileContents, replaceStr, byStr);
        UnloadFileText(fileContents);
        fileContents = temp;
    };

    replaceFn("WG_SIZE", TextFormat("%u", m_shaderParams.workgroupSize));
    replaceFn("MAX_SPHERE_COUNT", TextFormat("%u", m_shaderParams.maxSphereCount));
    replaceFn("MAX_TRIANGLE_COUNT", TextFormat("%u", m_shaderParams.maxTriangleCount));

    const int usingUniform = m_shaderParams.storageType == SceneStorageType::UBO;
    replaceFn("USE_UNIFORM_OBJECTS", TextFormat("%d", usingUniform));

    return fileContents;
}


void Raytracer::compileComputeShader() {
    char* fileContents = loadComputeShaderContents();

    INFO("Compiling compute shader with:");
    INFO("    Workgroup Size: %u", m_shaderParams.workgroupSize);
    INFO("    Buffer Type: %s", m_shaderParams.storageType == SceneStorageType::UBO ? "UBO" : "SSBO");
    INFO("    Max Sphere Count: %u", m_shaderParams.maxSphereCount);
    INFO("    Max Triangle Count: %u", m_shaderParams.maxTriangleCount);

    const uint32_t shaderId = rlCompileShader(fileContents, RL_COMPUTE_SHADER);
    m_computeShaderProgram = rlLoadComputeShaderProgram(shaderId);
    if (m_computeShaderProgram != 0) {
        TRACE("Loaded compute shader program successfully [ID: %u]", m_computeShaderProgram);
    }

    UnloadFileText(fileContents);
}


void Raytracer::runComputeShader() {
    m_frameIndex++;
    static int frameIndex_uniLoc = getUniLoc("frameIndex");

    rlEnableShader(m_computeShaderProgram);

    rlSetUniform(frameIndex_uniLoc, &m_frameIndex, RL_SHADER_UNIFORM_INT, 1);
    rlBindImageTexture(m_outTexture.id, 0, m_outTexture.format, false);
    rlBindShaderBuffer(m_sceneSpheresBuffer, 2);
    rlBindShaderBuffer(m_sceneTrianglesBuffer, 3);

    const int groupX = m_textureSize.x / m_shaderParams.workgroupSize;
    const int groupY = m_textureSize.y / m_shaderParams.workgroupSize;
    rlComputeShaderDispatch(groupX, groupY, 1);
}


void Raytracer::reset() {
    m_frameIndex = 0;

    static Image img = GenImageColor(m_textureSize.x, m_textureSize.y, RED);
    ImageFormat(&img, m_outTexture.format);
    UpdateTexture(m_outTexture, img.data);
}


void Raytracer::setScene_materials(const rt::CompiledScene& scene) {
    TRACE("    Setting materialData [ID: %u]:", scene.m_materialData->getId());

    const int numMaterials_uniLoc = getUniLoc("numMaterials");
    const float numMaterials = scene.m_materialData->getMaterialCount();
    rlSetUniform(numMaterials_uniLoc, &numMaterials, RL_SHADER_UNIFORM_FLOAT, 1);
    TRACE("        numMaterials = %d", (int) numMaterials);

    const int materialTexture_uniLoc = getUniLoc("materialTexture");
    const int materialTexture = scene.m_materialData->getTextureId();
    rlSetUniformSampler(materialTexture_uniLoc, materialTexture);
    TRACE("        materialTextureId = %d", materialTexture);
}


void Raytracer::setScene_spheres(const rt::CompiledScene& scene) {
    const uint32_t numSpheres = std::min((uint32_t) scene.m_spheres.size(), m_shaderParams.maxSphereCount);

    const int numSpheres_uniLoc = getUniLoc("sceneInfo.numSpheres");
    rlSetUniform(numSpheres_uniLoc, &numSpheres, RL_SHADER_UNIFORM_INT, 1);
    TRACE("    Number of spheres: %u", numSpheres);

    if (m_shaderParams.storageType == SceneStorageType::UBO) {
        TRACE("    Setting scene-spheres UBO", numSpheres);

        for (int i = 0; i < numSpheres; i++) {
            const rt::internal::Sphere& obj = scene.m_spheres[i];
            const char* base = TextFormat("sceneSpheres.data[%d]", i);
            // copying the base, since textformat allocates internally
            char baseCopy[1024];
            TextCopy(baseCopy, base);

            const int position_uniLoc = getUniLoc("%s.position", baseCopy);
            const int radius_uniLoc = getUniLoc("%s.radius", baseCopy);
            const int materialIndex_uniLoc = getUniLoc("%s.materialIndex", baseCopy);

            // 16 bytes
            rlSetUniform(position_uniLoc, &obj.position, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(radius_uniLoc, &obj.radius, RL_SHADER_UNIFORM_FLOAT, 1);
            // 4 bytes (padded to 16 bytes)
            rlSetUniform(materialIndex_uniLoc, &obj.materialIndex, RL_SHADER_UNIFORM_FLOAT, 1);
        }

    } else {

        const uint32_t bufferSize = sizeof(rt::internal::Sphere) * numSpheres;
        rlUpdateShaderBuffer(m_sceneSpheresBuffer, scene.m_spheres.data(), bufferSize, 0);
        TRACE("    Setting scene-spheres SSBO[ID: %u] (buffer-size: %f KB)", m_sceneSpheresBuffer, numSpheres, bufferSize / 1024.0f);
    }
}


void Raytracer::setScene_triangles(const rt::CompiledScene& scene) {
    const uint32_t numTriangles = std::min((uint32_t) scene.m_triangles.size(), m_shaderParams.maxTriangleCount);

    const int numTriangles_uniLoc = getUniLoc("sceneInfo.numTriangles");
    rlSetUniform(numTriangles_uniLoc, &numTriangles, RL_SHADER_UNIFORM_INT, 1);
    TRACE("    Number of triangles: %u", numTriangles);

    if (m_shaderParams.storageType == SceneStorageType::UBO) {
        TRACE("    Setting scene-triangles UBO", numTriangles);

        for (int i = 0; i < numTriangles; i++) {
            const rt::internal::Triangle& obj = scene.m_triangles[i];
            const char* base = TextFormat("sceneTriangles.data[%d]", i);
            // copying the base, since textformat allocates internally
            char baseCopy[1024];
            TextCopy(baseCopy, base);

            const int v0_uniLoc = getUniLoc("%s.v0", baseCopy);
            const int v1_uniLoc = getUniLoc("%s.v1", baseCopy);
            const int v2_uniLoc = getUniLoc("%s.v2", baseCopy);
            const int uv0_uniLoc = getUniLoc("%s.uv0", baseCopy);
            const int uv1_uniLoc = getUniLoc("%s.uv1", baseCopy);
            const int uv2_uniLoc = getUniLoc("%s.uv2", baseCopy);
            const int materialIndex_uniLoc = getUniLoc("%s.materialIndex", baseCopy);

            // 12 bytes (padded to 16 bytes)
            rlSetUniform(v0_uniLoc, &obj.v0, RL_SHADER_UNIFORM_VEC3, 1);
            // 12 bytes (padded to 16 bytes)
            rlSetUniform(v1_uniLoc, &obj.v1, RL_SHADER_UNIFORM_VEC3, 1);
            // 12 bytes (padded to 16 bytes)
            rlSetUniform(v2_uniLoc, &obj.v2, RL_SHADER_UNIFORM_VEC3, 1);
            // 16 bytes
            rlSetUniform(uv0_uniLoc, &obj.uv0, RL_SHADER_UNIFORM_VEC2, 1);
            rlSetUniform(uv1_uniLoc, &obj.uv1, RL_SHADER_UNIFORM_VEC2, 1);
            // 12 bytes (padded to 16 bytes)
            rlSetUniform(uv2_uniLoc, &obj.uv2, RL_SHADER_UNIFORM_VEC2, 1);
            rlSetUniform(materialIndex_uniLoc, &obj.materialIndex, RL_SHADER_UNIFORM_FLOAT, 1);
        }

    } else {

        const uint32_t bufferSize = sizeof(rt::internal::Triangle) * numTriangles;
        rlUpdateShaderBuffer(m_sceneTrianglesBuffer, scene.m_triangles.data(), bufferSize, 0);
        TRACE("    Setting scene-triangles SSBO[ID: %u] (buffer-size: %f KB)", m_sceneTrianglesBuffer, numTriangles, bufferSize / 1024.0f);
    }
}
