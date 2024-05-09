#pragma once
#include <vector>
#include "LVK.h"
#include "imgui.h"
#include "app/app_subsystem.h"
#include "buffer_pool.h"

struct SDL_Window;

namespace ash
{
class Device : public AppSubsystem
{
  public:
    // Returns the singleton instance of the device.
    static Device* get();
    
    Device(SDL_Window* window, uint32_t width, uint32_t height, uint32_t persist_buffer_size = 12 * 1024 * 1024);

    // Resize the swapchain.
    void resize(uint32_t new_width, uint32_t new_height);
    
    // Gets the context.
    lvk::IContext* get_context() const { return context.get(); }
    
    // Gets the ImGui renderer.
    ImGuiRenderer* get_imgui() const { return imgui.get(); }
    
    uint32_t get_width() const { return width; }
    
    uint32_t get_height() const { return height; }
    
    template <typename T>
    BufferSlice create_persist_buffer(const T& data)
    {
        return persist_buffer->alloc(&data, sizeof(T));
    }
    
    BufferSlice create_persist_buffer(const void* data, uint32_t size)
    {
        return persist_buffer->alloc(data, size);
    }
    
    BufferPool* get_persist_buffer() { return persist_buffer.get(); }

  private:
    std::unique_ptr<lvk::IContext> context;
    std::unique_ptr<ImGuiRenderer> imgui;
    uint32_t width = 0;
    uint32_t height = 0;
    lvk::Holder<lvk::TextureHandle> white_texture;
    lvk::Holder<lvk::SamplerHandle> linear_sampler;
    std::unique_ptr<BufferPool> persist_buffer;
//    OffsetAllocator::Allocation default_material;
};
} // namespace ash
