#pragma once

#include "LVK.h"

namespace ash
{
class BufferRing;

struct RenderPassContext
{
    lvk::ICommandBuffer& cmd;
    BufferRing& temp_buffer;
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
    mat4 light;
    uint32_t sampler_linear;
    uint32_t sampler_shadow;
    uint32_t shadow_map;
    uint32_t padding;
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
