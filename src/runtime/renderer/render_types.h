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

struct alignas(16) GpuLight
{
    vec3 position;
    uint32_t type; // 0: directional, 1: point, 2: spot
    vec3 color;
    float intensity;
    vec3 direction;
    float range;
    float inner_cone_angle;
    float outer_cone_angle;
    float _pad[2];
};

constexpr uint32_t MAX_LIGHT_COUNT = 16u;

struct alignas(16) GlobalUniforms
{
    mat4 proj;
    mat4 view;
    uint32_t sampler;
    uint32_t padding[3];
    vec3 ambient_light;
    uint32_t lights_num;
    ash::GpuLight lights[MAX_LIGHT_COUNT];
};

struct ObjectUniforms
{
    mat4 model;
};

struct alignas(16) PushConstants
{
    uint64_t per_frame = 0;
    uint64_t per_object = 0;
    uint64_t material = 0;
};
}
