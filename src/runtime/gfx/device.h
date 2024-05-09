#pragma once
#include <vector>
#include "LVK.h"
#include "imgui.h"
#include "app/app_subsystem.h"
#include "storage_buffer.h"

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
    lvk::IContext* get_context() const { return context.get(); }
    
    // Gets the ImGui renderer.
    ImGuiRenderer* get_imgui() const { return imgui.get(); }
    
    // Gets the white texture.
    lvk::TextureHandle get_white_texture() const { return white_texture; }
    
    // Gets the  linear sampler.
    lvk::SamplerHandle get_linear_sampler() const { return linear_sampler; }
    
    // Returns the material buffer.
    StorageBuffer* get_material_buffer() const { return material_buffer.get(); }

  private:
    std::unique_ptr<lvk::IContext> context;
    std::unique_ptr<ImGuiRenderer> imgui;
    lvk::Holder<lvk::TextureHandle> white_texture;
    lvk::Holder<lvk::SamplerHandle> linear_sampler;
    std::unique_ptr<StorageBuffer> material_buffer;
//    OffsetAllocator::Allocation default_material;
};
} // namespace ash
