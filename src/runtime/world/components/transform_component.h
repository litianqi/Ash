#pragma once

#include "world/component.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;

namespace ash
{
// For simplicity, we choose OpenGL's clip space conventions:
// https://johannesugb.github.io/gpu-programming/why-do-opengl-proj-matrices-fail-in-vulkan/
// https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
constexpr vec3 RIGHT_VECTOR = vec3(1.0, 0.0, 0.0);
constexpr vec3 LEFT_VECTOR = vec3(-1.0, 0.0, 0.0);
constexpr vec3 UP_VECTOR = vec3(0.0, 1.0, 0.0);
constexpr vec3 DOWN_VECTOR = vec3(0.0, -1.0, 0.0);
constexpr vec3 FORWARD_VECTOR = vec3(0.0, 0.0, 1.0);
constexpr vec3 BACKWARD_VECTOR = vec3(0.0, 0.0, -1.0);

class TransformComponent : public Component
{
  public:
    // Set the local location of the transform.
    void set_local_location(const vec3& value)
    {
        local_location = value;
        dirty = true;
    }
    
    // Get the local location of the transform.
    const vec3& get_local_location() const
    {
        return local_location;
    }
    
    // Set the local rotation of the transform.
    void set_local_rotation(const quat& value)
    {
        local_rotation = value;
        dirty = true;
    }
    
    // Get the local rotation of the transform.
    const quat& get_local_rotation() const
    {
        return local_rotation;
    }
    
    // Set the local euler angles of the transform.
    void set_local_euler_angles(const vec3& value)
    {
        set_local_rotation(quat(value));
    }
    
    // Get the local euler angles of the transform.
    vec3 get_local_euler_angles() const
    {
        return eulerAngles(local_rotation);
    }

    // Set the local scale of the transform.
    void set_local_scale(const vec3& value)
    {
        local_scale = value;
        dirty = true;
    }
    
    // Get the local scale of the transform.
    const vec3& get_local_scale() const
    {
        return local_scale;
    }
    
    // Set the location of the transform.
    void set_location(const vec3& value);
    
    // Get the location of the transform.
    vec3 get_location() const;

    // Set the rotation of the transform.
    void set_rotation(const quat& value)
    {
        local_rotation = value;
        dirty = true;
    }
    
    // Get the rotation of the transform.
    quat get_rotation() const;

    // Set the euler angles of the transform.
    void set_euler_angles(const vec3& value)
    {
        set_rotation(quat(value));
    }
    
    // Get the euler angles of the transform.
    vec3 get_euler_angles() const
    {
        return eulerAngles(get_rotation());
    }

    // Set the scale of the transform.
    void set_scale(const vec3& value)
    {
        local_scale = value;
        dirty = true;
    }
    
    // Get the scale of the transform.
    vec3 get_scale() const;
    
    // Get the forward vector of the transform.
    vec3 get_forward() const
    {
        return mat3_cast(local_rotation) * FORWARD_VECTOR;
    }
    
    // Get the backward vector of the transform.
    vec3 get_backward() const
    {
        return mat3_cast(local_rotation) * BACKWARD_VECTOR;
    }
    
    // Get the left vector of the transform.
    vec3 get_left() const
    {
        return mat3_cast(local_rotation) * LEFT_VECTOR;
    }
    
    // Get the right vector of the transform.
    vec3 get_right() const
    {
        return mat3_cast(local_rotation) * RIGHT_VECTOR;
    }
    
    // Get the up vector of the transform.
    vec3 get_up() const
    {
        return mat3_cast(local_rotation) * UP_VECTOR;
    }
    
    // Get the down vector of the transform.
    vec3 get_down() const
    {
        return mat3_cast(local_rotation) * DOWN_VECTOR;
    }

    // Set the local to parent matrix of the transform.
    void set_local_to_parent(const mat4& matrix);
    
    // Get the local to parent matrix of the transform.
    const mat4& get_local_to_parent() const;

    // Set the local to world matrix of the transform.
    void set_local_to_world(const mat4& matrix);
    
    // Get the local to world matrix of the transform.
    mat4 get_local_to_world() const;

  private:
    vec3 local_location = vec3(0.0, 0.0, 0.0);
    quat local_rotation = quat(1.0, 0.0, 0.0, 0.0);
    vec3 local_scale = vec3(1.0, 1.0, 1.0);
    glm::mat4 local_to_parent = glm::mat4(1.0);
    bool dirty = true; // True if the local_to_parent matrix needs to be updated
};
} // namespace ash
