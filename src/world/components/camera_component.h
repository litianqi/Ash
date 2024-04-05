#pragma once

#include <numbers>
#include "world/component.h"
#include "world/components/transform_component.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#undef near
#undef far

using glm::mat4;

namespace ash
{
class CameraComponent : public Component
{
  public:
    float aspect_ratio = 1.0f;
    float fov = std::numbers::pi * 0.5f;
    float near = 0.1f;
    float far = 512.f;

    void set_eye_at_up(vec3 eye, vec3 at, vec3 up);

    void set_look_direction(vec3 forward, vec3 up);
    
    [[nodiscard]]
    mat4 get_view_matrix() const;

    [[nodiscard]]
    mat4 get_projection_matrix() const
    {
        return glm::perspectiveLH(fov, aspect_ratio, near, far);
    }

    [[nodiscard]]
    mat4 get_view_projection_matrix() const
    {
        return get_projection_matrix() * get_view_matrix();
    }
};
} // namespace ash
