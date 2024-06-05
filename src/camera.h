
#pragma once

#include "src/structs/camera.h"


struct SceneCameraParams {
    float sensitivity = 0.002f;
    float speed = 5.0f;
    float rotationSpeed = 2.0f;
};


class SceneCamera {

public:
    SceneCamera(rt::Camera camera, SceneCameraParams params);
    bool update(float dt);
    const rt::Camera& get() const { return m_camera; }

private:
    SceneCameraParams m_params;
    rt::Camera m_camera;
};
