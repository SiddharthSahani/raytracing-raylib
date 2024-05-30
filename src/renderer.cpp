
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
    m_sceneSpheresBuffer =
        rlLoadShaderBuffer(sizeof(rt::Sphere) * m_maxSphereCount, nullptr, RL_DYNAMIC_COPY);
    m_scenePlanesBuffer =
        rlLoadShaderBuffer(sizeof(rt::Plane) * m_maxPlaneCount, nullptr, RL_DYNAMIC_COPY);
}


void Renderer::compileComputeShader(unsigned computeLocalSize, unsigned maxSphereCount,
                                    unsigned maxPlaneCount, bool useBuffers) {
    m_computeLocalSize = computeLocalSize;
    m_maxSphereCount = maxSphereCount;
    m_maxPlaneCount = maxPlaneCount;
    m_usingBuffers = useBuffers;

    if (useBuffers) {
        makeBufferObjects();
    }

    char* fileText = LoadFileText("shaders/raytracer.glsl");

    auto replaceFn = [&](const char* replaceStr, const char* byStr) {
        char* temp = TextReplace(fileText, replaceStr, byStr);
        UnloadFileText(fileText);
        fileText = temp;
    };

    replaceFn("WG_SIZE", TextFormat("%d", m_computeLocalSize));
    replaceFn("MAX_SPHERE_COUNT", TextFormat("%d", std::max(1, m_maxSphereCount)));
    replaceFn("MAX_PLANE_COUNT", TextFormat("%d", std::max(1, m_maxPlaneCount)));

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
    rlBindShaderBuffer(m_sceneSpheresBuffer, 1);
    rlBindShaderBuffer(m_scenePlanesBuffer, 2);
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
    if (!m_usingBuffers) {
        numSpheres = std::min(numSpheres, m_maxSphereCount);
        numPlanes = std::min(numPlanes, m_maxPlaneCount);
    }

    if (!m_usingBuffers) {

        for (int i = 0; i < numSpheres; i++) {
            unsigned shaderLoc_pos = getUniformLoc(TextFormat("sceneSpheres.data[%d].position", i));
            unsigned shaderLoc_radius =
                getUniformLoc(TextFormat("sceneSpheres.data[%d].radius", i));
            unsigned shaderLoc_color = getUniformLoc(TextFormat("sceneSpheres.data[%d].color", i));
            unsigned shaderLoc_roughness = getUniformLoc(TextFormat("sceneSpheres.data[%d].roughness", i));

            rlSetUniform(shaderLoc_pos, &scene.spheres[i].position, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_radius, &scene.spheres[i].radius, RL_SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(shaderLoc_color, &scene.spheres[i].color, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_roughness, &scene.spheres[i].roughness, RL_SHADER_UNIFORM_FLOAT, 1);
        }

        for (int i = 0; i < numPlanes; i++) {
            unsigned shaderLoc_center = getUniformLoc(TextFormat("scenePlanes.data[%d].center", i));
            unsigned shaderLoc_uDir =
                getUniformLoc(TextFormat("scenePlanes.data[%d].uDirection", i));
            unsigned shaderLoc_uSize = getUniformLoc(TextFormat("scenePlanes.data[%d].uSize", i));
            unsigned shaderLoc_vDir =
                getUniformLoc(TextFormat("scenePlanes.data[%d].vDirection", i));
            unsigned shaderLoc_vSize = getUniformLoc(TextFormat("scenePlanes.data[%d].vSize", i));
            unsigned shaderLoc_color = getUniformLoc(TextFormat("scenePlanes.data[%d].color", i));
            unsigned shaderLoc_roughness = getUniformLoc(TextFormat("scenePlanes.data[%d].roughness", i));

            rlSetUniform(shaderLoc_center, &scene.planes[i].center, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_uDir, &scene.planes[i].uDirection, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_uSize, &scene.planes[i].uSize, RL_SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(shaderLoc_vDir, &scene.planes[i].vDirection, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_vSize, &scene.planes[i].vSize, RL_SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(shaderLoc_color, &scene.planes[i].color, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_roughness, &scene.planes[i].roughness, RL_SHADER_UNIFORM_FLOAT, 1);
        }

    } else {

        int sceneSpheresSize = sizeof(rt::Sphere) * numSpheres;
        int scenePlanesSize = sizeof(rt::Plane) * numPlanes;
        rlUpdateShaderBuffer(m_sceneSpheresBuffer, scene.spheres.data(), sceneSpheresSize, 0);
        rlUpdateShaderBuffer(m_scenePlanesBuffer, scene.planes.data(), scenePlanesSize, 0);
    }

    unsigned shaderLoc_backgroundColor = getUniformLoc("sceneInfo.backgroundColor");
    unsigned shaderLoc_numSpheres = getUniformLoc("sceneInfo.numSpheres");
    unsigned shaderLoc_numPlanes = getUniformLoc("sceneInfo.numPlanes");

    Vector4 backgroundColorVec = ColorNormalize(scene.backgroundColor);

    rlSetUniform(shaderLoc_backgroundColor, &backgroundColorVec, RL_SHADER_UNIFORM_VEC3, 1);
    rlSetUniform(shaderLoc_numSpheres, &numSpheres, RL_SHADER_UNIFORM_INT, 1);
    rlSetUniform(shaderLoc_numPlanes, &numPlanes, RL_SHADER_UNIFORM_INT, 1);
}


void Renderer::setCurrentConfig(const rt::Config& config) {
    rlEnableShader(m_computeShaderProgram);
    m_hasConfig = true;

    unsigned shaderLoc_bounceLimit = getUniformLoc("config.bounceLimit");
    unsigned shaderLoc_numSamples = getUniformLoc("config.numSamples");

    rlSetUniform(shaderLoc_bounceLimit, &config.bounceLimit, RL_SHADER_UNIFORM_FLOAT, 1);
    rlSetUniform(shaderLoc_numSamples, &config.numSamples, RL_SHADER_UNIFORM_FLOAT, 1);
}
