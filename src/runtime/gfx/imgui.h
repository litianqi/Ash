#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <lvk/LVK.h>
#include <imgui/imgui.h>

struct SDL_Window;

namespace ash {
class ImGuiRenderer {
  public:
    explicit ImGuiRenderer(lvk::IContext& device, const char* default_font_ttf = nullptr, float font_size_pixels = 24.0f);
    ~ImGuiRenderer();
    
    
    void resize(uint32_t width, uint32_t height);
    
    void update_font(const char* default_font_ttf, float font_size_pixels);

    void begin_frame();
    void end_frame(lvk::ICommandBuffer& cmd, const lvk::Framebuffer& fb);

  private:
    lvk::Holder<lvk::RenderPipelineHandle> create_pipeline_state(lvk::Format color_format, lvk::Format depth_format);

  private:
    lvk::IContext& ctx_;
    lvk::Holder<lvk::ShaderModuleHandle> vert_;
    lvk::Holder<lvk::ShaderModuleHandle> frag_;
    lvk::Holder<lvk::RenderPipelineHandle> pipeline_;
    lvk::Holder<lvk::TextureHandle> fontTexture_;
    uint32_t nonLinearColorSpace_ = 0;

    uint32_t frameIndex_ = 0;

    struct DrawableData {
        lvk::Holder<lvk::BufferHandle> vb_;
        lvk::Holder<lvk::BufferHandle> ib_;
        uint32_t numAllocatedIndices_ = 0;
        uint32_t numAllocatedVerteices_ = 0;
    };

    DrawableData drawables_[18] = {};
};

} // namespace lvk
