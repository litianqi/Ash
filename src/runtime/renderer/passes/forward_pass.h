#pragma once

#include <vector>
#include "LVK.h"
#include "core/math.h"
#include "renderer/render_types.h"
#include "renderer/render_list.h"

namespace ash
{
class ForwardPass
{
  public:
    struct PassData
    {
        mat4 proj = mat4(1.0f);
        mat4 view = mat4(1.0f);
        lvk::SamplerHandle sampler;
        RenderList& opaque;
        RenderList& transparent;
    };
    
    explicit ForwardPass(lvk::IContext& context);
    void render(const RenderPassContext& context, const PassData& data);
    
  private:
    lvk::Holder<lvk::ShaderModuleHandle> vert;
    lvk::Holder<lvk::ShaderModuleHandle> frag;
    lvk::Holder<lvk::RenderPipelineHandle> render_pipeline;
};
} // namespace ash
