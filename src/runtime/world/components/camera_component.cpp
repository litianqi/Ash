#include "camera_component.h"

namespace ash
{
mat4 CameraComponent::get_view_matrix() const
{
    assert(owner);
    return glm::inverse(owner->get_component<TransformComponent>()->get_local_to_world());
}

void CameraComponent::set_eye_at_up(vec3 eye, vec3 at, vec3 up)
{
    assert(owner);
    auto view_matrix = glm::lookAtLH(eye, at, up);
    auto* transform = owner->get_component<TransformComponent>();
    transform->set_local_to_world(view_matrix);
}

void CameraComponent::set_look_direction(vec3 forward, vec3 up)
{
    assert(owner);
    auto* transform = owner->get_component<TransformComponent>();
    auto eye = transform->get_location();
    auto at = eye + glm::normalize(forward);
    set_eye_at_up(eye, at, up);
}
} // namespace ash
