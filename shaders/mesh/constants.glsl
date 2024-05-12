#include "lighting.glsl"

layout (std430, buffer_reference) readonly buffer PerFrame {
    mat4 proj;
    mat4 view;
    uint sampler0;
    uint padding[3];
    vec3 ambient_light;
    uint lights_num;
    Light lights[MAX_LIGHT_COUNT];
};

layout (std430, buffer_reference) readonly buffer PerObject {
    mat4 model;
};

layout(std430, buffer_reference) readonly buffer Material {
    vec4 base_color_factor;
    float metallic_factor;
    float roughness_factor;
    uint base_color_texture;
    uint metallic_roughness_texture;
    uint alpha_mask;
    float alpha_cutoff;
};

layout (push_constant) uniform PushConstants {
    PerFrame per_frame;
    PerObject per_object;
    Material material;
} pc;
