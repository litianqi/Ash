#pragma once
#include <vector>
#include "LVK.h"
#include "HelpersImGui.h"
#include "app/app_subsystem.h"

struct SDL_Window;

namespace ash
{
class Device : public AppSubsystem
{
  public:
    // Returns the singleton instance of the device.
    static Device* get();
    
    Device(SDL_Window* window, uint32_t width, uint32_t height);

    // Resize the swapchain.
    void resize(uint32_t width, uint32_t height);
    
    // Gets the context.
    lvk::IContext* get_context() { return context.get(); }
    
    // Gets the ImGui renderer.
    lvk::ImGuiRenderer* get_imgui() { return imgui.get(); }

  private:
    std::unique_ptr<lvk::IContext> context;
    std::unique_ptr<lvk::ImGuiRenderer> imgui;
};
} // namespace ash
