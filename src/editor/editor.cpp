#include "app.h"
#include "core/fps_counter.h"
#include "SDL3/SDL_main.h"

namespace ash
{
class EditorApp : public BaseApp
{
  public:
    FPSCounter fps_counter;
    
    void startup() override
    {
        BaseApp::startup();
        spdlog::info("EditorApp started!");
    }
    
    void cleanup() override
    {
        BaseApp::cleanup();
        spdlog::info("EditorApp stopped!");
    }

    void update(float dt) override
    {
        fps_counter.tick(dt);
    }

    void render() override
    {
        LVK_PROFILER_FUNCTION();
        
        lvk::TextureHandle swapchain_texture = context->getCurrentSwapchainTexture();
        lvk::Framebuffer framebuffer = {
            .color = {{.texture = swapchain_texture}},
        };
        lvk::RenderPass render_pass = {.color = {{
                                           .loadOp = lvk::LoadOp_Clear,
                                           .storeOp = lvk::StoreOp_Store,
                                           .clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                                       }}};

        imgui->beginFrame(framebuffer);

        ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("FPS:    %.2f", fps_counter.get_fps());
        ImGui::End();
        
        lvk::ICommandBuffer& command_buffer = context->acquireCommandBuffer();
        
        command_buffer.cmdBeginRendering(render_pass, framebuffer);
        imgui->endFrame(command_buffer);
        command_buffer.cmdEndRendering();
        
        context->submit(command_buffer, swapchain_texture);
    }
};
}

ASH_CREATE_APPLICATION(ash::EditorApp)

