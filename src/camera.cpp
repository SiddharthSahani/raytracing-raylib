
#include "src/camera.h"
#include <raylib/raymath.h>


SceneCamera::SceneCamera(Vector3 position, Vector3 direction, float fov, Vector2 imgSize, SceneCameraParams params)
    : m_direction(direction), m_params(params), m_fov(fov) {
    m_camera.position = position;

    Matrix projMat = MatrixPerspective(fov * DEG2RAD, imgSize.x / imgSize.y, 0.1, 100.0);
    Matrix viewMat = MatrixLookAt(position, Vector3Add(position, direction), {0, 1, 0});
    m_camera.invProjMat = MatrixInvert(projMat);
    m_camera.invViewMat = MatrixInvert(viewMat);
}


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
    const Vector3 rightDirection = Vector3CrossProduct(m_direction, upDirection);
    const float camSpeed = m_params.speed * timestep * (IsKeyDown(KEY_LEFT_SHIFT) ? 5 : 1);

    if (IsKeyDown(KEY_W)) {
        m_camera.position = Vector3Add(m_camera.position, Vector3Scale(m_direction, camSpeed));
        cameraUpdated = true;
    }
    if (IsKeyDown(KEY_S)) {
        m_camera.position = Vector3Subtract(m_camera.position, Vector3Scale(m_direction, camSpeed));
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
        m_camera.position = Vector3Subtract(m_camera.position, Vector3Scale(upDirection, camSpeed));
        cameraUpdated = true;
    }
    if (IsKeyDown(KEY_E)) {
        m_camera.position = Vector3Add(m_camera.position, Vector3Scale(upDirection, camSpeed));
        cameraUpdated = true;
    }

    // mouse movement
    if (delta.x != 0.0f || delta.y != 0.0f) {
        const float yawDelta = delta.x * m_params.rotationSpeed;
        const float pitchDelta = delta.y * m_params.rotationSpeed;

        m_direction = Vector3RotateByAxisAngle(m_direction, rightDirection, -pitchDelta);
        m_direction = Vector3RotateByAxisAngle(m_direction, upDirection, -yawDelta);
        m_direction = Vector3Normalize(m_direction);
        cameraUpdated = true;
    }

    if (cameraUpdated) {
        Matrix viewMat = MatrixLookAt(m_camera.position, Vector3Add(m_camera.position, m_direction), {0, 1, 0});
        m_camera.invViewMat = MatrixInvert(viewMat);
    }

    return cameraUpdated;
}
