#include "constants.glsl"

layout (location=0) in vec3 normal;
layout (location=1) in vec2 uv;
layout (location=0) out vec4 out_FragColor;

void main() {
  vec4 base_color = textureBindless2D(pc.material.base_color_texture, pc.per_frame.sampler0, uv);
  if (pc.material.alpha_mask == 1) {
      if (base_color.a < pc.material.alpha_cutoff) {
          discard;
      }
  }
  out_FragColor = base_color;
};