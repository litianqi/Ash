#include "ash.h"
#include "SDL3/SDL_main.h"

namespace ash
{
class EmptyApp : public BaseApp
{
  public:
    FPSCounter fps_counter;
    
    void startup() override
    {
        BaseApp::startup();
        auto* context = Device::get()->get_context();
        spdlog::info("EmptyApp started!");
    }
    
    void cleanup() override
    {
        BaseApp::cleanup();
        spdlog::info("EmptyApp stopped!");
    }

    void update(float dt) override
    {
        fps_counter.tick(dt);
    }
    
    void render_ui() override
    {
        ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("FPS:    %.2f", fps_counter.get_fps());
        ImGui::End();
    }

    void render() override
    {
        ZoneScoped;
        
        auto* context = Device::get()->get_context();
        auto* imgui = Device::get()->get_imgui();
        
        lvk::TextureHandle swapchain_texture = context->getCurrentSwapchainTexture();
        lvk::Framebuffer framebuffer = {
            .color = {{.texture = swapchain_texture}},
        };
        lvk::RenderPass render_pass = {.color = {{
                           .loadOp = lvk::LoadOp_Clear,
                           .storeOp = lvk::StoreOp_Store,
                           .clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                       }}};

        lvk::ICommandBuffer& command_buffer = context->acquireCommandBuffer();
        
        command_buffer.cmdBeginRendering(render_pass, framebuffer);
        imgui->end_frame(command_buffer, framebuffer);
        command_buffer.cmdEndRendering();
        
        context->submit(command_buffer, swapchain_texture);
    }
};
}

ASH_CREATE_APPLICATION(ash::EmptyApp)

