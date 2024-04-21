#pragma once

#include "world/component.h"
#include "resource/mesh_resource.h"

namespace ash
{
class MeshComponent : public Component
{
  public:
    MeshComponent(const MeshPtr& mesh) : mesh(mesh) {}
    
    MeshPtr mesh;
    
    void on_create() override;
    
    void on_destroy() override;
};
} // namespace ash
