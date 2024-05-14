#pragma once

#include "core/math.h"
#include "gfx/buffer_ring.h"

namespace ash
{
class Device;
class World;
class CameraComponent;
class DirectionalLightComponent;

struct RendererDesc
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t temp_buffer_size = 1024 * 1024;
};

// Defines a series of commands and settings that describes how Ash renders a frame.
// Renderer is similar to Unity's `RenderPipeline`, or UE5's `FSceneRenderer`.
class Renderer
{
  public:
    explicit Renderer(Device& device, const RendererDesc& desc);
    virtual ~Renderer() = default;
    
    static inline auto SWAPCHAIN_FORMAT = lvk::Format_Invalid;
    static inline auto DEPTH_FORMAT = lvk::Format_Z_UN24;
    static inline auto SHADOW_MAP_FORMAT = lvk::Format_Z_UN16;

    virtual void resize(uint32_t new_width, uint32_t new_height);
    virtual void render(const World* world, const CameraComponent* camera, const DirectionalLightComponent* main_light) = 0;

  protected:
    void create_depth_buffer();
    void create_shadow_map();
    
    uint32_t width = 0;
    uint32_t height = 0;
    std::unique_ptr<BufferRing> temp_buffer;
    lvk::Holder<lvk::TextureHandle> depth_buffer;
    lvk::Holder<lvk::TextureHandle> shadow_map;
    lvk::Framebuffer shadow_framebuffer;
    lvk::Holder<lvk::SamplerHandle> sampler_linear;
    lvk::Holder<lvk::SamplerHandle> sampler_shadow;
};
} // namespace ash
