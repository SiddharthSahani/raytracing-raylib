
#include "src/camera.h"
#include <raylib/raymath.h>


SceneCamera::SceneCamera(rt::Camera camera, SceneCameraParams params)
    : m_camera(camera), m_params(params) {}


bool SceneCamera::update(float timestep) {
    const Vector2 delta = Vector2Scale(GetMouseDelta(), m_params.sensitivity);

    static bool rmbPressed = false;

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !rmbPressed) {
        rmbPressed = true;
        DisableCursor();
    }
    if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        if (rmbPressed) {
            rmbPressed = false;
            EnableCursor();
        }
        return false;
    }

    bool cameraUpdated = false;

    const Vector3 upDirection = {0, 1, 0};
    const Vector3 rightDirection = Vector3CrossProduct(m_camera.direction, upDirection);
    const float camSpeed = m_params.speed * timestep;

    if (IsKeyDown(KEY_W)) {
        m_camera.position =
            Vector3Add(m_camera.position, Vector3Scale(m_camera.direction, camSpeed));
        cameraUpdated = true;
    }
    if (IsKeyDown(KEY_S)) {
        m_camera.position =
            Vector3Subtract(m_camera.position, Vector3Scale(m_camera.direction, camSpeed));
        cameraUpdated = true;
    }
    if (IsKeyDown(KEY_A)) {
        m_camera.position =
            Vector3Subtract(m_camera.position, Vector3Scale(rightDirection, camSpeed));
        cameraUpdated = true;
    }
    if (IsKeyDown(KEY_D)) {
        m_camera.position = Vector3Add(m_camera.position, Vector3Scale(rightDirection, camSpeed));
        cameraUpdated = true;
    }
    if (IsKeyDown(KEY_Q)) {
        m_camera.position = Vector3Add(m_camera.position, Vector3Scale(upDirection, camSpeed));
        cameraUpdated = true;
    }
    if (IsKeyDown(KEY_E)) {
        m_camera.position = Vector3Subtract(m_camera.position, Vector3Scale(upDirection, camSpeed));
        cameraUpdated = true;
    }

    // mouse movement
    if (delta.x != 0.0f || delta.y != 0.0f) {
        const float yawDelta = delta.x * m_params.rotationSpeed;
        const float pitchDelta = delta.y * m_params.rotationSpeed;

        m_camera.direction =
            Vector3RotateByAxisAngle(m_camera.direction, rightDirection, -pitchDelta);
        m_camera.direction = Vector3RotateByAxisAngle(m_camera.direction, upDirection, -yawDelta);
        m_camera.direction = Vector3Normalize(m_camera.direction);
        cameraUpdated = true;
    }

    return cameraUpdated;
}
