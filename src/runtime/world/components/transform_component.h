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
    void set_local_location(const vec3& value)
    {
        local_location = value;
        dirty = true;
    }

    [[nodiscard]]
    const vec3& get_local_location() const
    {
        return local_location;
    }
    
    void set_local_rotation(const quat& value)
    {
        local_rotation = value;
        dirty = true;
    }
    
    [[nodiscard]]
    const quat& get_local_rotation() const
    {
        return local_rotation;
    }
    
    void set_local_euler_angles(const vec3& value)
    {
        set_local_rotation(quat(value));
    }
    
    [[nodiscard]]
    vec3 get_local_euler_angles() const
    {
        return eulerAngles(local_rotation);
    }

    void set_local_scale(const vec3& value)
    {
        local_scale = value;
        dirty = true;
    }

    [[nodiscard]]
    const vec3& get_local_scale() const
    {
        return local_scale;
    }
    
    void set_location(const vec3& value);
    
    [[nodiscard]]
    vec3 get_location() const;

    void set_rotation(const quat& value)
    {
        local_rotation = value;
        dirty = true;
    }

    [[nodiscard]]
    quat get_rotation() const;

    void set_euler_angles(const vec3& value)
    {
        set_rotation(quat(value));
    }

    [[nodiscard]]
    vec3 get_euler_angles() const
    {
        return eulerAngles(get_rotation());
    }

    void set_scale(const vec3& value)
    {
        local_scale = value;
        dirty = true;
    }

    [[nodiscard]]
    vec3 get_scale() const;

    [[nodiscard]]
    vec3 get_forward() const
    {
        return mat3_cast(local_rotation) * FORWARD_VECTOR;
    }

    [[nodiscard]]
    vec3 get_backward() const
    {
        return mat3_cast(local_rotation) * BACKWARD_VECTOR;
    }
    
    [[nodiscard]]
    vec3 get_left() const
    {
        return mat3_cast(local_rotation) * LEFT_VECTOR;
    }
    
    [[nodiscard]]
    vec3 get_right() const
    {
        return mat3_cast(local_rotation) * RIGHT_VECTOR;
    }
    
    [[nodiscard]]
    vec3 get_up() const
    {
        return mat3_cast(local_rotation) * UP_VECTOR;
    }
    
    [[nodiscard]]
    vec3 get_down() const
    {
        return mat3_cast(local_rotation) * DOWN_VECTOR;
    }

    void set_local_to_parent(const mat4& matrix);

    [[nodiscard]]
    const mat4& get_local_to_parent() const;

    void set_local_to_world(const mat4& matrix);

    [[nodiscard]]
    mat4 get_local_to_world() const;

  private:
    vec3 local_location = vec3(0.0, 0.0, 0.0);
    quat local_rotation = quat(1.0, 0.0, 0.0, 0.0);
    vec3 local_scale = vec3(1.0, 1.0, 1.0);
    glm::mat4 local_to_parent = glm::mat4(1.0);
    bool dirty = true; // True if the local_to_parent matrix needs to be updated
};
} // namespace ash
