#include "ash.h"
#include "SDL3/SDL_main.h"
#include "stb/stb_image.h"
#include "glm/ext.hpp"
#include "glm/glm.hpp"
// #include "SDL_timer.h"
// #include "imgui/imgui_demo.cpp"

using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

#define ASH_CUBE_USE_DEPTH_BUFFER 1

namespace // types, constants and helper functions
{
struct VertexPosUvw
{
    vec3 pos;
    vec3 color;
    vec2 uv;
};

struct UniformsPerFrame
{
    mat4 proj;
    mat4 view;
    uint32_t texture0;
    uint32_t texture1;
    uint32_t sampler;
};
struct UniformsPerObject
{
    mat4 model;
};

const char* code_vs = R"(
layout (location=0) out vec3 color;
layout (location=1) out vec2 uv;

struct Vertex {
  float x, y, z;
  float r, g, b;
  vec2 uv;
};

layout(std430, buffer_reference) readonly buffer VertexBuffer {
  Vertex vertices[];
};

layout(std430, buffer_reference) readonly buffer PerFrame {
  mat4 proj;
  mat4 view;
  uint texture0;
  uint texture1;
  uint sampler0;
};

layout(std430, buffer_reference) readonly buffer PerObject {
  mat4 model;
};

layout(push_constant) uniform constants {
	PerFrame per_frame;
	PerObject per_object;
  VertexBuffer vb;
} pc;

void main() {
  mat4 proj = pc.per_frame.proj;
  mat4 view = pc.per_frame.view;
  mat4 model = pc.per_object.model;
  Vertex v = pc.vb.vertices[gl_VertexIndex];
  gl_Position = proj * view * model * vec4(v.x, v.y, v.z, 1.0);
  color = vec3(v.r, v.g, v.b);
  uv = v.uv;
}
)";

const char* code_fs = R"(
layout (location=0) in vec3 color;
layout (location=1) in vec2 uv;
layout (location=0) out vec4 out_FragColor;

layout(std430, buffer_reference) readonly buffer PerFrame {
  mat4 proj;
  mat4 view;
  uint texture0;
  uint texture1;
  uint sampler0;
};

layout(push_constant) uniform constants {
	PerFrame per_frame;
} pc;

void main() {
  vec4 t0 = textureBindless2D(pc.per_frame.texture0, pc.per_frame.sampler0, 2.0*uv);
  vec4 t1 = textureBindless2D(pc.per_frame.texture1, pc.per_frame.sampler0, uv);
  out_FragColor = vec4(color * (t0.rgb + t1.rgb), 1.0);
};
)";

const float half = 1.0f;
// UV-mapped cube with indices: 24 vertices, 36 indices
VertexPosUvw vertex_data[] = {
    // top
    {{-half, -half, +half}, {0.0, 0.0, 1.0}, {0, 0}}, // 0
    {{+half, -half, +half}, {1.0, 0.0, 1.0}, {1, 0}}, // 1
    {{+half, +half, +half}, {1.0, 1.0, 1.0}, {1, 1}}, // 2
    {{-half, +half, +half}, {0.0, 1.0, 1.0}, {0, 1}}, // 3
                                                      // bottom
    {{-half, -half, -half}, {1.0, 1.0, 1.0}, {0, 0}}, // 4
    {{-half, +half, -half}, {0.0, 1.0, 0.0}, {0, 1}}, // 5
    {{+half, +half, -half}, {1.0, 1.0, 0.0}, {1, 1}}, // 6
    {{+half, -half, -half}, {1.0, 0.0, 0.0}, {1, 0}}, // 7
                                                      // left
    {{+half, +half, -half}, {1.0, 1.0, 0.0}, {1, 0}}, // 8
    {{-half, +half, -half}, {0.0, 1.0, 0.0}, {0, 0}}, // 9
    {{-half, +half, +half}, {0.0, 1.0, 1.0}, {0, 1}}, // 10
    {{+half, +half, +half}, {1.0, 1.0, 1.0}, {1, 1}}, // 11
                                                      // right
    {{-half, -half, -half}, {1.0, 1.0, 1.0}, {0, 0}}, // 12
    {{+half, -half, -half}, {1.0, 0.0, 0.0}, {1, 0}}, // 13
    {{+half, -half, +half}, {1.0, 0.0, 1.0}, {1, 1}}, // 14
    {{-half, -half, +half}, {0.0, 0.0, 1.0}, {0, 1}}, // 15
                                                      // front
    {{+half, -half, -half}, {1.0, 0.0, 0.0}, {0, 0}}, // 16
    {{+half, +half, -half}, {1.0, 1.0, 0.0}, {1, 0}}, // 17
    {{+half, +half, +half}, {1.0, 1.0, 1.0}, {1, 1}}, // 18
    {{+half, -half, +half}, {1.0, 0.0, 1.0}, {0, 1}}, // 19
                                                      // back
    {{-half, +half, -half}, {0.0, 1.0, 0.0}, {1, 0}}, // 20
    {{-half, -half, -half}, {1.0, 1.0, 1.0}, {0, 0}}, // 21
    {{-half, -half, +half}, {0.0, 0.0, 1.0}, {0, 1}}, // 22
    {{-half, +half, +half}, {0.0, 1.0, 1.0}, {1, 1}}, // 23
};

uint16_t index_data[] = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
                         12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};

constexpr uint32_t kNumCubes = 16;
constexpr uint32_t kNumBufferedFrames = 3;

lvk::Holder<lvk::TextureHandle> load_texture(lvk::IContext& context, const fs::path& path)
{
    int32_t tex_width = 0;
    int32_t tex_height = 0;
    int32_t channels = 0;
    uint8_t* pixels = stbi_load(path.string().c_str(), &tex_width, &tex_height, &channels, 4);
    if (!pixels)
    {
        spdlog::error("Unable to load image, please check if the file \"{}\" exists", path.string());
        return {};
    }
    auto texture = context.createTexture(
        {
            .type = lvk::TextureType_2D,
            .format = lvk::Format_RGBA_UN8,
            .dimensions = {(uint32_t)tex_width, (uint32_t)tex_height},
            .usage = lvk::TextureUsageBits_Sampled,
            .data = pixels,
            .debugName = path.filename().string().c_str(),
        },
        nullptr);
    stbi_image_free(pixels);
    return texture;
}

lvk::Holder<lvk::TextureHandle> create_xor_pattern_texture(lvk::IContext& context)
{
    const uint32_t tex_width = 256;
    const uint32_t tex_height = 256;
    std::vector<uint32_t> pixels(tex_width * tex_height);
    for (uint32_t y = 0; y != tex_height; y++)
    {
        for (uint32_t x = 0; x != tex_width; x++)
        {
            // create a XOR pattern
            pixels[y * tex_width + x] = 0xFF000000 + ((x ^ y) << 16) + ((x ^ y) << 8) + (x ^ y);
        }
    }
    return context.createTexture(
        {
            .type = lvk::TextureType_2D,
            .format = lvk::Format_BGRA_UN8,
            .dimensions = {tex_width, tex_height},
            .usage = lvk::TextureUsageBits_Sampled,
            .data = pixels.data(),
            .debugName = "XOR pattern",
        },
        nullptr);
}

enum class CameraControllerType : int
{
    Fly = 0,
    Orbit = 1,
};
} // namespace

namespace ash
{
class CameraApp : public BaseApp
{
  public:
    FPSCounter fps_counter;

    World world;
    GameObjectPtr cubes[kNumCubes] = {};
    GameObjectPtr camera;
    CameraControllerType camera_controller_type = CameraControllerType::Fly;
    UniformsPerObject per_object[kNumCubes] = {};

    uint32_t frame_index = 0;

    lvk::Framebuffer framebuffer;
    lvk::Holder<lvk::ShaderModuleHandle> vert;
    lvk::Holder<lvk::ShaderModuleHandle> frag;
    lvk::Holder<lvk::RenderPipelineHandle> render_pipeline;
    lvk::Holder<lvk::BufferHandle> vb, ib; // buffers for vertices and indices
    std::vector<lvk::Holder<lvk::BufferHandle>> ub_per_frame, ub_per_object;
    lvk::Holder<lvk::TextureHandle> texture0, texture1;
    lvk::Holder<lvk::SamplerHandle> sampler;
    lvk::RenderPass render_pass;
    lvk::DepthState depth_state;

    void startup() override
    {
        BaseApp::startup();
        spdlog::info("Hello, Cube!");

        auto* context = Device::get()->get_context();

        // Vertex buffer, Index buffer and Vertex Input. Buffers are allocated in GPU memory.
        vb = context->createBuffer({.usage = lvk::BufferUsageBits_Storage,
                                    .storage = lvk::StorageType_Device,
                                    .size = sizeof(vertex_data),
                                    .data = vertex_data,
                                    .debugName = "Buffer: vertex"},
                                   nullptr);
        ib = context->createBuffer({.usage = lvk::BufferUsageBits_Index,
                                    .storage = lvk::StorageType_Device,
                                    .size = sizeof(index_data),
                                    .data = index_data,
                                    .debugName = "Buffer: index"},
                                   nullptr);
        // create a Uniform buffers to store uniforms for 2 objects
        for (uint32_t i = 0; i != kNumBufferedFrames; i++)
        {
            ub_per_frame.push_back(context->createBuffer({.usage = lvk::BufferUsageBits_Uniform,
                                                          .storage = lvk::StorageType_HostVisible,
                                                          .size = sizeof(UniformsPerFrame),
                                                          .debugName = "Buffer: uniforms (per frame)"},
                                                         nullptr));
            ub_per_object.push_back(context->createBuffer({.usage = lvk::BufferUsageBits_Uniform,
                                                           .storage = lvk::StorageType_HostVisible,
                                                           .size = kNumCubes * sizeof(UniformsPerObject),
                                                           .debugName = "Buffer: uniforms (per object)"},
                                                          nullptr));
        }

        depth_state = {.compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true};

        texture0 = create_xor_pattern_texture(*context);
        texture1 = load_texture(*context, get_resources_dir() / fs::path("wood_polished_01_diff.png"));

        sampler = context->createSampler({.debugName = "Sampler: linear"}, nullptr);

        render_pass = {.color = {{
                           .loadOp = lvk::LoadOp_Clear,
                           .storeOp = lvk::StoreOp_Store,
                           .clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                       }}};
#if ASH_CUBE_USE_DEPTH_BUFFER
        render_pass.depth = {.loadOp = lvk::LoadOp_Clear, .clearDepth = 1.0};
#else
        render_pass.depth = {.loadOp = lvk::LoadOp_DontCare};
#endif // ASH_CUBE_USE_DEPTH_BUFFER

        vert = context->createShaderModule({code_vs, lvk::Stage_Vert, "Shader Module: main (vert)"});
        frag = context->createShaderModule({code_fs, lvk::Stage_Frag, "Shader Module: main (frag)"});

        render_pipeline = context->createRenderPipeline(
            {
                .smVert = vert,
                .smFrag = frag,
                .color =
                    {
                        {.format = context->getSwapchainFormat()},
                    },
                .depthFormat = framebuffer.depthStencil.texture ? context->getFormat(framebuffer.depthStencil.texture)
                                                                : lvk::Format_Invalid,
                .cullMode = lvk::CullMode_Back,
                .frontFaceWinding = lvk::WindingMode_CW,
                .debugName = "Pipeline: mesh",
            },
            nullptr);

        // Setup world
        const auto cubes_in_line = (uint32_t)sqrt(kNumCubes);
        for (int32_t i = 0; i < kNumCubes; i++)
        {
            const vec3 location = vec3(-1.5f * sqrt(kNumCubes) + 4.0f * (i % cubes_in_line),
                                       -1.5f * sqrt(kNumCubes) + 4.0f * (i / cubes_in_line), 0);
            cubes[i] = world.create(std::format("Cube {}", i), location);
        }
        camera = world.create("Camera", vec3(0.f));
        auto* camera_component = camera->add_component<CameraComponent>();
        camera_component->fov = 45.0f * (glm::pi<float>() / 180.0f);
        camera_component->aspect_ratio = (float)display_width / (float)display_height;
        add_or_update_camera_controller();
    }

    void cleanup() override
    {
        vb = nullptr;
        ib = nullptr;
        ub_per_frame.clear();
        ub_per_object.clear();
        vert = nullptr;
        frag = nullptr;
        render_pipeline = nullptr;
        texture0 = nullptr;
        texture1 = nullptr;
        sampler = nullptr;
        framebuffer = {};

        BaseApp::cleanup();
    }

    void update(float dt) override
    {
        fps_counter.tick(dt);
        frame_index = (frame_index + 1) % kNumBufferedFrames;
        world.update(dt);
    }

    void add_or_update_camera_controller()
    {
        if (camera_controller_type == CameraControllerType::Fly &&
            !camera->has_component<FlyCameraControllerComponent>())
        {
            camera->remove_components<OrbitCameraControllerComponent>();
            camera->add_component<FlyCameraControllerComponent>();
            // place a "camera" behind the cubes, the distance depends on the total number of cubes
            camera->set_location(vec3(0.0f, 0.0f, -1.f * sqrtf(kNumCubes / 16) * 20.0f * half));
        }
        else if (camera_controller_type == CameraControllerType::Orbit &&
                 !camera->has_component<OrbitCameraControllerComponent>())
        {
            camera->remove_components<FlyCameraControllerComponent>();
            auto* obit = camera->add_component<OrbitCameraControllerComponent>();
            obit->distance = 2.f;
        }
    }

    void render_ui() override
    {
        ImGui::Begin("Hello Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("FPS:    %.2f", fps_counter.get_fps());
        ImGui::Separator();
        ImGui::RadioButton("Fly Camera", (int*)&camera_controller_type, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Orbit Camera", (int*)&camera_controller_type, 1);
        add_or_update_camera_controller();
        ImGui::End();
    }

    void render() override
    {
        ZoneScoped;

        auto* context = Device::get()->get_context();
        auto* imgui = Device::get()->get_imgui();

        lvk::TextureHandle swapchain_texture = context->getCurrentSwapchainTexture();
        framebuffer = {
            .color = {{.texture = swapchain_texture}},
        };

        const auto* camera_component = camera->get_component<CameraComponent>();
        const UniformsPerFrame per_frame = {
            .proj = camera_component->get_projection_matrix(),
            .view = camera_component->get_view_matrix(),
            .texture0 = texture0.index(),
            .texture1 = texture1.index(),
            .sampler = sampler.index(),
        };
        context->upload(ub_per_frame[frame_index], &per_frame, sizeof(per_frame));

        for (uint32_t i = 0; i != kNumCubes; i++)
        {
            per_object[i].model = cubes[i]->get_matrix();
        }
        context->upload(ub_per_object[frame_index], &per_object, sizeof(per_object));

        // Command buffers (1-N per thread): create, submit and forget
        lvk::ICommandBuffer& buffer = context->acquireCommandBuffer();

        const lvk::Viewport viewport = {0.0f, 0.0f, (float)display_width, (float)display_height, 0.0f, +1.0f};
        const lvk::ScissorRect scissor = {0, 0, (uint32_t)display_width, (uint32_t)display_height};

        // This will clear the framebuffer
        buffer.cmdBeginRendering(render_pass, framebuffer);
        {
            buffer.cmdBindRenderPipeline(render_pipeline);
            buffer.cmdBindViewport(viewport);
            buffer.cmdBindScissorRect(scissor);
            buffer.cmdPushDebugGroupLabel("Render Mesh", 0xff0000ff);
            buffer.cmdBindDepthState(depth_state);
            buffer.cmdBindIndexBuffer(ib, lvk::IndexFormat_UI16);
            // Draw cubes: we use uniform buffer to update matrices
            for (uint32_t i = 0; i != kNumCubes; i++)
            {
                struct
                {
                    uint64_t per_frame;
                    uint64_t per_object;
                    uint64_t vb;
                } bindings = {
                    .per_frame = context->gpuAddress(ub_per_frame[frame_index]),
                    .per_object = context->gpuAddress(ub_per_object[frame_index], i * sizeof(UniformsPerObject)),
                    .vb = context->gpuAddress(vb),
                };
                buffer.cmdPushConstants(bindings);
                buffer.cmdDrawIndexed(3 * 6 * 2);
            }
            buffer.cmdPopDebugGroupLabel();
        }
        imgui->end_frame(buffer, framebuffer);
        buffer.cmdEndRendering();

        context->submit(buffer, swapchain_texture);
    }
};
} // namespace ash

ASH_CREATE_APPLICATION(ash::CameraApp)
