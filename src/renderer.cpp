
#include "src/renderer.h"
#include <raylib/rlgl.h>


Renderer::Renderer(Vector2 windowSize, Vector2 imageSize)
    : m_windowSize(windowSize), m_imageSize(imageSize) {
    InitWindow(m_windowSize.x, m_windowSize.y, "Raytracing");
    SetTargetFPS(30);

    makeOutImage();
}


Renderer::~Renderer() {
    rlUnloadShaderProgram(m_computeShaderProgram);
    UnloadTexture(m_outImage);
    CloseWindow();
}


void Renderer::render(bool compute, bool draw) {
    if (compute) {
        runComputeShader();
    }
    if (draw) {
        drawOutImage();
    }
}


void Renderer::compileComputeShader(CompileShaderParams _params) {
    m_compileParams = _params;

    makeBufferObjects();

    char* fileText = LoadFileText("shaders/raytracer.glsl");

    auto replaceFn = [&](const char* replaceStr, int number) {
        const char* byStr = TextFormat("%d", number);
        char* temp = TextReplace(fileText, replaceStr, byStr);
        UnloadFileText(fileText);
        fileText = temp;
    };

    replaceFn("WG_SIZE", m_compileParams.workgroupSize);
    replaceFn("MAX_SPHERE_COUNT", std::max(1u, m_compileParams.maxSphereCount));
    replaceFn("MAX_TRIANGLE_COUNT", std::max(1u, m_compileParams.maxTriangleCount));

    if (m_compileParams.storageType == SceneStorageType::Buffers) {
        replaceFn("USE_UNIFORM_OBJECTS", 0);
    } else {
        replaceFn("USE_UNIFORM_OBJECTS", 1);
    }

    const unsigned shaderId = rlCompileShader(fileText, RL_COMPUTE_SHADER);
    m_computeShaderProgram = rlLoadComputeShaderProgram(shaderId);

    UnloadFileText(fileText);
}


void Renderer::resetImage() {
    static Image img = GenImageColor(m_imageSize.x, m_imageSize.y, BLANK);
    ImageFormat(&img, m_outImage.format);
    UpdateTexture(m_outImage, img.data);
    m_frameIndex = 0;
}


void Renderer::setCamera(const SceneCamera& camera) const {
    const rt::Camera& iCamera = camera.get();

    rlEnableShader(m_computeShaderProgram);

    const int uniLoc_invViewMat = getUniformLoc("camera.invViewMat");
    const int uniLoc_invProjMat = getUniformLoc("camera.invProjMat");
    const int uniLoc_position = getUniformLoc("camera.position");

    rlSetUniformMatrix(uniLoc_invViewMat, iCamera.invViewMat);
    rlSetUniformMatrix(uniLoc_invProjMat, iCamera.invProjMat);
    rlSetUniform(uniLoc_position, &iCamera.position, RL_SHADER_UNIFORM_VEC3, 1);
}


void Renderer::setScene(const rt::CompiledScene& scene) const {
    rlEnableShader(m_computeShaderProgram);

    const int uniLoc_materialTexture = getUniformLoc("materialTexture");
    rlSetUniformSampler(uniLoc_materialTexture, scene.m_materialData->getTextureId());

    const int uniLoc_numMaterials = getUniformLoc("numMaterials");
    float numMaterials = scene.m_materialData->getMaterialCount();
    rlSetUniform(uniLoc_numMaterials, &numMaterials, RL_SHADER_UNIFORM_FLOAT, 1);

    setScene_spheres(scene);
    setScene_triangles(scene);

    const int uniLoc_backgroundColor = getUniformLoc("sceneInfo.backgroundColor");
    const int uniLoc_numSpheres = getUniformLoc("sceneInfo.numSpheres");

    rlSetUniform(uniLoc_backgroundColor, &scene.m_backgroundColor, RL_SHADER_UNIFORM_VEC3, 1);
}


void Renderer::setConfig(const rt::Config& config) const {
    rlEnableShader(m_computeShaderProgram);

    const int uniLoc_bounceLimit = getUniformLoc("config.bounceLimit");
    const int uniLoc_numSamples = getUniformLoc("config.numSamples");

    rlSetUniform(uniLoc_bounceLimit, &config.bounceLimit, RL_SHADER_UNIFORM_FLOAT, 1);
    rlSetUniform(uniLoc_numSamples, &config.numSamples, RL_SHADER_UNIFORM_FLOAT, 1);
}


template <class... Args> int Renderer::getUniformLoc(const char* fmt, Args... args) const {
    const char* uniformName = TextFormat(fmt, args...);
    return rlGetLocationUniform(m_computeShaderProgram, uniformName);
}


void Renderer::runComputeShader() {
    m_frameIndex++;
    static int uniLoc_frameIndex = getUniformLoc("frameIndex");

    const int groupX = m_imageSize.x / m_compileParams.workgroupSize;
    const int groupY = m_imageSize.y / m_compileParams.workgroupSize;

    rlEnableShader(m_computeShaderProgram);

    rlSetUniform(uniLoc_frameIndex, &m_frameIndex, RL_SHADER_UNIFORM_INT, 1);
    rlBindImageTexture(m_outImage.id, 0, m_outImage.format, false);
    rlBindShaderBuffer(m_sceneSpheresBuffer, 2);
    rlBindShaderBuffer(m_sceneTrianglesBuffer, 3);

    rlComputeShaderDispatch(groupX, groupY, 1);
}


void Renderer::drawOutImage() const {
    BeginDrawing();
    ClearBackground(GREEN);

    DrawTexturePro(m_outImage, {0, 0, m_imageSize.x, m_imageSize.y},
                   {0, 0, m_windowSize.x, m_windowSize.y}, {0, 0}, 0, WHITE);

    DrawFPS(10, 10);
    DrawText(TextFormat("Frame Time: %.5f", GetFrameTime()), 10, 30, 20, DARKBLUE);
    DrawText(TextFormat("Frame Index: %d", m_frameIndex), 10, 50, 20, DARKBLUE);

    EndDrawing();
}


void Renderer::makeOutImage() {
    Image image = GenImageColor(m_imageSize.x, m_imageSize.y, BLUE);
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16);
    m_outImage = LoadTextureFromImage(image);
    UnloadImage(image);
}


void Renderer::makeBufferObjects() {
    const int totalObjects = m_compileParams.maxSphereCount + m_compileParams.maxTriangleCount;

    if (m_compileParams.storageType == SceneStorageType::Buffers) {
        m_sceneSpheresBuffer = rlLoadShaderBuffer(
            sizeof(rt::Sphere) * m_compileParams.maxSphereCount, nullptr, RL_DYNAMIC_COPY);
        m_sceneTrianglesBuffer = rlLoadShaderBuffer(
            sizeof(rt::Triangle) * m_compileParams.maxTriangleCount, nullptr, RL_DYNAMIC_COPY);
    }
}


void Renderer::setScene_spheres(const rt::CompiledScene& scene) const {
    unsigned numSpheres = scene.m_spheres.size();

    if (m_compileParams.storageType == SceneStorageType::Uniforms) {
        numSpheres = std::min(numSpheres, m_compileParams.maxSphereCount);

        for (int i = 0; i < numSpheres; i++) {
            const rt::internal::Sphere& obj = scene.m_spheres[i];
            const char* base = TextFormat("sceneSpheres.data[%d]", i);
            char copy[1024];
            TextCopy(copy, base);

            const int uniLoc_position = getUniformLoc("%s.position", copy);
            const int uniLoc_radius = getUniformLoc("%s.radius", copy);
            const int uniLoc_materialIndex = getUniformLoc("%s.materialIndex", copy);

            // 16 bytes
            rlSetUniform(uniLoc_position, &obj.position, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(uniLoc_radius, &obj.radius, RL_SHADER_UNIFORM_FLOAT, 1);
            // 4 -> 16 bytes
            rlSetUniform(uniLoc_materialIndex, &obj.materialIndex, RL_SHADER_UNIFORM_FLOAT, 1);
        }

    } else {

        const unsigned sphereBufferSize = sizeof(rt::Sphere) * numSpheres;
        rlUpdateShaderBuffer(m_sceneSpheresBuffer, scene.m_spheres.data(), sphereBufferSize, 0);
    }

    const int uniLoc_numSpheres = getUniformLoc("sceneInfo.numSpheres");
    rlSetUniform(uniLoc_numSpheres, &numSpheres, RL_SHADER_UNIFORM_INT, 1);
}


void Renderer::setScene_triangles(const rt::CompiledScene& scene) const {
    unsigned numTriangles = scene.m_triangles.size();

    if (m_compileParams.storageType == SceneStorageType::Uniforms) {
        numTriangles = std::min(numTriangles, m_compileParams.maxTriangleCount);

        for (int i = 0; i < numTriangles; i++) {
            const rt::internal::Triangle& obj = scene.m_triangles[i];
            const char* base = TextFormat("sceneTriangles.data[%d]", i);
            char copy[1024];
            TextCopy(copy, base);

            const int uniLoc_v0 = getUniformLoc("%s.v0", copy);
            const int uniLoc_v1 = getUniformLoc("%s.v1", copy);
            const int uniLoc_v2 = getUniformLoc("%s.v2", copy);
            const int uniLoc_uv0 = getUniformLoc("%s.uv0", copy);
            const int uniLoc_uv1 = getUniformLoc("%s.uv1", copy);
            const int uniLoc_uv2 = getUniformLoc("%s.uv2", copy);
            const int uniLoc_materialIndex = getUniformLoc("%s.materialIndex", copy);

            // 12 -> 16 bytes
            rlSetUniform(uniLoc_v0, &obj.v0, RL_SHADER_UNIFORM_VEC3, 1);
            // 12 -> 16 bytes
            rlSetUniform(uniLoc_v1, &obj.v1, RL_SHADER_UNIFORM_VEC3, 1);
            // 12 -> 16 bytes
            rlSetUniform(uniLoc_v2, &obj.v2, RL_SHADER_UNIFORM_VEC3, 1);
            // 16 bytes
            rlSetUniform(uniLoc_uv0, &obj.uv0, RL_SHADER_UNIFORM_VEC2, 1);
            rlSetUniform(uniLoc_uv1, &obj.uv1, RL_SHADER_UNIFORM_VEC2, 1);
            // 16 bytes
            rlSetUniform(uniLoc_uv2, &obj.uv2, RL_SHADER_UNIFORM_VEC2, 1);
            rlSetUniform(uniLoc_materialIndex, &obj.materialIndex, RL_SHADER_UNIFORM_FLOAT, 1);
        }

    } else {

        const unsigned triangleBufferSize = sizeof(rt::Triangle) * numTriangles;
        rlUpdateShaderBuffer(m_sceneTrianglesBuffer, scene.m_triangles.data(), triangleBufferSize,
                             0);
    }

    const int uniLoc_numTriangles = getUniformLoc("sceneInfo.numTriangles");
    rlSetUniform(uniLoc_numTriangles, &numTriangles, RL_SHADER_UNIFORM_INT, 1);
}
