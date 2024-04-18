#pragma once

#include "glm/glm.hpp"
#include "glm/ext.hpp"

using glm::mat3;
using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;

namespace ash
{
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

static inline void mat4_decompose(const mat4& m, vec3& scale, quat& rotation, vec3& translation)
{
    scale = vec3(glm::length(m[0]), glm::length(m[1]), glm::length(m[2]));
    mat3 rotate_matrix(vec3(m[0]) / scale.x, vec3(m[1]) / scale.y, vec3(m[2]) / scale.z);
    rotation = quat_cast(rotate_matrix);
    translation = vec3(m[3]);
}
} // namespace ash
