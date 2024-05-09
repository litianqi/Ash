#pragma once

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;
using glm::mat3;
using glm::mat4;

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

struct Bounds {
    vec3 origin;
    float sphere_radius = 0.f;
    vec3 extents;
};

static inline float float_reciprocal(const float& f, float epsilon = glm::epsilon<float>())
{
    return glm::equal(f, 0.f, epsilon) ? 0.f : 1.f / f;
}

static inline vec3 vec3_reciprocal(const vec3& v, float epsilon = glm::epsilon<float>())
{
    vec3 reciprocal_v = v;
    reciprocal_v.x = float_reciprocal(v.x, epsilon);
    reciprocal_v.y = float_reciprocal(v.y, epsilon);
    reciprocal_v.z = float_reciprocal(v.z, epsilon);
    return reciprocal_v;
}

static inline vec3 mat4_decompose_scale(const mat4& m)
{
    return vec3(glm::length(m[0]), glm::length(m[1]), glm::length(m[2]));
}

static inline vec3 mat4_decompose_translation(const mat4& m)
{
    return vec3(m[3]);
}

static inline void mat4_decompose_scale_rotation(const mat4& m, vec3& scale, quat& rotation)
{
    scale = vec3(glm::length(m[0]), glm::length(m[1]), glm::length(m[2]));
    mat3 rotate_matrix(vec3(m[0]) / scale.x, vec3(m[1]) / scale.y, vec3(m[2]) / scale.z);
    rotation = quat_cast(rotate_matrix);
}

static inline void mat4_decompose(const mat4& m, vec3& scale, quat& rotation, vec3& translation)
{
    scale = vec3(glm::length(m[0]), glm::length(m[1]), glm::length(m[2]));
    mat3 rotate_matrix(vec3(m[0]) / scale.x, vec3(m[1]) / scale.y, vec3(m[2]) / scale.z);
    rotation = quat_cast(rotate_matrix);
    translation = vec3(m[3]);
}

static inline mat4 mat4_compose(const vec3& scale, const quat& rotation, const vec3& translation)
{
    return glm::translate(glm::mat4(1.0), translation) *
           glm::mat4_cast(rotation) *
           glm::scale(glm::mat4(1.0), scale);
}
} // namespace ash
