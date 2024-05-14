#include "constants.glsl"

layout (location=0) in vec3 pos;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 uv;

// output
struct PerVertex {
  vec3 normal;
  vec2 uv;
  vec3 pos;
  vec4 shadow_coords;
};
layout (location=0) out PerVertex vtx;

void main() {
  mat4 proj = pc.per_frame.proj;
  mat4 view = pc.per_frame.view;
  mat4 light = pc.per_frame.light;
  mat4 model = pc.per_object.model;
  gl_Position = proj * view * model * vec4(pos, 1.0);

  // Compute the normal in world-space
  mat3 norm_matrix = transpose(inverse(mat3(model)));
  vtx.normal = normalize(norm_matrix * normal);
  vtx.uv = uv;
  vtx.pos = pos;
  vtx.shadow_coords = light * model * vec4(pos, 1.0);
}
