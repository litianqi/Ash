#include "constants.glsl"

struct PerVertex {
    vec3 normal;
    vec2 uv;
    vec3 pos;
    vec4 shadow_coords;
};

layout (location=0) in PerVertex vtx;
layout (location = 0) out vec4 out_FragColor;

float PCF3(vec3 uvw) {
    float size = 1.0 / textureBindlessSize2D(pc.per_frame.shadow_map).x;
    float shadow = 0.0;
    for (int v=-1; v<=+1; v++)
        for (int u=-1; u<=+1; u++)
            shadow += textureBindless2DShadow(pc.per_frame.shadow_map, pc.per_frame.sampler_shadow, uvw + size * vec3(u, v, 0));
    return shadow / 9;
}

float shadow(vec4 s) {
    s = s / s.w;
    if (s.z > -1.0 && s.z < 1.0) {
        float depthBias = -0.00005;
//        float shadowSample = PCF3(vec3(s.x, 1.0 - s.y, s.z + depthBias));
        return mix(0.3, 1.0, shadowSample);
    }
    return 1.0;
}

float calc_shadow(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
//    projCoords = projCoords * 0.5 + 0.5;
    projCoords = vec3(projCoords.x, 1.0 - projCoords.y, projCoords.z);
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = textureBindless2D(pc.per_frame.shadow_map, pc.per_frame.sampler_linear, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main() {
    vec4 base_color = pc.material.base_color_texture > 0 
        ? textureBindless2D(pc.material.base_color_texture, pc.per_frame.sampler_linear, vtx.uv) * pc.material.base_color_factor 
        : pc.material.base_color_factor;

    if (pc.material.alpha_mask == 1) {
        if (base_color.a < pc.material.alpha_cutoff) {
            discard;
        }
    }

    vec3 color = base_color.rgb;
    vec3 ambient = pc.per_frame.ambient_light;
    vec3 diffuse = vec3(0.0);

    for (uint i = 0U; i < pc.per_frame.lights_num; ++i)
    {
        Light light = pc.per_frame.lights[i];
        if (light.type == DIRECTIONAL_LIGHT)
        {
            diffuse += apply_directional_light(light, vtx.normal);
        }
        else if (light.type == POINT_LIGHT)
        {
            diffuse += apply_point_light(light, vtx.pos, vtx.normal);
        }
        else // SPOT_LIGHT
        {
            diffuse += apply_spot_light(light, vtx.pos);
        }
    }
    
    float shadow = calc_shadow(vtx.shadow_coords);
    out_FragColor = vec4((ambient + (1.0 - shadow) * diffuse) * color, base_color.a);
//    out_FragColor = vec4(vec3(1.0) * shadow, base_color.a);
//    out_FragColor = vec4(vtx.shadow_coords.xyz / vtx.shadow_coords.w, base_color.a);
    // out_FragColor = vec4(normal, base_color.a);
    // out_FragColor = vec4(pc.per_frame.lights[0].direction, base_color.a);
};