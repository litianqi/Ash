/*
 * LightweightVK
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "HelpersImGui.h"

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_tables.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/backends/imgui_impl_sdl3.h"
#if defined(LVK_WITH_IMPLOT)
#include "implot/implot.cpp"
#include "implot/implot_items.cpp"
#include "imgui.h"

#endif // LVK_WITH_IMPLOT

#include <math.h>

static const char* codeVS = R"(
layout (location = 0) out vec4 out_color;
layout (location = 1) out vec2 out_uv;
layout (location = 2) out flat uint out_textureId;

struct Vertex {
  float x, y;
  float u, v;
  uint rgba;
};

layout(std430, buffer_reference) readonly buffer VertexBuffer {
  Vertex vertices[];
};

layout(push_constant) uniform PushConstants {
  vec4 LRTB;
  VertexBuffer vb;
  uint textureId;
} pc;

void main() {
  float L = pc.LRTB.x;
  float R = pc.LRTB.y;
  float T = pc.LRTB.z;
  float B = pc.LRTB.w;
  mat4 proj = mat4(
    2.0 / (R - L),                   0.0,  0.0, 0.0,
    0.0,                   2.0 / (T - B),  0.0, 0.0,
    0.0,                             0.0, -1.0, 0.0,
    (R + L) / (L - R), (T + B) / (B - T),  0.0, 1.0);
  Vertex v = pc.vb.vertices[gl_VertexIndex];
  out_color = unpackUnorm4x8(v.rgba);
  out_uv = vec2(v.u, v.v);
  out_textureId = pc.textureId;
  gl_Position = proj * vec4(v.x, v.y, 0, 1);
})";

static const char* codeFS = R"(
layout (location = 0) in vec4 in_color;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in flat uint in_textureId;

layout (location = 0) out vec4 out_color;

layout (constant_id = 0) const bool kNonLinearColorSpace = false;

void main() {
  vec4 c = in_color * texture(sampler2D(kTextures2D[in_textureId], kSamplers[0]), in_uv);
  // Render UI in linear color space to sRGB framebuffer.
  out_color = kNonLinearColorSpace ? vec4(pow(c.rgb, vec3(2.2)), c.a) : c;
})";

namespace ash
{

lvk::Holder<lvk::RenderPipelineHandle> ImGuiRenderer::create_pipeline_state(lvk::Format color_format,
                                                                            lvk::Format depth_format)
{
    nonLinearColorSpace_ = ctx_.getSwapChainColorSpace() == lvk::ColorSpace_SRGB_NONLINEAR ? 1u : 0u;
    return ctx_.createRenderPipeline(
        {
            .smVert = vert_,
            .smFrag = frag_,
            .specInfo = {.entries = {{.constantId = 0, .size = sizeof(nonLinearColorSpace_)}},
                         .data = &nonLinearColorSpace_,
                         .dataSize = sizeof(nonLinearColorSpace_)},
            .color = {{
                .format = color_format,
                .blendEnabled = true,
                .srcRGBBlendFactor = lvk::BlendFactor_SrcAlpha,
                .dstRGBBlendFactor = lvk::BlendFactor_OneMinusSrcAlpha,
            }},
            .depthFormat = depth_format,
            .cullMode = lvk::CullMode_None,
        },
        nullptr);
}

ImGuiRenderer::ImGuiRenderer(lvk::IContext& device, const char* default_font_ttf, float font_size_pixels) : ctx_(device)
{
    ImGui::CreateContext();
#if defined(LVK_WITH_IMPLOT)
    ImPlot::CreateContext();
#endif // LVK_WITH_IMPLOT

    // this initializes imgui for SDL
    // TODO: ImGui_ImplSDL3_InitForVulkan(desc.window);

    update_font(default_font_ttf, font_size_pixels);

    vert_ = ctx_.createShaderModule({codeVS, lvk::Stage_Vert, "Shader Module: imgui (vert)"});
    frag_ = ctx_.createShaderModule({codeFS, lvk::Stage_Frag, "Shader Module: imgui (frag)"});
}

ImGuiRenderer::~ImGuiRenderer()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->TexID = nullptr;
#if defined(LVK_WITH_IMPLOT)
    ImPlot::DestroyContext();
#endif // LVK_WITH_IMPLOT
    ImGui::DestroyContext();
}

void ImGuiRenderer::update_font(const char* default_font_ttf, float font_size_pixels)
{
    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig cfg = ImFontConfig();
    cfg.FontDataOwnedByAtlas = false;
    cfg.RasterizerMultiply = 1.5f;
    cfg.SizePixels = ceilf(font_size_pixels);
    cfg.PixelSnapH = true;
    cfg.OversampleH = 4;
    cfg.OversampleV = 4;
    ImFont* font = nullptr;
    if (default_font_ttf)
    {
        font = io.Fonts->AddFontFromFileTTF(default_font_ttf, cfg.SizePixels, &cfg);
    }

    io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;

    // init fonts
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    fontTexture_ = ctx_.createTexture({.type = lvk::TextureType_2D,
                                       .format = lvk::Format_RGBA_UN8,
                                       .dimensions = {(uint32_t)width, (uint32_t)height},
                                       .usage = lvk::TextureUsageBits_Sampled,
                                       .data = pixels},
                                      nullptr);
    io.Fonts->TexID = ImTextureID(fontTexture_.indexAsVoid());
    io.FontDefault = font;
}


void ImGuiRenderer::resize(uint32_t width, uint32_t height)
{
    ImGui::GetIO().DisplaySize = ImVec2(width, height);
}

void ImGuiRenderer::begin_frame()
{
    ImGui::NewFrame();
}

void ImGuiRenderer::end_frame(lvk::ICommandBuffer& cmd, const lvk::Framebuffer& fb)
{
    static_assert(sizeof(ImDrawIdx) == 2);
    LVK_ASSERT_MSG(sizeof(ImDrawIdx) == 2, "The constants below may not work with the ImGui data.");

    if (pipeline_.empty()) {
        auto color_format = ctx_.getFormat(fb.color[0].texture);
        auto depth_format = fb.depthStencil.texture ? ctx_.getFormat(fb.depthStencil.texture) : lvk::Format_Invalid;
        pipeline_ = create_pipeline_state(color_format, depth_format);
    }

    ImGui::EndFrame();
    ImGui::Render();

    ImDrawData* dd = ImGui::GetDrawData();

    int fb_width = (int)(dd->DisplaySize.x * dd->FramebufferScale.x);
    int fb_height = (int)(dd->DisplaySize.y * dd->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0 || dd->CmdListsCount == 0)
    {
        return;
    }

    cmd.cmdPushDebugGroupLabel("ImGui Rendering", 0xff00ff00);
    cmd.cmdBindDepthState({});
    cmd.cmdBindViewport({
        .x = 0.0f,
        .y = 0.0f,
        .width = (dd->DisplaySize.x * dd->FramebufferScale.x),
        .height = (dd->DisplaySize.y * dd->FramebufferScale.y),
    });

    const float L = dd->DisplayPos.x;
    const float R = dd->DisplayPos.x + dd->DisplaySize.x;
    const float T = dd->DisplayPos.y;
    const float B = dd->DisplayPos.y + dd->DisplaySize.y;

    const ImVec2 clip_off = dd->DisplayPos;
    const ImVec2 clip_scale = dd->FramebufferScale;

    DrawableData& drawableData = drawables_[frameIndex_];
    frameIndex_ = (frameIndex_ + 1) % LVK_ARRAY_NUM_ELEMENTS(drawables_);

    if (drawableData.numAllocatedIndices_ < dd->TotalIdxCount)
    {
        drawableData.ib_ = ctx_.createBuffer({
            .usage = lvk::BufferUsageBits_Index,
            .storage = lvk::StorageType_HostVisible,
            .size = dd->TotalIdxCount * sizeof(ImDrawIdx),
            .debugName = "ImGui: drawableData.ib_",
        });
        drawableData.numAllocatedIndices_ = dd->TotalIdxCount;
    }
    if (drawableData.numAllocatedVerteices_ < dd->TotalVtxCount)
    {
        drawableData.vb_ = ctx_.createBuffer({
            .usage = lvk::BufferUsageBits_Storage,
            .storage = lvk::StorageType_HostVisible,
            .size = dd->TotalVtxCount * sizeof(ImDrawVert),
            .debugName = "ImGui: drawableData.vb_",
        });
        drawableData.numAllocatedVerteices_ = dd->TotalVtxCount;
    }

    // upload vertex/index buffers
    {
        ImDrawVert* vtx = (ImDrawVert*)ctx_.getMappedPtr(drawableData.vb_);
        uint16_t* idx = (uint16_t*)ctx_.getMappedPtr(drawableData.ib_);
        for (int n = 0; n < dd->CmdListsCount; n++)
        {
            const ImDrawList* cmdList = dd->CmdLists[n];
            memcpy(vtx, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx += cmdList->VtxBuffer.Size;
            idx += cmdList->IdxBuffer.Size;
        }
        ctx_.flushMappedMemory(drawableData.vb_, 0, dd->TotalVtxCount * sizeof(ImDrawVert));
        ctx_.flushMappedMemory(drawableData.ib_, 0, dd->TotalIdxCount * sizeof(ImDrawIdx));
    }

    uint32_t idxOffset = 0;
    uint32_t vtxOffset = 0;

    cmd.cmdBindIndexBuffer(drawableData.ib_, lvk::IndexFormat_UI16);
    cmd.cmdBindRenderPipeline(pipeline_);

    for (int n = 0; n < dd->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = dd->CmdLists[n];

        for (int cmd_i = 0; cmd_i < cmdList->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd draw_cmd = cmdList->CmdBuffer[cmd_i];
            LVK_ASSERT(draw_cmd.UserCallback == nullptr);

            ImVec2 clipMin((draw_cmd.ClipRect.x - clip_off.x) * clip_scale.x,
                           (draw_cmd.ClipRect.y - clip_off.y) * clip_scale.y);
            ImVec2 clipMax((draw_cmd.ClipRect.z - clip_off.x) * clip_scale.x,
                           (draw_cmd.ClipRect.w - clip_off.y) * clip_scale.y);
            // clang-format off
            if (clipMin.x < 0.0f) clipMin.x = 0.0f;
            if (clipMin.y < 0.0f) clipMin.y = 0.0f;
            if (clipMax.x > fb_width ) clipMax.x = (float)fb_width;
            if (clipMax.y > fb_height) clipMax.y = (float)fb_height;
            if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) continue;
            // clang-format on
            struct VulkanImguiBindData
            {
                float LRTB[4]; // ortho projection: left, right, top, bottom
                uint64_t vb = 0;
                uint32_t textureId = 0;
            } bindData = {
                .LRTB = {L, R, T, B},
                .vb = ctx_.gpuAddress(drawableData.vb_),
                .textureId = static_cast<uint32_t>(reinterpret_cast<ptrdiff_t>(draw_cmd.TextureId)),
            };
            cmd.cmdPushConstants(bindData);
            cmd.cmdBindScissorRect({uint32_t(clipMin.x), uint32_t(clipMin.y), uint32_t(clipMax.x - clipMin.x),
                                    uint32_t(clipMax.y - clipMin.y)});
            cmd.cmdDrawIndexed(draw_cmd.ElemCount, 1u, idxOffset + draw_cmd.IdxOffset, int32_t(vtxOffset + draw_cmd.VtxOffset));
        }
        idxOffset += cmdList->IdxBuffer.Size;
        vtxOffset += cmdList->VtxBuffer.Size;
    }

    cmd.cmdPopDebugGroupLabel();
}
} // namespace ash
