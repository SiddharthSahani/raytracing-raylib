
#include "src/renderer.h"
#include <raylib/rlgl.h>


Renderer::Renderer(Vector2 windowSize, Vector2 imageSize, unsigned computeLocalSize,
                   unsigned maxSphereCount)
    : m_windowSize(windowSize), m_imageSize(imageSize), m_computeLocalSize(computeLocalSize),
      m_maxSphereCount(maxSphereCount) {
    InitWindow(m_windowSize.x, m_windowSize.y, "Raytracing");
    SetTargetFPS(30);

    makeImage();
    makeBufferObjects();
    compileComputeShader();
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


void Renderer::makeImage() {
    Image image = GenImageColor(m_imageSize.x, m_imageSize.y, BLUE);
    ImageFormat(&image, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    m_outImage = LoadTextureFromImage(image);
    UnloadImage(image);
}


void Renderer::makeBufferObjects() {}


void Renderer::compileComputeShader() {
    char* fileText = LoadFileText("shaders/raytracer.glsl");

    auto replaceFn = [&](const char* replaceStr, const char* byStr) {
        char* temp = TextReplace(fileText, replaceStr, byStr);
        UnloadFileText(fileText);
        fileText = temp;
    };

    replaceFn("WG_SIZE_PLACEHOLDER", TextFormat("%d", m_computeLocalSize));
    replaceFn("MAX_SPHERE_COUNT_PLACEHOLDER", TextFormat("%d", m_computeLocalSize));

    unsigned shaderId = rlCompileShader(fileText, RL_COMPUTE_SHADER);
    m_computeShaderProgram = rlLoadComputeShaderProgram(shaderId);

    UnloadFileText(fileText);
}


void Renderer::runComputeShader() {
    if (!canRender()) {
        return;
    }

    static unsigned shaderLoc_frameIndex =
        rlGetLocationUniform(m_computeShaderProgram, "frameIndex");
    static int frameIndex = 0;
    frameIndex++;

    const int groupX = m_imageSize.x / m_computeLocalSize;
    const int groupY = m_imageSize.y / m_computeLocalSize;

    rlEnableShader(m_computeShaderProgram);
    rlSetUniform(shaderLoc_frameIndex, &frameIndex, RL_SHADER_UNIFORM_INT, 1);
    rlBindImageTexture(m_outImage.id, 0, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, false);
    rlComputeShaderDispatch(groupX, groupY, 1);
}


void Renderer::setCurrentCamera(const rt::Camera& camera) {
    rlEnableShader(m_computeShaderProgram);
    m_hasCamera = true;

    unsigned shaderLoc_position = rlGetLocationUniform(m_computeShaderProgram, "camera.position");
    unsigned shaderLoc_direction = rlGetLocationUniform(m_computeShaderProgram, "camera.direction");

    rlSetUniform(shaderLoc_position, &camera.position, RL_SHADER_UNIFORM_VEC3, 1);
    rlSetUniform(shaderLoc_direction, &camera.direction, RL_SHADER_UNIFORM_VEC3, 1);
}


void Renderer::setCurrentScene(const rt::Scene& scene) {
    rlEnableShader(m_computeShaderProgram);
    m_hasScene = true;

    int n = std::min((unsigned)scene.spheres.size(), m_maxSphereCount);

    for (int i = 0; i < n; i++) {
        unsigned shaderLoc_position = rlGetLocationUniform(
            m_computeShaderProgram, TextFormat("scene.spheres[%d].position", i));
        unsigned shaderLoc_radius =
            rlGetLocationUniform(m_computeShaderProgram, TextFormat("scene.spheres[%d].radius", i));
        unsigned shaderLoc_color =
            rlGetLocationUniform(m_computeShaderProgram, TextFormat("scene.spheres[%d].color", i));

        Vector4 colorVec = ColorNormalize(scene.spheres[i].color);

        rlSetUniform(shaderLoc_position, &scene.spheres[i].position, RL_SHADER_UNIFORM_VEC3, 1);
        rlSetUniform(shaderLoc_radius, &scene.spheres[i].radius, RL_SHADER_UNIFORM_FLOAT, 1);
        rlSetUniform(shaderLoc_color, &colorVec, RL_SHADER_UNIFORM_VEC3, 1);
    }

    unsigned shaderLoc_backgroundColor =
        rlGetLocationUniform(m_computeShaderProgram, "scene.backgroundColor");
    unsigned shaderLoc_numSpheres =
        rlGetLocationUniform(m_computeShaderProgram, "scene.numSpheres");

    Vector4 backgroundColorVec = ColorNormalize(scene.backgroundColor);

    rlSetUniform(shaderLoc_backgroundColor, &backgroundColorVec, RL_SHADER_UNIFORM_VEC3, 1);
    rlSetUniform(shaderLoc_numSpheres, &n, RL_SHADER_UNIFORM_INT, 1);
}


void Renderer::setCurrentConfig(const rt::Config& config) {
    rlEnableShader(m_computeShaderProgram);
    m_hasConfig = true;

    unsigned shaderLoc_bounceLimit =
        rlGetLocationUniform(m_computeShaderProgram, "config.bounceLimit");
    unsigned shaderLoc_numSamples =
        rlGetLocationUniform(m_computeShaderProgram, "config.numSamples");

    rlSetUniform(shaderLoc_bounceLimit, &config.bounceLimit, RL_SHADER_UNIFORM_FLOAT, 1);
    rlSetUniform(shaderLoc_numSamples, &config.numSamples, RL_SHADER_UNIFORM_FLOAT, 1);
}
