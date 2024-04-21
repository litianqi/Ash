#pragma once

#include "world/component.h"
#include "core/math.h"

namespace ash
{
class FlyCameraControllerComponent : public Component
{
  public:
    // Current pitch of the camera.
    float pitch = 0.f;
    // Current yaw of the camera.
    float yaw = 0.f;
    // Sensitivity to look input.
    vec2 look_sensitivity = vec2(0.001f, 0.001f);
    // Move speed of the camera.
    float move_speed = 10.f;

    void update(float dt) override;
};

class OrbitCameraControllerComponent : public Component
{
  public:
    // Current pitch of the camera.
    float pitch = 0.f;
    // Current yaw of the camera.
    float yaw = 0.f;
    // Pivot point of the camera. Camera will orbit around this point.
    vec3 pivot = vec3(0.f);
    // Distance from the pivot point.
    float distance = 20.f;
    // Sensitivity to look input.
    vec2 look_sensitivity = vec2(0.001f, 0.001f);
    // Sensitivity to scroll input.
    float scroll_sensitivity = 1.f;
    // Minimum distance from the pivot point.
    float min_distance = 1.f;
    // Maximum distance from the pivot point.
    float max_distance = 100.f;

    void update(float dt) override;
};
} // namespace ash
