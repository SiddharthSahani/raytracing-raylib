
#pragma once

#include "src/structs/camera.h"


struct SceneCameraParams {
    float sensitivity = 0.002f;
    float speed = 5.0f;
    float rotationSpeed = 2.0f;
};


class SceneCamera {

public:
    SceneCamera(Vector3 position, Vector3 direction, float fov, Vector2 imgSize, SceneCameraParams params);
    bool update(float dt);
    void updateProjMatrix(Vector2 imgSize, float fov);
    SceneCameraParams& getParams() { return m_params; }
    const rt::Camera& get() const { return m_camera; }

private:
    Vector3 m_direction;
    rt::Camera m_camera;
    SceneCameraParams m_params;
};
