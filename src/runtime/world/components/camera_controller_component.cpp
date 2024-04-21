#include "camera_controller_component.h"
#include "input/input_manager.h"
#include "core/math.h"
#include "spdlog/spdlog.h"

using glm::vec3;

namespace
{
quat pitch_yaw_rotation(float pitch, float yaw)
{
    quat pitch_rotation = glm::angleAxis(pitch, ash::RIGHT_VECTOR);
    quat yaw_rotation = glm::angleAxis(yaw, ash::UP_VECTOR);
    return yaw_rotation * pitch_rotation;
}
}

namespace ash
{
void FlyCameraControllerComponent::update(float dt)
{
    assert(owner);
    InputManager* input = InputManager::get();

    // Update camera rotation
    {
        auto mouse_delta = input->get_mouse_delta();
        if (input->is_mouse_down(MouseButton::RIGHT) && glm::length(mouse_delta) > 0.f)
        {
            mouse_delta *= look_sensitivity;
            pitch += mouse_delta.y;
            yaw += mouse_delta.x;
        }
        auto new_rotation = pitch_yaw_rotation(pitch, yaw);
        owner->set_rotation(new_rotation);
    }

    // Update camera location
    {
        auto move_axis = vec2(0.f);
        move_axis.x += input->is_key_down(KeyCode::W) ? 1.f : 0.f;
        move_axis.x += input->is_key_down(KeyCode::S) ? -1.f : 0.f;
        move_axis.y += input->is_key_down(KeyCode::D) ? 1.f : 0.f;
        move_axis.y += input->is_key_down(KeyCode::A) ? -1.f : 0.f;
        if (glm::length(move_axis) > 0.f)
        {
            auto velocity =
                glm::normalize(owner->get_forward() * move_axis.x + owner->get_right() * move_axis.y) * move_speed;
            auto move_delta = velocity * dt;
            auto new_location = owner->get_location() + move_delta;
            owner->set_location(new_location);
        }
    }
}

void OrbitCameraControllerComponent::update(float dt)
{
    assert(owner);
    InputManager* input = InputManager::get();
    
    // Update camera rotation
    {
        auto mouse_delta = input->get_mouse_delta();
        if (input->is_mouse_down(MouseButton::LEFT) && glm::length(mouse_delta) > 0.f)
        {
            mouse_delta *= look_sensitivity;
            pitch += mouse_delta.y;
            yaw += mouse_delta.x;
        }
        auto new_rotation = pitch_yaw_rotation(pitch, yaw);
        owner->set_rotation(new_rotation);
    }

    // Update camera location
    {
        auto mouse_scroll_delta = input->get_mouse_scroll_delta();
        if (glm::length(mouse_scroll_delta) > 0.f)
        {
            distance -= mouse_scroll_delta.y * scroll_sensitivity;
            distance = glm::clamp(distance, min_distance, max_distance);
        }
        auto new_location = pivot + owner->get_forward() * -1.f * distance;
        owner->set_location(new_location);
    }
}
} // namespace ash
