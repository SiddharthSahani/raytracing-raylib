
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


void Renderer::render(const SceneCamera& camera, const rt::Scene& scene, const rt::Config& config,
                      bool forceCameraUpdate, bool raytrace) {

    BeginDrawing();
    ClearBackground(scene.backgroundColor);

    if (raytrace) {
        if (forceCameraUpdate || m_camera != &camera) {
            m_camera = &camera;
            updateCurrentCamera();
        }
        if (m_scene != &scene) {
            m_scene = &scene;
            updateCurrentScene();
        }
        if (m_config != &config) {
            m_config = &config;
            updateCurrentConfig();
        }

        runComputeShader();

        DrawTexturePro(m_outImage, {0, 0, m_imageSize.x, m_imageSize.y},
                       {0, 0, m_windowSize.x, m_windowSize.y}, {0, 0}, 0, WHITE);
    } else {

        Camera3D rlCamera{
            .position = camera.m_camera.position,
            .target = Vector3Add(camera.m_camera.position, camera.m_direction),
            .up = {0, 1, 0},
            .fovy = camera.m_fov,
            .projection = CAMERA_PERSPECTIVE,
        };

        BeginMode3D(rlCamera);

        for (const rt::Sphere& sph : scene.spheres) {
            Color col = ColorFromNormalized({
                scene.materials[sph.materialIndex].albedo.x,
                scene.materials[sph.materialIndex].albedo.y,
                scene.materials[sph.materialIndex].albedo.z,
                1.0,
            });
            DrawSphere(sph.position, sph.radius, col);
        }

        for (const rt::Triangle& tri : scene.triangles) {
            Color col = ColorFromNormalized({
                scene.materials[tri.materialIndex].albedo.x,
                scene.materials[tri.materialIndex].albedo.y,
                scene.materials[tri.materialIndex].albedo.z,
                1.0,
            });
            DrawTriangle3D(tri.v0, tri.v1, tri.v2, col);
            DrawTriangle3D(tri.v0, tri.v2, tri.v1, col);
        }

        // DOESNT SUPPORT PLANES yet
        // for (const rt::Plane& pl : scene.planes) {
        //     Color col = ColorFromNormalized({
        //         scene.materials[pl.materialIndex].albedo.x,
        //         scene.materials[pl.materialIndex].albedo.y,
        //         scene.materials[pl.materialIndex].albedo.z,
        //         1.0,
        //     });
        // }

        EndMode3D();
    }

    DrawFPS(10, 10);
    DrawText(TextFormat("Frame Time: %.5f", GetFrameTime()), 10, 30, 20, DARKBLUE);
    DrawText(TextFormat("Frame Index: %d", m_frameIndex), 10,50, 20, DARKBLUE);
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


void Renderer::resetImage() {
    static Image img = GenImageColor(m_imageSize.x, m_imageSize.y, BLANK);
    ImageFormat(&img, m_outImage.format);
    UpdateTexture(m_outImage, img.data);
    m_frameIndex = 0;
}


bool Renderer::canRender() const {
    return m_computeShaderProgram && m_camera && m_scene && m_config;
}


void Renderer::updateCurrentCamera() {
    const rt::Camera& camera = m_camera->get();

    rlEnableShader(m_computeShaderProgram);

    const int uniLoc_invViewMat = getUniformLoc("camera.invViewMat");
    const int uniLoc_invProjMat = getUniformLoc("camera.invProjMat");
    const int uniLoc_position = getUniformLoc("camera.position");

    rlSetUniformMatrix(uniLoc_invViewMat, camera.invViewMat);
    rlSetUniformMatrix(uniLoc_invProjMat, camera.invProjMat);
    rlSetUniform(uniLoc_position, &camera.position, RL_SHADER_UNIFORM_VEC3, 1);
}


void Renderer::updateCurrentScene() {
    const rt::Scene& scene = *m_scene;

    rlEnableShader(m_computeShaderProgram);

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


void Renderer::updateCurrentConfig() {
    const rt::Config& config = *m_config;

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
    if (!canRender()) {
        return;
    }

    m_frameIndex++;
    static int uniLoc_frameIndex = getUniformLoc("frameIndex");

    const int groupX = m_imageSize.x / m_compileParams.workgroupSize;
    const int groupY = m_imageSize.y / m_compileParams.workgroupSize;

    rlEnableShader(m_computeShaderProgram);

    rlSetUniform(uniLoc_frameIndex, &m_frameIndex, RL_SHADER_UNIFORM_INT, 1);
    rlBindImageTexture(m_outImage.id, 0, m_outImage.format, false);
    rlBindShaderBuffer(m_sceneMaterialsBuffer, 1);
    rlBindShaderBuffer(m_sceneSpheresBuffer, 2);
    rlBindShaderBuffer(m_scenePlanesBuffer, 3);
    rlBindShaderBuffer(m_sceneTrianglesBuffer, 4);

    rlComputeShaderDispatch(groupX, groupY, 1);
}


void Renderer::makeOutImage() {
    Image image = GenImageColor(m_imageSize.x, m_imageSize.y, BLUE);
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16);
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
