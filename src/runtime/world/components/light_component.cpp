#include "light_component.h"
#include "spdlog/spdlog.h"

namespace ash
{
GpuLight DirectionalLightComponent::get_gpu_light() const
{
    return GpuLight{
        .position = owner->get_location(),
        .type = 0,
        .color = color,
        .intensity = intensity,
        .direction = owner->get_forward(),
        .range = 0.0f,
        .inner_cone_angle = 0.0f,
        .outer_cone_angle = 0.0f,
    };
}

GpuLight PointLightComponent::get_gpu_light() const
{
    return GpuLight{
        .position = owner->get_location(),
        .type = 1,
        .color = color,
        .intensity = intensity,
        .direction = owner->get_forward(),
        .range = range,
        .inner_cone_angle = 0.0f,
        .outer_cone_angle = 0.0f,
    };
}

GpuLight SpotLightComponent::get_gpu_light() const
{
    return GpuLight{
        .position = owner->get_location(),
        .type = 2,
        .color = color,
        .intensity = intensity,
        .direction = owner->get_forward(),
        .range = range,
        .inner_cone_angle = inner_cone_angle,
        .outer_cone_angle = outer_cone_angle,
    };
}
} // namespace ash
