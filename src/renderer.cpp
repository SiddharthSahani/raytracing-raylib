
#include "src/renderer.h"
#include "src/logger.h"
#include <raylib/rlgl.h>


Renderer::Renderer(Vector2 windowSize, Vector2 imageSize)
    : m_windowSize(windowSize), m_imageSize(imageSize) {

    SetTraceLogLevel(LOG_WARNING);

    InitWindow(m_windowSize.x, m_windowSize.y, "Raytracing");
    INFO("Created window of size = %d x %d", (int)windowSize.x, (int)windowSize.y);

    SetTargetFPS(30);

    makeOutImage();
}


Renderer::~Renderer() {
    rlUnloadShaderProgram(m_computeShaderProgram);
    TRACE("Unloaded compute shader program (id: %u)", m_computeShaderProgram);

    UnloadTexture(m_outImage);
    TRACE("Unloaded output texture (id: %u)", m_outImage.id);

    CloseWindow();
    INFO("Closed window");
}


void Renderer::render(bool compute, bool draw, bool drawDebug) {
    if (compute) {
        runComputeShader();
    }
    if (draw) {
        BeginDrawing();
        ClearBackground(GREEN);

        drawOutImage();
        if (drawDebug) {
            drawDebugDisplay();
        }

        EndDrawing();
    }
}


void Renderer::compileComputeShader(CompileShaderParams _params) {
    if (m_compiled) {
        INFO("Cannot compile shader again");
        return;
    }

    m_compiled = true;
    m_compileParams = _params;

    makeBufferObjects();

    const char* shaderPath = "shaders/raytracer.glsl";
    char* fileText = LoadFileText(shaderPath);
    if (fileText != NULL) {
        TRACE("File '%s' loaded succesfully", shaderPath);
    }

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

    INFO("Compiling compute shader with");
    INFO("    Workgroup Size: %u", m_compileParams.workgroupSize);
    INFO("    Buffer Type: %s",
         m_compileParams.storageType == SceneStorageType::Uniforms ? "UBO" : "SSBO");
    INFO("    Max Sphere Count: %u", m_compileParams.maxSphereCount);
    INFO("    Max Triangle Count: %u", m_compileParams.maxTriangleCount);

    const unsigned shaderId = rlCompileShader(fileText, RL_COMPUTE_SHADER);
    if (shaderId != 0) {
        TRACE("Compiled compute shader successfully (id: %d)", shaderId);
    }

    m_computeShaderProgram = rlLoadComputeShaderProgram(shaderId);
    if (m_computeShaderProgram != 0) {
        TRACE("Loaded compute shader program (id: %d)", m_computeShaderProgram);
    }

    UnloadFileText(fileText);
    TRACE("Unloaded file '%s'", shaderPath);
}


void Renderer::resetImage() {
    static Image img = GenImageColor(m_imageSize.x, m_imageSize.y, BLANK);
    ImageFormat(&img, m_outImage.format);
    UpdateTexture(m_outImage, img.data);
    m_frameIndex = 0;
}


bool Renderer::saveImage(const char* fileName) const {
    TRACE("Saving texture as '%s'", fileName);
    Image img = LoadImageFromTexture(m_outImage);
    bool res = ExportImage(img, fileName);
    if (res) {
        INFO("Saved texture as '%s'", fileName);
    }
    return res;
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
    m_currentScene = &scene;

    INFO("Setting scene [ID: %u]", scene.getId());
    rlEnableShader(m_computeShaderProgram);

    const int uniLoc_numMaterials = getUniformLoc("numMaterials");
    float numMaterials = scene.m_materialData->getMaterialCount();
    rlSetUniform(uniLoc_numMaterials, &numMaterials, RL_SHADER_UNIFORM_FLOAT, 1);
    TRACE("    Uniform float set [index = %d | numMaterials = %d]", uniLoc_numMaterials, scene.m_materialData->getMaterialCount());

    const int uniLoc_materialTexture = getUniformLoc("materialTexture");
    rlSetUniformSampler(uniLoc_materialTexture, scene.m_materialData->getTextureId());
    TRACE("    Uniform sampler set [index = %d | materialTextureId = %d]", uniLoc_materialTexture, scene.m_materialData->getTextureId());

    setScene_spheres(scene);
    setScene_triangles(scene);

    const int uniLoc_backgroundColor = getUniformLoc("sceneInfo.backgroundColor");
    rlSetUniform(uniLoc_backgroundColor, &scene.m_backgroundColor, RL_SHADER_UNIFORM_VEC3, 1);
    TRACE("    Uniform vec3 set [index = %d | scene.backgroundColor = (%f %f %f)]", uniLoc_backgroundColor, scene.m_backgroundColor.x, scene.m_backgroundColor.y, scene.m_backgroundColor.z);
}


void Renderer::setConfig(const rt::Config& config) const {
    m_currentConfig = &config;

    INFO("Setting configuration: {numSamples: %d, bounceLimit: %d}", (int) config.numSamples, (int) config.bounceLimit);
    rlEnableShader(m_computeShaderProgram);

    const int uniLoc_numSamples = getUniformLoc("config.numSamples");
    rlSetUniform(uniLoc_numSamples, &config.numSamples, RL_SHADER_UNIFORM_FLOAT, 1);
    TRACE("    Uniform float set [index = %d | config.numSamples = %d]", uniLoc_numSamples, (int) config.numSamples);

    const int uniLoc_bounceLimit = getUniformLoc("config.bounceLimit");
    rlSetUniform(uniLoc_bounceLimit, &config.bounceLimit, RL_SHADER_UNIFORM_FLOAT, 1);
    TRACE("    Uniform float set [index = %d | config.bounceLimit = %d]", uniLoc_bounceLimit, (int) config.bounceLimit);
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
    const Rectangle srcRect = {0, 0, m_imageSize.x, m_imageSize.y};
    const Rectangle destRect = {0, 0, m_windowSize.x, m_windowSize.y};
    DrawTexturePro(m_outImage, srcRect, destRect, {0, 0}, 0, WHITE);
}


void Renderer::drawDebugDisplay() const {
    DrawRectangle(5, 5, 360, 205, {0, 0, 0, 30});
    const char* text = nullptr;

    text = TextFormat("Frame time: %.3fms (%d FPS)", GetFrameTime(), GetFPS());
    DrawText(text, 10, 10, 20, BLUE);

    DrawText("Config:", 10, 40, 20, BLUE);
    text = TextFormat("Samples per frame: %d", (int)m_currentConfig->numSamples);
    DrawText(text, 20, 60, 20, BLUE);
    text = TextFormat("Max bounces: %d", (int)m_currentConfig->bounceLimit);
    DrawText(text, 20, 80, 20, BLUE);

    text = TextFormat("Scene: %u", m_currentScene->getId());
    DrawText(text, 10, 120, 20, BLUE);
    text = TextFormat("Spheres: %d", m_currentScene->m_spheres.size());
    DrawText(text, 20, 140, 20, BLUE);
    text = TextFormat("Triangles: %d", m_currentScene->m_triangles.size());
    DrawText(text, 20, 160, 20, BLUE);
    text = TextFormat("Num materials: %d", m_currentScene->m_materialData->getMaterialCount());
    DrawText(text, 20, 180, 20, BLUE);
}


void Renderer::makeOutImage() {
    Image image = GenImageColor(m_imageSize.x, m_imageSize.y, BLUE);
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16);
    m_outImage = LoadTextureFromImage(image);
    UnloadImage(image);

    if (m_outImage.id != 0) {
        INFO("Created output texture of size = %d x %d (id: %u)", (int) m_imageSize.x, (int) m_imageSize.y, m_outImage.id);
    }
}


void Renderer::makeBufferObjects() {
    if (m_compileParams.storageType != SceneStorageType::Buffers) {
        return;
    }

    m_sceneSpheresBuffer = rlLoadShaderBuffer(sizeof(rt::Sphere) * m_compileParams.maxSphereCount, nullptr, RL_DYNAMIC_COPY);
    m_sceneTrianglesBuffer = rlLoadShaderBuffer(sizeof(rt::Triangle) * m_compileParams.maxTriangleCount, nullptr, RL_DYNAMIC_COPY);

    if (m_sceneSpheresBuffer != 0) {
        TRACE("Created buffer for scene-spheres (ssbo-id: %u)", m_sceneSpheresBuffer);
    }
    if (m_sceneTrianglesBuffer != 0) {
        TRACE("Created buffer for scene-triangles (ssbo-id: %u)", m_sceneTrianglesBuffer);
    }

    if (m_sceneSpheresBuffer == 0 && m_sceneTrianglesBuffer == 0) {
        INFO("Created buffers for scene's spheres and triangls (ssbos: %u %u)", m_sceneSpheresBuffer, m_sceneTrianglesBuffer);
    }
}


void Renderer::setScene_spheres(const rt::CompiledScene& scene) const {
    unsigned numSpheres = scene.m_spheres.size();

    if (m_compileParams.storageType == SceneStorageType::Uniforms) {
        numSpheres = std::min(numSpheres, m_compileParams.maxSphereCount);

        TRACE("    Setting scene-spheres uniform");

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

        TRACE("    Setting scene-spheres buffer of size = %u bytes", sphereBufferSize);
        rlUpdateShaderBuffer(m_sceneSpheresBuffer, scene.m_spheres.data(), sphereBufferSize, 0);
    }

    const int uniLoc_numSpheres = getUniformLoc("sceneInfo.numSpheres");
    rlSetUniform(uniLoc_numSpheres, &numSpheres, RL_SHADER_UNIFORM_INT, 1);

    TRACE("    Uniform float set [index = %d | numSpheres =  %u]", uniLoc_numSpheres, numSpheres);
}


void Renderer::setScene_triangles(const rt::CompiledScene& scene) const {
    unsigned numTriangles = scene.m_triangles.size();

    if (m_compileParams.storageType == SceneStorageType::Uniforms) {
        numTriangles = std::min(numTriangles, m_compileParams.maxTriangleCount);

        TRACE("    Setting scene-triangles uniform");

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
        TRACE("    Setting scene-triangles buffer of size = %u bytes", triangleBufferSize);

        rlUpdateShaderBuffer(m_sceneTrianglesBuffer, scene.m_triangles.data(), triangleBufferSize, 0);
    }

    const int uniLoc_numTriangles = getUniformLoc("sceneInfo.numTriangles");
    rlSetUniform(uniLoc_numTriangles, &numTriangles, RL_SHADER_UNIFORM_INT, 1);

    TRACE("    Uniform float set [index = %d | numTriangles =  %u]", uniLoc_numTriangles, numTriangles);
}
