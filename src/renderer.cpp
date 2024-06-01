
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


template <class... Args> int Renderer::getUniformLoc(const char* fmt, Args... args) const {
    const char* uniformName = TextFormat(fmt, args...);
    return rlGetLocationUniform(m_computeShaderProgram, uniformName);
}


bool Renderer::canRender() const {
    return m_computeShaderProgram && m_hasCamera && m_hasScene && m_hasConfig;
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

    unsigned shaderId = rlCompileShader(fileText, RL_COMPUTE_SHADER);
    m_computeShaderProgram = rlLoadComputeShaderProgram(shaderId);

    UnloadFileText(fileText);
}


void Renderer::runComputeShader() {
    if (!canRender()) {
        return;
    }

    static unsigned shaderLoc_frameIndex = getUniformLoc("frameIndex");
    static int frameIndex = 0;
    frameIndex++;

    const int groupX = m_imageSize.x / m_compileParams.workgroupSize;
    const int groupY = m_imageSize.y / m_compileParams.workgroupSize;

    rlEnableShader(m_computeShaderProgram);

    rlSetUniform(shaderLoc_frameIndex, &frameIndex, RL_SHADER_UNIFORM_INT, 1);
    rlBindImageTexture(m_outImage.id, 0, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, false);
    rlBindShaderBuffer(m_sceneMaterialsBuffer, 1);
    rlBindShaderBuffer(m_sceneSpheresBuffer, 2);
    rlBindShaderBuffer(m_scenePlanesBuffer, 3);
    rlBindShaderBuffer(m_sceneTrianglesBuffer, 4);

    rlComputeShaderDispatch(groupX, groupY, 1);
}


void Renderer::setCurrentCamera(const rt::Camera& camera) {
    rlEnableShader(m_computeShaderProgram);
    m_hasCamera = true;

    unsigned shaderLoc_position = getUniformLoc("camera.position");
    unsigned shaderLoc_direction = getUniformLoc("camera.direction");

    rlSetUniform(shaderLoc_position, &camera.position, RL_SHADER_UNIFORM_VEC3, 1);
    rlSetUniform(shaderLoc_direction, &camera.direction, RL_SHADER_UNIFORM_VEC3, 1);
}


void Renderer::setCurrentScene(const rt::Scene& scene) {
    rlEnableShader(m_computeShaderProgram);
    m_hasScene = true;

    unsigned numSpheres = scene.spheres.size();
    unsigned numPlanes = scene.planes.size();
    unsigned numTriangles = scene.triangles.size();
    if (m_compileParams.storageType == SceneStorageType::Uniforms) {
        numSpheres = std::min(numSpheres, m_compileParams.maxSphereCount);
        numPlanes = std::min(numPlanes, m_compileParams.maxPlaneCount);
        numTriangles = std::min(numTriangles, m_compileParams.maxTriangleCount);
    }

    int sceneMaterialsSize = sizeof(rt::Material) * (numSpheres + numPlanes + numTriangles);
    rlUpdateShaderBuffer(m_sceneMaterialsBuffer, scene.materials.data(), sceneMaterialsSize, 0);

    if (m_compileParams.storageType == SceneStorageType::Uniforms) {

        for (int i = 0; i < numSpheres; i++) {
            unsigned shaderLoc_pos = getUniformLoc("sceneSpheres.data[%d].position", i);
            unsigned shaderLoc_radius = getUniformLoc("sceneSpheres.data[%d].radius", i);
            unsigned shaderLoc_matIdx = getUniformLoc("sceneSpheres.data[%d].materialIndex", i);

            rlSetUniform(shaderLoc_pos, &scene.spheres[i].position, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_radius, &scene.spheres[i].radius, RL_SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(shaderLoc_matIdx, &scene.spheres[i].materialIndex, RL_SHADER_UNIFORM_INT,
                         1);
        }

        for (int i = 0; i < numPlanes; i++) {
            unsigned shaderLoc_center = getUniformLoc("scenePlanes.data[%d].center", i);
            unsigned shaderLoc_uDir = getUniformLoc("scenePlanes.data[%d].uDirection", i);
            unsigned shaderLoc_uSize = getUniformLoc("scenePlanes.data[%d].uSize", i);
            unsigned shaderLoc_vDir = getUniformLoc("scenePlanes.data[%d].vDirection", i);
            unsigned shaderLoc_vSize = getUniformLoc("scenePlanes.data[%d].vSize", i);
            unsigned shaderLoc_matIdx = getUniformLoc("scenePlanes.data[%d].materialIndex", i);

            rlSetUniform(shaderLoc_center, &scene.planes[i].center, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_uDir, &scene.planes[i].uDirection, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_uSize, &scene.planes[i].uSize, RL_SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(shaderLoc_vDir, &scene.planes[i].vDirection, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_vSize, &scene.planes[i].vSize, RL_SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(shaderLoc_matIdx, &scene.planes[i].materialIndex, RL_SHADER_UNIFORM_INT,
                         1);
        }

        for (int i = 0; i < numTriangles; i++) {
            unsigned shaderLoc_v0 = getUniformLoc("sceneTriangles.data[%d].v0", i);
            unsigned shaderLoc_v1 = getUniformLoc("sceneTriangles.data[%d].v1", i);
            unsigned shaderLoc_v2 = getUniformLoc("sceneTriangles.data[%d].v2", i);
            unsigned shaderLoc_matIdx = getUniformLoc("sceneTriangles.data[%d].materialIndex", i);

            rlSetUniform(shaderLoc_v0, &scene.triangles[i].v0, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_v1, &scene.triangles[i].v1, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_v2, &scene.triangles[i].v2, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_matIdx, &scene.triangles[i].materialIndex, RL_SHADER_UNIFORM_INT,
                         1);
        }

    } else {

        int sceneSpheresSize = sizeof(rt::Sphere) * numSpheres;
        int scenePlanesSize = sizeof(rt::Plane) * numPlanes;
        int sceneTrianglesSize = sizeof(rt::Triangle) * numTriangles;
        rlUpdateShaderBuffer(m_sceneSpheresBuffer, scene.spheres.data(), sceneSpheresSize, 0);
        rlUpdateShaderBuffer(m_scenePlanesBuffer, scene.planes.data(), scenePlanesSize, 0);
        rlUpdateShaderBuffer(m_sceneTrianglesBuffer, scene.triangles.data(), sceneTrianglesSize, 0);
    }

    unsigned shaderLoc_backgroundColor = getUniformLoc("sceneInfo.backgroundColor");
    unsigned shaderLoc_numSpheres = getUniformLoc("sceneInfo.numSpheres");
    unsigned shaderLoc_numPlanes = getUniformLoc("sceneInfo.numPlanes");
    unsigned shaderLoc_numTriangles = getUniformLoc("sceneInfo.numTriangles");

    Vector4 backgroundColorVec = ColorNormalize(scene.backgroundColor);

    rlSetUniform(shaderLoc_backgroundColor, &backgroundColorVec, RL_SHADER_UNIFORM_VEC3, 1);
    rlSetUniform(shaderLoc_numSpheres, &numSpheres, RL_SHADER_UNIFORM_INT, 1);
    rlSetUniform(shaderLoc_numPlanes, &numPlanes, RL_SHADER_UNIFORM_INT, 1);
    rlSetUniform(shaderLoc_numTriangles, &numTriangles, RL_SHADER_UNIFORM_INT, 1);
}


void Renderer::setCurrentConfig(const rt::Config& config) {
    rlEnableShader(m_computeShaderProgram);
    m_hasConfig = true;

    unsigned shaderLoc_bounceLimit = getUniformLoc("config.bounceLimit");
    unsigned shaderLoc_numSamples = getUniformLoc("config.numSamples");

    rlSetUniform(shaderLoc_bounceLimit, &config.bounceLimit, RL_SHADER_UNIFORM_FLOAT, 1);
    rlSetUniform(shaderLoc_numSamples, &config.numSamples, RL_SHADER_UNIFORM_FLOAT, 1);
}
