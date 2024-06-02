
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


void Renderer::draw() const {
    BeginDrawing();

    DrawTexturePro(m_outImage, {0, 0, m_imageSize.x, m_imageSize.y},
                   {0, 0, m_windowSize.x, m_windowSize.y}, {0, 0}, 0, WHITE);

    DrawFPS(10, 10);
    DrawText(TextFormat("Frame Time: %.5f", GetFrameTime()), 10, 30, 20, DARKBLUE);
    EndDrawing();
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
    replaceFn("MAX_PLANE_COUNT", std::max(1u, m_compileParams.maxPlaneCount));
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


void Renderer::runComputeShader() {
    if (!canRender()) {
        return;
    }

    static int uniLoc_frameIndex = getUniformLoc("frameIndex");
    static int frameIndex = 0;
    frameIndex++;

    const int groupX = m_imageSize.x / m_compileParams.workgroupSize;
    const int groupY = m_imageSize.y / m_compileParams.workgroupSize;

    rlEnableShader(m_computeShaderProgram);

    rlSetUniform(uniLoc_frameIndex, &frameIndex, RL_SHADER_UNIFORM_INT, 1);
    rlBindImageTexture(m_outImage.id, 0, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, false);
    rlBindShaderBuffer(m_sceneMaterialsBuffer, 1);
    rlBindShaderBuffer(m_sceneSpheresBuffer, 2);
    rlBindShaderBuffer(m_scenePlanesBuffer, 3);
    rlBindShaderBuffer(m_sceneTrianglesBuffer, 4);

    rlComputeShaderDispatch(groupX, groupY, 1);
}


bool Renderer::canRender() const {
    return m_computeShaderProgram && m_hasCamera && m_hasScene && m_hasConfig;
}


void Renderer::setCurrentCamera(const rt::Camera& camera) {
    rlEnableShader(m_computeShaderProgram);
    m_hasCamera = true;

    const int uniLoc_position = getUniformLoc("camera.position");
    const int uniLoc_direction = getUniformLoc("camera.direction");

    rlSetUniform(uniLoc_position, &camera.position, RL_SHADER_UNIFORM_VEC3, 1);
    rlSetUniform(uniLoc_direction, &camera.direction, RL_SHADER_UNIFORM_VEC3, 1);
}


void Renderer::setCurrentScene(const rt::Scene& scene) {
    rlEnableShader(m_computeShaderProgram);
    m_hasScene = true;

    const int materialBufferSize = sizeof(rt::Material) * scene.materials.size();
    rlUpdateShaderBuffer(m_sceneMaterialsBuffer, scene.materials.data(), materialBufferSize, 0);

    setScene_spheres(scene);
    setScene_planes(scene);
    setScene_triangles(scene);

    const int uniLoc_backgroundColor = getUniformLoc("sceneInfo.backgroundColor");
    const int uniLoc_numSpheres = getUniformLoc("sceneInfo.numSpheres");
    const int uniLoc_numPlanes = getUniformLoc("sceneInfo.numPlanes");

    const Vector4 backgroundColorVec = ColorNormalize(scene.backgroundColor);
    rlSetUniform(uniLoc_backgroundColor, &backgroundColorVec, RL_SHADER_UNIFORM_VEC3, 1);
}


void Renderer::setCurrentConfig(const rt::Config& config) {
    rlEnableShader(m_computeShaderProgram);
    m_hasConfig = true;

    const int uniLoc_bounceLimit = getUniformLoc("config.bounceLimit");
    const int uniLoc_numSamples = getUniformLoc("config.numSamples");

    rlSetUniform(uniLoc_bounceLimit, &config.bounceLimit, RL_SHADER_UNIFORM_FLOAT, 1);
    rlSetUniform(uniLoc_numSamples, &config.numSamples, RL_SHADER_UNIFORM_FLOAT, 1);
}


template <class... Args> int Renderer::getUniformLoc(const char* fmt, Args... args) const {
    const char* uniformName = TextFormat(fmt, args...);
    return rlGetLocationUniform(m_computeShaderProgram, uniformName);
}


void Renderer::makeOutImage() {
    Image image = GenImageColor(m_imageSize.x, m_imageSize.y, BLUE);
    ImageFormat(&image, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    m_outImage = LoadTextureFromImage(image);
    UnloadImage(image);
}


void Renderer::makeBufferObjects() {
    const int totalObjects = m_compileParams.maxSphereCount + m_compileParams.maxPlaneCount +
                             m_compileParams.maxTriangleCount;
    m_sceneMaterialsBuffer =
        rlLoadShaderBuffer(sizeof(rt::Material) * totalObjects, nullptr, RL_DYNAMIC_COPY);

    if (m_compileParams.storageType == SceneStorageType::Buffers) {
        m_sceneSpheresBuffer = rlLoadShaderBuffer(
            sizeof(rt::Sphere) * m_compileParams.maxSphereCount, nullptr, RL_DYNAMIC_COPY);
        m_scenePlanesBuffer = rlLoadShaderBuffer(sizeof(rt::Plane) * m_compileParams.maxPlaneCount,
                                                 nullptr, RL_DYNAMIC_COPY);
        m_sceneTrianglesBuffer = rlLoadShaderBuffer(
            sizeof(rt::Triangle) * m_compileParams.maxTriangleCount, nullptr, RL_DYNAMIC_COPY);
    }
}


void Renderer::setScene_spheres(const rt::Scene& scene) {
    unsigned numSpheres = scene.spheres.size();

    if (m_compileParams.storageType == SceneStorageType::Uniforms) {
        numSpheres = std::min(numSpheres, m_compileParams.maxSphereCount);

        for (int i = 0; i < numSpheres; i++) {
            const rt::Sphere& obj = scene.spheres[i];
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
            rlSetUniform(uniLoc_materialIndex, &obj.materialIndex, RL_SHADER_UNIFORM_INT, 1);
        }

    } else {

        const unsigned sphereBufferSize = sizeof(rt::Sphere) * numSpheres;
        rlUpdateShaderBuffer(m_sceneSpheresBuffer, scene.spheres.data(), sphereBufferSize, 0);
    }

    const int uniLoc_numSpheres = getUniformLoc("sceneInfo.numSpheres");
    rlSetUniform(uniLoc_numSpheres, &numSpheres, RL_SHADER_UNIFORM_INT, 1);
}


void Renderer::setScene_planes(const rt::Scene& scene) {
    unsigned numPlanes = scene.planes.size();

    if (m_compileParams.storageType == SceneStorageType::Uniforms) {
        numPlanes = std::min(numPlanes, m_compileParams.maxPlaneCount);

        for (int i = 0; i < numPlanes; i++) {
            const rt::Plane& obj = scene.planes[i];
            const char* base = TextFormat("scenePlanes.data[%d]", i);
            char copy[1024];
            TextCopy(copy, base);

            const int uniLoc_center = getUniformLoc("%s.center", copy);
            const int uniLoc_materialIndex = getUniformLoc("%s.materialIndex", copy);
            const int uniLoc_uDirection = getUniformLoc("%s.uDirection", copy);
            const int uniLoc_uSize = getUniformLoc("%s.uSize", copy);
            const int uniLoc_vDirection = getUniformLoc("%s.vDirection", copy);
            const int uniLoc_vSize = getUniformLoc("%s.vSize", copy);

            // 16 bytes
            rlSetUniform(uniLoc_center, &obj.center, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(uniLoc_materialIndex, &obj.materialIndex, RL_SHADER_UNIFORM_INT, 1);
            // 16 bytes
            rlSetUniform(uniLoc_uDirection, &obj.uDirection, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(uniLoc_uSize, &obj.uSize, RL_SHADER_UNIFORM_FLOAT, 1);
            // 16 bytes
            rlSetUniform(uniLoc_vDirection, &obj.vDirection, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(uniLoc_vSize, &obj.vSize, RL_SHADER_UNIFORM_FLOAT, 1);
        }

    } else {

        const unsigned planeBufferSize = sizeof(rt::Plane) * numPlanes;
        rlUpdateShaderBuffer(m_scenePlanesBuffer, scene.planes.data(), planeBufferSize, 0);
    }

    const int uniLoc_numPlanes = getUniformLoc("sceneInfo.numPlanes");
    rlSetUniform(uniLoc_numPlanes, &numPlanes, RL_SHADER_UNIFORM_INT, 1);
}


void Renderer::setScene_triangles(const rt::Scene& scene) {
    unsigned numTriangles = scene.triangles.size();

    if (m_compileParams.storageType == SceneStorageType::Uniforms) {
        numTriangles = std::min(numTriangles, m_compileParams.maxTriangleCount);

        for (int i = 0; i < numTriangles; i++) {
            const rt::Triangle& obj = scene.triangles[i];
            const char* base = TextFormat("sceneTriangles.data[%d]", i);
            char copy[1024];
            TextCopy(copy, base);

            const int uniLoc_v0 = getUniformLoc("%s.v0", copy);
            const int uniLoc_v1 = getUniformLoc("%s.v1", copy);
            const int uniLoc_v2 = getUniformLoc("%s.v2", copy);
            const int uniLoc_materialIndex = getUniformLoc("%s.materialIndex", copy);

            // 12 -> 16 bytes
            rlSetUniform(uniLoc_v0, &obj.v0, RL_SHADER_UNIFORM_VEC3, 1);
            // 12 ->16 bytes
            rlSetUniform(uniLoc_v1, &obj.v1, RL_SHADER_UNIFORM_VEC3, 1);
            // 16 bytes
            rlSetUniform(uniLoc_v2, &obj.v2, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(uniLoc_materialIndex, &obj.materialIndex, RL_SHADER_UNIFORM_INT, 1);
        }

    } else {

        const unsigned triangleBufferSize = sizeof(rt::Triangle) * numTriangles;
        rlUpdateShaderBuffer(m_sceneTrianglesBuffer, scene.triangles.data(), triangleBufferSize, 0);
    }

    const int uniLoc_numTriangles = getUniformLoc("sceneInfo.numTriangles");
    rlSetUniform(uniLoc_numTriangles, &numTriangles, RL_SHADER_UNIFORM_INT, 1);
}
