#pragma once

#include "world/component.h"
#include "glm/glm.hpp"

using glm::vec2;
using glm::vec3;

namespace ash
{
class FlyCameraControllerComponent : public Component
{
  public:
    float pitch = 0.f;
    float yaw = 0.f;
    vec2 look_sensitivity = vec2(0.001f, 0.001f);
    float move_speed = 10.f;

    void update(float dt) override;
};

class OrbitCameraControllerComponent : public Component
{
  public:
    float pitch = 0.f;
    float yaw = 0.f;
    vec3 pivot = vec3(0.f);
    float distance = 20.f;
    vec2 look_sensitivity = vec2(0.001f, 0.001f);
    float scroll_sensitivity = 1.f;
    float min_distance = 1.f;
    float max_distance = 100.f;

    void update(float dt) override;
};
} // namespace ash
