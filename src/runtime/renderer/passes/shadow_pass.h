#pragma once

#include <vector>
#include "LVK.h"
#include "core/math.h"
#include "renderer/render_types.h"
#include "renderer/render_list.h"

namespace ash
{
class ShadowPass
{
  public:
    struct PassData
    {
        mat4 light = mat4(1.0f);
        RenderList& opaque;
        RenderList& transparent;
    };
    
    explicit ShadowPass(lvk::IContext& context);
    void render(const RenderPassContext& context, const PassData& data);
    
  private:
    lvk::Holder<lvk::ShaderModuleHandle> shadow_vert;
    lvk::Holder<lvk::ShaderModuleHandle> shadow_frag;
    lvk::Holder<lvk::RenderPipelineHandle> shadow_opaque_pipeline;
    lvk::Holder<lvk::RenderPipelineHandle> shadow_transparent_pipeline;
};
} // namespace ash
