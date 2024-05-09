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
    
  private:
    std::unique_ptr<ForwardPass> forward_pass;
};
} // namespace ash
