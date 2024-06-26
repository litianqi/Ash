#pragma once

#include <variant>
#include "world/component.h"
#include "core/math.h"
#include "renderer/render_types.h"

namespace ash
{
class LightComponent : public Component
{
  public:
    virtual GpuLight get_gpu_light() const = 0;
};

class DirectionalLightComponent : public LightComponent
{
  public:
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    
    GpuLight get_gpu_light() const override;
};

class PointLightComponent : public LightComponent
{
  public:
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float range = 0.0f;
    
    GpuLight get_gpu_light() const override;
};

class SpotLightComponent : public LightComponent
{
  public:
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float range = 0.0f;
    float inner_cone_angle = 0.0f;
    float outer_cone_angle = 0.0f;
    
    GpuLight get_gpu_light() const override;
};
} // namespace ash
