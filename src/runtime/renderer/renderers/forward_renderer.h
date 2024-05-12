#pragma once

#include "renderer/renderer.h"
#include <vector>
#include "LVK.h"
#include "renderer/passes/forward_pass.h"

namespace ash
{
class ForwardRenderer : public Renderer
{
  public:
    ForwardRenderer(Device& device, const RendererDesc& desc);
    void render(const World* world, const CameraComponent* camera) override;
    
    void set_shader_type(ShaderType type) { shader_type = type; }
    
  private:
    std::unique_ptr<ForwardPass> forward_pass;
    ShaderType shader_type = ShaderType::SIMPLE_LIT;
};
} // namespace ash
