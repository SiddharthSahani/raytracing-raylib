
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


int Renderer::getUniformLoc(const char* uniformName) const {
    return rlGetLocationUniform(m_computeShaderProgram, uniformName);
}


void Renderer::makeOutImage() {
    Image image = GenImageColor(m_imageSize.x, m_imageSize.y, BLUE);
    ImageFormat(&image, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    m_outImage = LoadTextureFromImage(image);
    UnloadImage(image);
}


void Renderer::makeBufferObjects() {
    m_sceneMaterialsBuffer = rlLoadShaderBuffer(
        sizeof(rt::Material) * (m_maxSphereCount + m_maxPlaneCount + m_maxTriangleCount), nullptr,
        RL_DYNAMIC_COPY);

    if (m_usingBuffers) {
        m_sceneSpheresBuffer =
            rlLoadShaderBuffer(sizeof(rt::Sphere) * m_maxSphereCount, nullptr, RL_DYNAMIC_COPY);
        m_scenePlanesBuffer =
            rlLoadShaderBuffer(sizeof(rt::Plane) * m_maxPlaneCount, nullptr, RL_DYNAMIC_COPY);
        m_sceneTrianglesBuffer =
            rlLoadShaderBuffer(sizeof(rt::Triangle) * m_maxTriangleCount, nullptr, RL_DYNAMIC_COPY);
    }
}


void Renderer::compileComputeShader(unsigned computeLocalSize, unsigned maxSphereCount,
                                    unsigned maxPlaneCount, unsigned maxTriangleCount,
                                    bool useBuffers) {
    m_computeLocalSize = computeLocalSize;
    m_maxSphereCount = maxSphereCount;
    m_maxPlaneCount = maxPlaneCount;
    m_maxTriangleCount = maxTriangleCount;
    m_usingBuffers = useBuffers;

    makeBufferObjects();

    char* fileText = LoadFileText("shaders/raytracer.glsl");

    auto replaceFn = [&](const char* replaceStr, const char* byStr) {
        char* temp = TextReplace(fileText, replaceStr, byStr);
        UnloadFileText(fileText);
        fileText = temp;
    };

    replaceFn("WG_SIZE", TextFormat("%d", m_computeLocalSize));
    replaceFn("MAX_SPHERE_COUNT", TextFormat("%d", std::max(1, m_maxSphereCount)));
    replaceFn("MAX_PLANE_COUNT", TextFormat("%d", std::max(1, m_maxPlaneCount)));
    replaceFn("MAX_TRIANGLE_COUNT", TextFormat("%d", std::max(1, m_maxTriangleCount)));

    if (m_usingBuffers) {
        replaceFn("USE_UNIFORM_OBJECTS", TextFormat("%d", 0));
    } else {
        replaceFn("USE_UNIFORM_OBJECTS", TextFormat("%d", 1));
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

    const int groupX = m_imageSize.x / m_computeLocalSize;
    const int groupY = m_imageSize.y / m_computeLocalSize;

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

    int numSpheres = scene.spheres.size();
    int numPlanes = scene.planes.size();
    int numTriangles = scene.triangles.size();
    if (!m_usingBuffers) {
        numSpheres = std::min(numSpheres, m_maxSphereCount);
        numPlanes = std::min(numPlanes, m_maxPlaneCount);
        numTriangles = std::min(numTriangles, m_maxTriangleCount);
    }

    int sceneMaterialsSize = sizeof(rt::Material) * (numSpheres + numPlanes + numTriangles);
    rlUpdateShaderBuffer(m_sceneMaterialsBuffer, scene.materials.data(), sceneMaterialsSize, 0);

    if (!m_usingBuffers) {

        for (int i = 0; i < numSpheres; i++) {
            unsigned shaderLoc_pos = getUniformLoc(TextFormat("sceneSpheres.data[%d].position", i));
            unsigned shaderLoc_radius =
                getUniformLoc(TextFormat("sceneSpheres.data[%d].radius", i));
            unsigned shaderLoc_matIdx =
                getUniformLoc(TextFormat("sceneSpheres.data[%d].materialIndex", i));

            rlSetUniform(shaderLoc_pos, &scene.spheres[i].position, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_radius, &scene.spheres[i].radius, RL_SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(shaderLoc_matIdx, &scene.spheres[i].materialIndex, RL_SHADER_UNIFORM_INT,
                         1);
        }

        for (int i = 0; i < numPlanes; i++) {
            unsigned shaderLoc_center = getUniformLoc(TextFormat("scenePlanes.data[%d].center", i));
            unsigned shaderLoc_uDir =
                getUniformLoc(TextFormat("scenePlanes.data[%d].uDirection", i));
            unsigned shaderLoc_uSize = getUniformLoc(TextFormat("scenePlanes.data[%d].uSize", i));
            unsigned shaderLoc_vDir =
                getUniformLoc(TextFormat("scenePlanes.data[%d].vDirection", i));
            unsigned shaderLoc_vSize = getUniformLoc(TextFormat("scenePlanes.data[%d].vSize", i));
            unsigned shaderLoc_matIdx =
                getUniformLoc(TextFormat("scenePlanes.data[%d].materialIndex", i));

            rlSetUniform(shaderLoc_center, &scene.planes[i].center, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_uDir, &scene.planes[i].uDirection, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_uSize, &scene.planes[i].uSize, RL_SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(shaderLoc_vDir, &scene.planes[i].vDirection, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_vSize, &scene.planes[i].vSize, RL_SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(shaderLoc_matIdx, &scene.planes[i].materialIndex, RL_SHADER_UNIFORM_INT,
                         1);
        }

        for (int i = 0; i < numTriangles; i++) {
            unsigned shaderLoc_v0 = getUniformLoc(TextFormat("sceneTriangles.data[%d].v0", i));
            unsigned shaderLoc_v1 = getUniformLoc(TextFormat("sceneTriangles.data[%d].v1", i));
            unsigned shaderLoc_v2 = getUniformLoc(TextFormat("sceneTriangles.data[%d].v2", i));
            unsigned shaderLoc_matIdx =
                getUniformLoc(TextFormat("sceneTriangles.data[%d].materialIndex", i));

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
