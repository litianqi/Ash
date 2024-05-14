#pragma once

#include <vector>
#include "LVK.h"
#include "core/math.h"
#include "renderer/render_types.h"
#include "renderer/render_list.h"

namespace ash
{
enum class ShaderType
{
    UNLIT,
    SIMPLE_LIT,
};

class ForwardPass
{
  public:
    struct PassData
    {
        mat4 proj = mat4(1.0f);
        mat4 view = mat4(1.0f);
        mat4 light = mat4(1.0f);
        lvk::SamplerHandle sampler_linear;
        lvk::SamplerHandle sampler_shadow;
        lvk::TextureHandle shadow_map;
        ShaderType shader_type = ShaderType::SIMPLE_LIT;
        RenderList& opaque;
        RenderList& transparent;
        vec3 ambient_light = vec3(0.2f);
        std::vector<GpuLight>& lights;
    };
    
    explicit ForwardPass(lvk::IContext& context);
    void render(const RenderPassContext& context, const PassData& data);
    
  private:
    lvk::Holder<lvk::ShaderModuleHandle> unlit_vert;
    lvk::Holder<lvk::ShaderModuleHandle> unlit_frag;
    lvk::Holder<lvk::RenderPipelineHandle> unlit_opaque_pipeline;
    lvk::Holder<lvk::RenderPipelineHandle> unlit_transparent_pipeline;
    
    lvk::Holder<lvk::ShaderModuleHandle> simple_lit_vert;
    lvk::Holder<lvk::ShaderModuleHandle> simple_lit_frag;
    lvk::Holder<lvk::RenderPipelineHandle> simple_lit_opaque_pipeline;
    lvk::Holder<lvk::RenderPipelineHandle> simple_lit_transparent_pipeline;
};
} // namespace ash
