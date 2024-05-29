
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
    m_sceneObjectsBuffer = rlLoadShaderBuffer(100 * 1024, nullptr, RL_DYNAMIC_COPY);
}


void Renderer::compileComputeShader(unsigned computeLocalSize, unsigned maxSphereCount, bool useBuffers) {
    m_computeLocalSize = computeLocalSize;
    m_maxSphereCount = maxSphereCount;

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
    replaceFn("MAX_SPHERE_COUNT", TextFormat("%d", m_maxSphereCount));

    if (m_sceneObjectsBuffer == 0) {
        replaceFn("USE_UNIFORM_OBJECTS", TextFormat("%d", 1));
    } else {
        replaceFn("USE_UNIFORM_OBJECTS", TextFormat("%d", 0));
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
    rlBindShaderBuffer(m_sceneObjectsBuffer, 1);
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
    if (m_sceneObjectsBuffer == 0) {
        numSpheres = std::min(numSpheres, m_maxSphereCount);
    }

    if (m_sceneObjectsBuffer == 0) {

        for (int i = 0; i < numSpheres; i++) {
            unsigned shaderLoc_pos = getUniformLoc(TextFormat("sceneObjects.spheres[%d].position", i));
            unsigned shaderLoc_radius = getUniformLoc(TextFormat("sceneObjects.spheres[%d].radius", i));
            unsigned shaderLoc_color = getUniformLoc(TextFormat("sceneObjects.spheres[%d].color", i));

            rlSetUniform(shaderLoc_pos, &scene.spheres[i].position, RL_SHADER_UNIFORM_VEC3, 1);
            rlSetUniform(shaderLoc_radius, &scene.spheres[i].radius, RL_SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(shaderLoc_color, &scene.spheres[i].color, RL_SHADER_UNIFORM_VEC3, 1);
        }

    } else {

        int sceneObjectsSize = sizeof(rt::Sphere) * numSpheres;
        rlUpdateShaderBuffer(m_sceneObjectsBuffer, scene.spheres.data(), sceneObjectsSize, 0);

    }

    unsigned shaderLoc_backgroundColor = getUniformLoc("sceneInfo.backgroundColor");
    unsigned shaderLoc_numSpheres = getUniformLoc("sceneInfo.numSpheres");

    Vector4 backgroundColorVec = ColorNormalize(scene.backgroundColor);

    rlSetUniform(shaderLoc_backgroundColor, &backgroundColorVec, RL_SHADER_UNIFORM_VEC3, 1);
    rlSetUniform(shaderLoc_numSpheres, &numSpheres, RL_SHADER_UNIFORM_INT, 1);
}


void Renderer::setCurrentConfig(const rt::Config& config) {
    rlEnableShader(m_computeShaderProgram);
    m_hasConfig = true;

    unsigned shaderLoc_bounceLimit = getUniformLoc("config.bounceLimit");
    unsigned shaderLoc_numSamples = getUniformLoc("config.numSamples");

    rlSetUniform(shaderLoc_bounceLimit, &config.bounceLimit, RL_SHADER_UNIFORM_FLOAT, 1);
    rlSetUniform(shaderLoc_numSamples, &config.numSamples, RL_SHADER_UNIFORM_FLOAT, 1);
}
