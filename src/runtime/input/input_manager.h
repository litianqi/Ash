#pragma once

#include <set>
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_mouse.h"
#include "glm/glm.hpp"
#include "app/app_subsystem.h"

using glm::vec2;

namespace ash
{
class BaseApp;

using KeyCode  = SDL_Keycode;

enum class MouseButton : uint8_t
{
    INVALID = 0,

    LEFT = SDL_BUTTON_LEFT,
    RIGHT = SDL_BUTTON_RIGHT,
    MIDDLE = SDL_BUTTON_MIDDLE,
    X1 = SDL_BUTTON_X1,
    X2 = SDL_BUTTON_X2,

    COUNT
};

class InputManager : public AppSubsystem
{
  public:
    // Returns the singleton instance of the input manager.
    static InputManager* get();

    // Updates the input manager state. This should be called at the start of each frame.
    void tick();

    // Returns whether the given key code is held down.
    bool is_key_down(KeyCode key) const
    {
        return key_down.contains(key);
    }

    // Returns true during the frame the user pressed the given key code.
    bool is_key_pressed(KeyCode key) const
    {
        return key_down.contains(key) && !previous_key_down.contains(key);
    }

    // Returns true during the frame the user releases the given key code.
    bool is_key_released(KeyCode key) const
    {
        return !key_down.contains(key) && previous_key_down.contains(key);
    }

    // Returns whether the given mouse button is held down.
    bool is_mouse_down(MouseButton button) const
    {
        return mouse_down.contains(button);
    }

    // Returns true during the frame the user pressed the given mouse button.
    bool is_mouse_pressed(MouseButton button) const
    {
        return mouse_down.contains(button) && !previous_mouse_down.contains(button);
    }

    // Returns true during the frame the user releases the given mouse button.
    bool is_mouse_released(MouseButton button) const
    {
        return !mouse_down.contains(button) && previous_mouse_down.contains(button);
    }

    // Returns the current mouse position, relative to window.
    vec2 get_mouse_position() const
    {
        return mouse_position;
    }

    // Returns the current mouse delta.
    vec2 get_mouse_delta() const
    {
        return mouse_position - previous_mouse_position;
    }

    // Returns the current mouse scroll delta.
    vec2 get_mouse_scroll_delta() const
    {
        return mouse_scroll_delta;
    }

  private:
    void on_key_down(KeyCode key)
    {
        key_down.insert(key);
    }
    
    void on_key_up(KeyCode key)
    {
        key_down.erase(key);
    }
    
    void on_mouse_down(MouseButton mouse)
    {
        mouse_down.insert(mouse);
    }
    
    void on_mouse_up(MouseButton mouse)
    {
        mouse_down.erase(mouse);
    }

    void on_mouse_move(const vec2& position)
    {
        mouse_position = position;
    }

    void on_mouse_scroll(const vec2& scroll_delta)
    {
        mouse_scroll_delta = scroll_delta;
    }
    
    std::set<KeyCode> key_down;
    std::set<KeyCode> previous_key_down;

    std::set<MouseButton> mouse_down;
    std::set<MouseButton> previous_mouse_down;

    vec2 mouse_position = vec2(0.0f);
    vec2 previous_mouse_position = vec2(0.0f);

    vec2 mouse_scroll_delta = vec2(0.0f);
    
    friend int run_application(BaseApp& app, int argc, char* argv[]);
};
} // namespace ash
