#include "input_manager.h"

namespace ash
{
void InputManager::tick()
{
    previous_key_down = key_down;
    previous_mouse_down = mouse_down;
    previous_mouse_position = mouse_position;
    mouse_scroll_delta = glm::vec2(0.f, 0.f);
}
}
