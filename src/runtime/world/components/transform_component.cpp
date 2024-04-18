#include "transform_component.h"
#include "core/file.h"
#include "core/math.h"

namespace ash
{
void TransformComponent::set_location(const vec3& value)
{
    assert(owner);
    if (const auto parent = owner->get_parent())
    {
        const auto new_local_location = glm::inverse(parent->get_transform()->get_local_to_world()) * vec4(value, 1.0f);
        set_local_location(new_local_location);
    }
    else
    {
        set_local_location(value);
    }
}

vec3 TransformComponent::get_location() const // NOLINT(misc-no-recursion)
{
    assert(owner);
    if (const auto parent = owner->get_parent())
    {
        return parent->get_transform()->get_location() + local_location;
    }
    else
    {
        return local_location;
    }
}

quat TransformComponent::get_rotation() const // NOLINT(misc-no-recursion)
{
    assert(owner);
    if (const auto parent = owner->get_parent())
    {
        return parent->get_transform()->get_rotation() + local_rotation;
    }
    else
    {
        return local_rotation;
    }
}

vec3 TransformComponent::get_scale() const // NOLINT(misc-no-recursion)
{
    assert(owner);
    if (const auto parent = owner->get_parent())
    {
        return parent->get_transform()->get_scale() * local_scale;
    }
    else
    {
        return local_scale;
    }
}

void TransformComponent::set_local_to_parent(const mat4& matrix)
{
    local_to_parent = matrix;
    mat4_decompose(matrix, local_scale, local_rotation, local_location);
    dirty = false;
}

const mat4& TransformComponent::get_local_to_parent() const
{
    if (dirty)
    {
        const_cast<TransformComponent*>(this)->local_to_parent = glm::translate(glm::mat4(1.0), local_location) *
                                                                 glm::mat4_cast(local_rotation) *
                                                                 glm::scale(glm::mat4(1.0), local_scale);
        const_cast<TransformComponent*>(this)->dirty = false;
    }
    return local_to_parent;
}

void TransformComponent::set_local_to_world(const mat4& matrix)
{
    assert(owner);
    if (const auto parent = owner->get_parent())
    {
        const auto new_local_to_parent = glm::inverse(parent->get_transform()->get_local_to_world()) * matrix;
        set_local_to_parent(new_local_to_parent);
    }
    else
    {
        set_local_to_parent(matrix);
    }
}

mat4 TransformComponent::get_local_to_world() const // NOLINT(misc-no-recursion)
{
    assert(owner);
    if (const auto parent = owner->get_parent())
    {
        return parent->get_transform()->get_local_to_world() * get_local_to_parent();
    }
    else
    {
        return get_local_to_parent();
    }
}
} // namespace ash
