#include "ash.h"
#include "SDL3/SDL_main.h"
#include "stb/stb_image.h"
#include "glm/ext.hpp"
#include "glm/glm.hpp"
// #include "SDL_timer.h"
// #include "imgui/imgui_demo.cpp"

namespace // types, constants and helper functions
{
struct UniformsPerFrame
{
    mat4 proj;
    mat4 view;
    uint32_t sampler;
};
struct UniformsPerObject
{
    mat4 model;
};

const char* code_vs = R"(
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
)";

const char* code_fs = R"(
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
)";

constexpr uint32_t kNumBufferedFrames = 3;

enum class CameraControllerType : int
{
    Fly = 0,
    Orbit = 1,
};
} // namespace

namespace ash
{
class GltfApp : public BaseApp
{
  public:
    FPSCounter fps_counter;

    std::unique_ptr<World> world;
    std::vector<GameObjectPtr> renderables;
    std::vector<UniformsPerObject> per_object;
    GameObjectPtr camera;
    CameraControllerType camera_controller_type = CameraControllerType::Fly;

    uint32_t frame_index = 0;

    lvk::Holder<lvk::TextureHandle> depth_buffer;
    lvk::Framebuffer framebuffer;
    lvk::Holder<lvk::ShaderModuleHandle> vert;
    lvk::Holder<lvk::ShaderModuleHandle> frag;
    lvk::Holder<lvk::RenderPipelineHandle> render_pipeline;
    std::vector<lvk::Holder<lvk::BufferHandle>> ub_per_frame, ub_per_object;
    lvk::Holder<lvk::SamplerHandle> sampler;
    lvk::RenderPass render_pass;
    lvk::DepthState depth_state;

    void create_depth_buffer()
    {
        auto* context = Device::get()->get_context();

        lvk::TextureDesc descDepth = {
            .type = lvk::TextureType_2D,
            .format = lvk::Format_Z_UN24,
            .dimensions = {display_width, display_height},
            .usage = lvk::TextureUsageBits_Attachment | lvk::TextureUsageBits_Sampled,
            .numMipLevels = lvk::calcNumMipLevels(display_width, display_height),
            .debugName = "Offscreen framebuffer (d)",
        };
        const uint8_t usage =
            lvk::TextureUsageBits_Attachment | lvk::TextureUsageBits_Sampled | lvk::TextureUsageBits_Storage;
        const lvk::Format format = lvk::Format_RGBA_UN8;
        lvk::TextureDesc descColor = {
            .type = lvk::TextureType_2D,
            .format = format,
            .dimensions = {display_width, display_height},
            .usage = usage,
            .numMipLevels = lvk::calcNumMipLevels(display_width, display_height),
            .debugName = "Offscreen framebuffer (color)",
        };
        depth_buffer = context->createTexture(descDepth);
    }

    void startup() override
    {
        BaseApp::startup();
        spdlog::info("Hello, glTF!");

        auto* context = Device::get()->get_context();

        create_depth_buffer();

        depth_state = {.compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true};

        sampler = context->createSampler({.debugName = "Sampler: linear"}, nullptr);

        render_pass = {.color = {{
                           .loadOp = lvk::LoadOp_Clear,
                           .storeOp = lvk::StoreOp_Store,
                           .clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                       }},
                       .depth = {.loadOp = lvk::LoadOp_Clear, .storeOp = lvk::StoreOp_Store, .clearDepth = 1.0}};

        vert = context->createShaderModule({code_vs, lvk::Stage_Vert, "Shader Module: main (vert)"});
        frag = context->createShaderModule({code_fs, lvk::Stage_Frag, "Shader Module: main (frag)"});

        const lvk::VertexInput vdesc = {
            .attributes =
                {
                    {.location = 0, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, position)},
                    {.location = 1, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, normal)},
                    {.location = 2, .format = lvk::VertexFormat::Float2, .offset = offsetof(Vertex, uv)},
                },
            .inputBindings = {{.stride = sizeof(Vertex)}},
        };

        render_pipeline = context->createRenderPipeline(
            {
                .vertexInput = vdesc,
                .smVert = vert,
                .smFrag = frag,
                .color =
                    {
                        {.format = context->getSwapchainFormat()},
                    },
                .depthFormat = context->getFormat(depth_buffer),
                .cullMode = lvk::CullMode_None,
                .frontFaceWinding = lvk::WindingMode_CCW,
                .debugName = "Pipeline: mesh",
            },
            nullptr);

        //> create world & load glTF into world
        world = std::make_unique<World>();
        auto load_model = [&](const fs::path& path) {
            auto model = load_gltf(path, *world);
            if (model.has_value())
            {
                for (auto& go : model->game_objects)
                {
                    if (go->has_component<MeshComponent>())
                    {
                        renderables.push_back(go);
                    }
                }
            }
        };
        // load_model(get_resources_dir() / "BoxTextured/glTF-Binary/BoxTextured.glb");
        // load_model(get_resources_dir() / "FlightHelmet/glTF/FlightHelmet.gltf");
        // load_model(get_resources_dir() / "DamagedHelmet/glTF-Binary/DamagedHelmet.glb");
        // load_gltf(get_resources_dir() / "Bistro_Godot.glb", *world);
        load_model(get_resources_dir() / "Sponza/glTF/Sponza.gltf");
        assert(!renderables.empty());

        //> create camera
        camera = world->create("Camera", vec3(0.f));
        auto* camera_component = camera->add_component<CameraComponent>();
        camera_component->fov = 45.0f * (glm::pi<float>() / 180.0f);
        camera_component->aspect_ratio = (float)display_width / (float)display_height;
        add_or_update_camera_controller();

        //> create a uniform buffers
        per_object.resize(renderables.size());
        for (uint32_t i = 0; i != kNumBufferedFrames; i++)
        {
            ub_per_frame.push_back(context->createBuffer({.usage = lvk::BufferUsageBits_Uniform,
                                                          .storage = lvk::StorageType_HostVisible,
                                                          .size = sizeof(UniformsPerFrame),
                                                          .debugName = "Buffer: uniforms (per frame)"},
                                                         nullptr));
            ub_per_object.push_back(context->createBuffer({.usage = lvk::BufferUsageBits_Uniform,
                                                           .storage = lvk::StorageType_HostVisible,
                                                           .size = renderables.size() * sizeof(UniformsPerObject),
                                                           .debugName = "Buffer: uniforms (per object)"},
                                                          nullptr));
        }
    }

    void cleanup() override
    {
        world = nullptr;
        ub_per_frame.clear();
        ub_per_object.clear();
        vert = nullptr;
        frag = nullptr;
        render_pipeline = nullptr;
        sampler = nullptr;
        depth_buffer = nullptr;
        framebuffer = {};

        BaseApp::cleanup();
    }

    void resize(uint32_t width, uint32_t height) override
    {
        BaseApp::resize(width, height);
        create_depth_buffer();
    }

    void update(float dt) override
    {
        fps_counter.tick(dt);
        frame_index = (frame_index + 1) % kNumBufferedFrames;
        world->update(dt);
    }

    void add_or_update_camera_controller()
    {
        if (camera_controller_type == CameraControllerType::Fly &&
            !camera->has_component<FlyCameraControllerComponent>())
        {
            camera->remove_components<OrbitCameraControllerComponent>();
            camera->add_component<FlyCameraControllerComponent>();
            camera->set_location(vec3(0.0f, 0.0f, -2.0f));
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
        ImGui::Begin("Hello glTF", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
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
        framebuffer = {.color = {{.texture = swapchain_texture}}, .depthStencil = {.texture = depth_buffer}};

        const auto* camera_component = camera->get_component<CameraComponent>();
        const UniformsPerFrame per_frame = {
            .proj = camera_component->get_projection_matrix(),
            .view = camera_component->get_view_matrix(),
            .sampler = sampler.index(),
        };
        context->upload(ub_per_frame[frame_index], &per_frame, sizeof(per_frame));

        for (uint32_t i = 0; i != renderables.size(); i++)
        {
            per_object[i].model = renderables[i]->get_matrix();
        }
        context->upload(ub_per_object[frame_index], per_object.data(), per_object.size() * sizeof(UniformsPerObject));

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
            // Draw cubes: we use uniform buffer to update matrices
            for (uint32_t i = 0; i != renderables.size(); i++)
            {
                auto* mesh_component = renderables[i]->get_component<MeshComponent>();
                buffer.cmdBindVertexBuffer(0, mesh_component->mesh->vertex_buffer);
                buffer.cmdBindIndexBuffer(mesh_component->mesh->index_buffer, lvk::IndexFormat_UI32);
                for (const auto& sub_mesh : mesh_component->mesh->sub_meshes)
                {
                    struct
                    {
                        uint64_t per_frame;
                        uint64_t per_object;
                        uint64_t material;
                    } bindings = {
                        .per_frame = context->gpuAddress(ub_per_frame[frame_index]),
                        .per_object = context->gpuAddress(ub_per_object[frame_index], i * sizeof(UniformsPerObject)),
                        .material = sub_mesh.material->uniform_buffer.get_gpu_address(),
                    };
                    buffer.cmdPushConstants(bindings);
                    buffer.cmdDrawIndexed(sub_mesh.index_count, 1, sub_mesh.index_offset);
                }
            }
            buffer.cmdPopDebugGroupLabel();
        }
        imgui->end_frame(buffer, framebuffer);
        buffer.cmdEndRendering();

        context->submit(buffer, swapchain_texture);
    }
};
} // namespace ash

ASH_CREATE_APPLICATION(ash::GltfApp)
