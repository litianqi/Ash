layout (location=0) in vec3 pos;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 uv;
layout (location=0) out vec3 out_normal;
layout (location=1) out vec2 out_uv;

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
  mat4 proj = pc.per_frame.proj;
  mat4 view = pc.per_frame.view;
  mat4 model = pc.per_object.model;
  gl_Position = proj * view * model * vec4(pos, 1.0);

  // Compute the normal in world-space
  mat3 norm_matrix = transpose(inverse(mat3(model)));
  out_normal = normalize(norm_matrix * normal);
  out_uv = uv;
}