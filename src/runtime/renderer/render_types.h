#pragma once

#include "LVK.h"

namespace ash
{
class BufferRing;

struct RenderPassContext
{
    lvk::ICommandBuffer& cmd;
    BufferRing& temp_buffer;
    uint32_t width = 0;
    uint32_t height = 0;

    lvk::Viewport get_viewport() const
    {
        return lvk::Viewport{0.0f, 0.0f, (float)width, (float)height, 0.0f, +1.0f};
    }
    lvk::ScissorRect get_scissor() const
    {
        return lvk::ScissorRect{0, 0, width, height};
    }
};

struct LightInstance
{
    vec3 position;
    float type;
    vec3 color;
    float intensity;
    vec3 direction;
    float range;
    float inner_cone_angle;
    float outer_cone_angle;
};
}
