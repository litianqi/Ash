const uint DIRECTIONAL_LIGHT = 0;
const uint POINT_LIGHT = 1;
const uint SPOT_LIGHT = 2;

const uint MAX_LIGHT_COUNT = 16;

struct Light
{
    vec3 position;
    uint type;
    vec3 color;
    float intensity;
    vec3 direction;
    float range;
    float inner_cone_angle;
    float outer_cone_angle;
    float _pad[2];
};

vec3 apply_directional_light(Light light, vec3 normal)
{
    vec3 world_to_light = normalize(-light.direction);
    float ndotl         = clamp(dot(normal, world_to_light), 0.0, 1.0);
    return ndotl * light.intensity * light.color;
}

vec3 apply_point_light(Light light, vec3 pos, vec3 normal)
{
    vec3  world_to_light = light.position - pos;
    float dist           = length(world_to_light) * 0.005;
    float atten          = 1.0 / (dist * dist);
    world_to_light       = normalize(world_to_light);
    float ndotl          = clamp(dot(normal, world_to_light), 0.0, 1.0);
    return ndotl * light.intensity * atten * light.color;
}

vec3 apply_spot_light(Light light, vec3 pos)
{
    vec3  light_to_pixel   = normalize(pos - light.position);
    float theta            = dot(light_to_pixel, normalize(light.direction));
    float intensity        = (theta - light.outer_cone_angle) / (light.inner_cone_angle - light.outer_cone_angle);
    return smoothstep(0.0, 1.0, intensity) * light.intensity * light.color;
}
