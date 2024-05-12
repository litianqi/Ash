#include "constants.glsl"

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 pos;
layout (location = 0) out vec4 out_FragColor;

void main() {
    vec4 base_color = pc.material.base_color_texture > 0 
        ? textureBindless2D(pc.material.base_color_texture, pc.per_frame.sampler0, uv) * pc.material.base_color_factor 
        : pc.material.base_color_factor;

    if (pc.material.alpha_mask == 1) {
        if (base_color.a < pc.material.alpha_cutoff) {
            discard;
        }
    }

    vec3 diffuse_color = base_color.rgb;
    vec3 light_contribution = pc.per_frame.ambient_light * diffuse_color;

    for (uint i = 0U; i < pc.per_frame.lights_num; ++i)
    {
        Light light = pc.per_frame.lights[i];
        if (light.type == DIRECTIONAL_LIGHT)
        {
            light_contribution += apply_directional_light(light, normal) * diffuse_color;
        }
        else if (light.type == POINT_LIGHT)
        {
            light_contribution += apply_point_light(light, pos, normal) * diffuse_color;
        }
        else // SPOT_LIGHT
        {
            light_contribution += apply_spot_light(light, pos) * diffuse_color;
        }
    }
    
    out_FragColor = vec4(light_contribution, base_color.a);
    // out_FragColor = vec4(normal, base_color.a);
    // out_FragColor = vec4(pc.per_frame.lights[0].direction, base_color.a);
};