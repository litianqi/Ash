layout (location=0) in vec3 normal;
layout (location=1) in vec2 uv;
layout (location=0) out vec4 out_FragColor;

layout(std430, buffer_reference) readonly buffer PerFrame {
  mat4 proj;
  mat4 view;
  uint sampler0;
};

layout(std430, buffer_reference) readonly buffer PerObject {
  mat4 model;
};

layout(std430, buffer_reference) readonly buffer Material {
  vec4 base_color_factor;
  float metallic_factor;
  float roughness_factor;
  uint base_color_texture;
  uint metallic_roughness_texture;
};

layout(push_constant) uniform constants {
	PerFrame per_frame;
    PerObject per_object;
    Material material;
} pc;

void main() {
  vec4 base_color = textureBindless2D(pc.material.base_color_texture, pc.per_frame.sampler0, uv);
  out_FragColor = base_color;
};