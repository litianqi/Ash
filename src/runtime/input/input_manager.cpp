#include "input_manager.h"
#include "app/app.h"

namespace ash
{
InputManager* InputManager::get()
{
    if (auto* app = BaseApp::get())
    {
        return app->get_subsystem<InputManager>();
    }
    return nullptr;
}

void InputManager::tick()
{
    previous_key_down = key_down;
    previous_mouse_down = mouse_down;
    previous_mouse_position = mouse_position;
    mouse_scroll_delta = glm::vec2(0.f, 0.f);
}
}
