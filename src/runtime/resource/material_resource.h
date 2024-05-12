#pragma once

#include "resource.h"
#include "texture_resource.h"
#include "core/math.h"
#include "gfx/buffer_pool.h"

namespace ash
{
struct alignas(16) GpuMaterial
{
    alignas(16) vec4 base_color_factor = {0.0f, 0.0f, 0.0f, 0.0f};
    float metallic_factor = 0.0f;
    float roughness_factor = 0.0f;
    uint32_t base_color_texture = 0;
    uint32_t metallic_roughness_texture = 0;
    uint32_t alpha_mask = 0;
    float alpha_cutoff = 0.0f;
//    float _padding[2];
};

// How the alpha value of the main factor and texture should be interpreted
enum class AlphaMode
{
    OPAQUE, // < Alpha value is ignored
    MASK,   // < Either full opaque or fully transparent
    BLEND   // < Output is combined with the background
};

class MaterialResource : public Resource
{
  public:
    vec4 base_color_factor = vec4(0.0f);
    float metallic_factor = 0.0f;
    float roughness_factor = 0.0f;
    TexturePtr base_color_texture;
    TexturePtr metallic_roughness_texture;
    AlphaMode alpha_mode = AlphaMode::OPAQUE;
    float alpha_cutoff = 0.5f;
    bool double_sided = false;
    BufferSlice uniform_buffer;
};

using MaterialPtr = ResourcePtr<MaterialResource>;
} // namespace ash
