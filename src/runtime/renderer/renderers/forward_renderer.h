#pragma once

#include "renderer/renderer.h"
#include <vector>
#include "LVK.h"
#include "renderer/passes/forward_pass.h"
#include "renderer/passes/shadow_pass.h"

namespace ash
{
class ForwardRenderer : public Renderer
{
  public:
    ForwardRenderer(Device& device, const RendererDesc& desc);
    void render(const World* world, const CameraComponent* camera, const DirectionalLightComponent* main_light) override;
    
    void set_shader_type(ShaderType type) { shader_type = type; }
    
  private:
    std::unique_ptr<ForwardPass> forward_pass;
    std::unique_ptr<ShadowPass> shadow_pass;
    ShaderType shader_type = ShaderType::SIMPLE_LIT;
};
} // namespace ash
