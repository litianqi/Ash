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
    // Aspect ratio of the camera.
    float aspect_ratio = 1.0f;
    // Field of view of the camera.
    float fov = std::numbers::pi * 0.5f;
    // Near plane of the camera.
    float near = 0.1f;
    // Far plane of the camera.
    float far = 512.f;

    // Set the eye, at, and up vectors of the camera.
    void set_eye_at_up(vec3 eye, vec3 at, vec3 up);

    // Set the look direction of the camera.
    void set_look_direction(vec3 forward, vec3 up);
    
    // Get the view matrix of the camera.
    mat4 get_view_matrix() const;
    
    // Get the projection matrix of the camera.
    mat4 get_projection_matrix() const
    {
        return glm::perspectiveLH(fov, aspect_ratio, near, far);
    }
    
    // Get the view-projection matrix of the camera.
    mat4 get_view_projection_matrix() const
    {
        return get_projection_matrix() * get_view_matrix();
    }
};
} // namespace ash
