
#include "src/renderer.h"
#include "src/logger.h"


Renderer::Renderer(Vector2 windowSize)
    : m_windowSize(windowSize) {

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(windowSize.x, windowSize.y, "Raytracing");
    SetTargetFPS(30);

    if (IsWindowReady()) {
        INFO("Created window of size = %d x %d", (int) windowSize.x, (int) windowSize.y);
    }

    Image img = GenImageChecked(4, 4, 1, 1, PINK, BLACK);
    m_blankTexture = LoadTextureFromImage(img);
    UnloadImage(img);

    m_texFragShader = LoadShader(nullptr, "shaders/texFrag.glsl");
    SetShaderValue(m_texFragShader, GetShaderLocation(m_texFragShader, "windowSize"), &m_windowSize, SHADER_UNIFORM_VEC2);
    const float gamma = 2.2f;
    SetShaderValue(m_texFragShader, GetShaderLocation(m_texFragShader, "gamma"), &gamma, SHADER_UNIFORM_FLOAT);
}


Renderer::~Renderer() {
    UnloadTexture(m_blankTexture);
    UnloadShader(m_texFragShader);
    CloseWindow();
    INFO("Closed window");
}


void Renderer::setRaytracer(std::weak_ptr<Raytracer> raytracer) {
    m_raytracer = raytracer;
}


void Renderer::setGamma(float gamma) {
    static int gamma_uniLoc = GetShaderLocation(m_texFragShader, "gamma");

    SetShaderValue(m_texFragShader, gamma_uniLoc, &gamma, SHADER_UNIFORM_FLOAT);
}


void Renderer::render() {
    if (auto raytracer = m_raytracer.lock()) {
        raytracer->runComputeShader();
    }
}


void Renderer::draw() {
    BeginDrawing();

    DrawTexturePro(m_blankTexture, {0, 0, 4, 4}, {0, 0, m_windowSize.x, m_windowSize.y}, {0, 0}, 0, WHITE);

    if (auto raytracer = m_raytracer.lock()) {
        const Texture outTexture = raytracer->getOutTexture();
        const Vector2 texSize = raytracer->getTextureSize();
        const Rectangle srcRect = {0, 0, texSize.x, texSize.y};
        const Rectangle destRect = {0, 0, m_windowSize.x, m_windowSize.y};

        BeginShaderMode(m_texFragShader);
        DrawTexturePro(outTexture, srcRect, destRect, {0, 0}, 0, WHITE);
        EndShaderMode();
    }

    DrawFPS(10, 10);
    EndDrawing();
}
