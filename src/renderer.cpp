
#include "src/renderer.h"

using namespace rl;
#include <raylib/raymath.h>
#include <raylib/rlgl.h>


Renderer::Renderer(rl::Vector2 windowSize, rl::Vector2 imageSize, unsigned computeLocalSize)
    : m_windowSize(windowSize), m_imageSize(imageSize), m_computeLocalSize(computeLocalSize) {
    rl::InitWindow(m_windowSize.x, m_windowSize.y, "Raytracing");
    rl::SetTargetFPS(30);

    makeOutputTexture();
    makeBufferObjects();
    compileComputeShader();
}


Renderer::~Renderer() {
    rlUnloadShaderProgram(m_computeShaderProgram);
    rl::UnloadTexture(m_outputTexture);
    rl::CloseWindow();
}


void Renderer::loop() {
    while (!rl::WindowShouldClose()) {
        runComputeShader();

        rl::BeginDrawing();

        rl::DrawTexturePro(m_outputTexture, {0, 0, m_imageSize.x, m_imageSize.y},
                           {0, 0, m_windowSize.x, m_windowSize.y}, {0, 0}, 0, rl::WHITE);

        rl::DrawFPS(10, 10);
        rl::EndDrawing();
    }
}


void Renderer::makeOutputTexture() {
    rl::Image image = rl::GenImageColor(m_imageSize.x, m_imageSize.y, rl::GREEN);
    rl::ImageFormat(&image, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    m_outputTexture = rl::LoadTextureFromImage(image);
    rl::UnloadImage(image);
}


void Renderer::makeBufferObjects() {
    m_buffer = rlLoadShaderBuffer(sizeof(float), nullptr, RL_DYNAMIC_COPY);
}


void Renderer::compileComputeShader() {
    char* fileText = rl::LoadFileText("shaders/raytracer.glsl");
    char* newfileText = rl::TextReplace(fileText, "WG_SIZE_PLACEHOLDER", rl::TextFormat("%d", m_computeLocalSize));
    rl::UnloadFileText(fileText);
    unsigned shaderId = rlCompileShader(newfileText, RL_COMPUTE_SHADER);
    m_computeShaderProgram = rlLoadComputeShaderProgram(shaderId);
    rl::UnloadFileText(newfileText);
}


void Renderer::runComputeShader() {
    float time = rl::GetTime();
    rlUpdateShaderBuffer(m_buffer, &time, sizeof(float), 0);

    rlBindImageTexture(m_outputTexture.id, 0, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, false);
    rlBindShaderBuffer(m_buffer, 1);

    rlEnableShader(m_computeShaderProgram);
    const int groupX = m_imageSize.x / m_computeLocalSize;
    const int groupY = m_imageSize.y / m_computeLocalSize;
    rlComputeShaderDispatch(groupX, groupY, 1);
    rlDisableShader();
}

// void Renderer::updateShaderCamera() {
//     rl::Vector3 position = {0, 0, 6};
//     rl::Vector3 direction = {0, 0, -1};

//     rl::Matrix viewMat = MatrixLookAt(position, Vector3Add(position, direction), {0, 1, 0});
//     rl::Matrix invViewMat = MatrixInvert(viewMat);

//     rl::Matrix projMat =
//         MatrixPerspective(60.0 * DEG2RAD, m_imageSize.x / m_imageSize.y, 0.1, 100.0);
//     rl::Matrix invProjMat = MatrixInvert(projMat);

//     rl::SetShaderValue(m_renderShader, rl::GetShaderLocation(m_renderShader, "uImageSize"),
//                        &m_imageSize, rl::SHADER_UNIFORM_VEC2);
//     rl::SetShaderValueMatrix(
//         m_renderShader, rl::GetShaderLocation(m_renderShader, "camera.invViewMat"), invViewMat);
//     rl::SetShaderValueMatrix(
//         m_renderShader, rl::GetShaderLocation(m_renderShader, "camera.invProjMat"), invProjMat);
//     rl::SetShaderValue(m_renderShader, rl::GetShaderLocation(m_renderShader, "camera.position"),
//                        &position, rl::SHADER_UNIFORM_VEC3);
// }


// void Renderer::updateShaderSpheres() {
//     {
//         rl::Vector3 spherePos = {0, 0, 0};
//         float sphereRad = 1.0;
//         rl::Vector3 sphereCol = {0.2, 0.9, 0.8};
//         rl::SetShaderValue(m_renderShader,
//                            rl::GetShaderLocation(m_renderShader, "scene.spheres[0].position"),
//                            &spherePos, rl::SHADER_UNIFORM_VEC3);
//         rl::SetShaderValue(m_renderShader,
//                            rl::GetShaderLocation(m_renderShader, "scene.spheres[0].radius"),
//                            &sphereRad, rl::SHADER_UNIFORM_FLOAT);
//         rl::SetShaderValue(m_renderShader,
//                            rl::GetShaderLocation(m_renderShader, "scene.spheres[0].color"),
//                            &sphereCol, rl::SHADER_UNIFORM_VEC3);
//     }
//     {
//         rl::Vector3 spherePos = {0, -4, 0};
//         float sphereRad = 3.0;
//         rl::Vector3 sphereCol = {1, 0, 1};
//         rl::SetShaderValue(m_renderShader,
//                            rl::GetShaderLocation(m_renderShader, "scene.spheres[1].position"),
//                            &spherePos, rl::SHADER_UNIFORM_VEC3);
//         rl::SetShaderValue(m_renderShader,
//                            rl::GetShaderLocation(m_renderShader, "scene.spheres[1].radius"),
//                            &sphereRad, rl::SHADER_UNIFORM_FLOAT);
//         rl::SetShaderValue(m_renderShader,
//                            rl::GetShaderLocation(m_renderShader, "scene.spheres[1].color"),
//                            &sphereCol, rl::SHADER_UNIFORM_VEC3);
//     }

//     int numSpheres = 2;
//     rl::SetShaderValue(m_renderShader, rl::GetShaderLocation(m_renderShader, "scene.numSpheres"),
//                        &numSpheres, rl::SHADER_UNIFORM_INT);

//     rl::Vector3 backgroundColor = {210, 210, 210};
//     backgroundColor = Vector3Divide(backgroundColor, {255, 255, 255});
//     rl::SetShaderValue(m_renderShader,
//                        rl::GetShaderLocation(m_renderShader, "scene.backgroundColor"),
//                        &backgroundColor, rl::SHADER_UNIFORM_VEC3);
// }
