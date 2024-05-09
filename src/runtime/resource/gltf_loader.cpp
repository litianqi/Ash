#include "gltf_loader.h"
#include "mesh_resource.h"
#include "fastgltf/core.hpp"
#include "fastgltf/glm_element_traits.hpp"
#include "stb/stb_image.h"
#include "spdlog/spdlog.h"
#include "gfx/device.h"
#include "world/world.h"
#include "world/components/mesh_component.h"

namespace ash
{
TexturePtr load_texture(lvk::IContext* context, const fs::path& base_dir, fastgltf::Asset& asset,
                        fastgltf::Image& image)
{
    lvk::Holder<lvk::TextureHandle> texture;

    int width, height, channels;

    std::visit(
        fastgltf::visitor{
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filePath) {
                assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                assert(filePath.uri.isLocalPath());   // We're only capable of loading
                                                      // local files.

                const std::string path(filePath.uri.path().begin(),
                                       filePath.uri.path().end()); // Thanks C++.
                unsigned char* data = stbi_load((base_dir / path).string().c_str(), &width, &height, &channels, 4);
                if (data)
                {
                    texture = context->createTexture(
                        {
                            .type = lvk::TextureType_2D,
                            .format = lvk::Format_RGBA_UN8,
                            .dimensions = {(uint32_t)width, (uint32_t)height},
                            .usage = lvk::TextureUsageBits_Sampled,
                            .data = data,
                            .debugName = image.name.c_str(),
                        },
                        nullptr);

                    stbi_image_free(data);
                }
            },
            [&](fastgltf::sources::Vector& vector) {
                unsigned char* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()),
                                                            &width, &height, &channels, 4);
                if (data)
                {
                    texture = context->createTexture(
                        {
                            .type = lvk::TextureType_2D,
                            .format = lvk::Format_RGBA_UN8,
                            .dimensions = {(uint32_t)width, (uint32_t)height},
                            .usage = lvk::TextureUsageBits_Sampled,
                            .data = data,
                            .debugName = image.name.c_str(),
                        },
                        nullptr);

                    stbi_image_free(data);
                }
            },
            [&](fastgltf::sources::BufferView& view) {
                auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                auto& buffer = asset.buffers[bufferView.bufferIndex];

                // Yes, we've already loaded every buffer into some GL buffer. However, with GL it's simpler
                // to just copy the buffer data again for the texture. Besides, this is just an example.
                std::visit(
                    fastgltf::visitor{
                        // We only care about VectorWithMime here, because we specify LoadExternalBuffers, meaning
                        // all buffers are already loaded into a vector.
                        [](auto& arg) {},
                        [&](fastgltf::sources::Array& vector) {
                            int width, height, channels;
                            unsigned char* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
                                                                        static_cast<int>(bufferView.byteLength), &width,
                                                                        &height, &channels, 4);
                            texture = context->createTexture(
                                {
                                    .type = lvk::TextureType_2D,
                                    .format = lvk::Format_RGBA_UN8,
                                    .dimensions = {(uint32_t)width, (uint32_t)height},
                                    .usage = lvk::TextureUsageBits_Sampled,
                                    .data = data,
                                    .debugName = image.name.c_str(),
                                },
                                nullptr);
                            stbi_image_free(data);
                        }},
                    buffer.data);
            },
        },
        image.data);

    if (texture.valid())
    {
        TexturePtr new_image = create_resource<TextureResource>();
        new_image->texture = std::move(texture);
        return new_image;
    }
    return {};
}

lvk::SamplerFilter extract_filter(fastgltf::Filter filter)
{
    switch (filter)
    {
    // nearest samplers
    case fastgltf::Filter::Nearest:
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::NearestMipMapLinear:
        return lvk::SamplerFilter_Nearest;

    // linear samplers
    case fastgltf::Filter::Linear:
    case fastgltf::Filter::LinearMipMapNearest:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return lvk::SamplerFilter_Linear;
    }
}

lvk::SamplerMip extract_mipmap_mode(fastgltf::Filter filter)
{
    switch (filter)
    {
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::LinearMipMapNearest:
        return lvk::SamplerMip_Nearest;

    case fastgltf::Filter::NearestMipMapLinear:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return lvk::SamplerMip_Linear;
    }
}

std::optional<GltfModel> load_gltf(const fs::path& path, World& world)
{
    auto* device = Device::get();
    assert(device);
    auto* context = device->get_context();
    assert(context);

    GltfModel model;
    fastgltf::Parser parser{};
    fs::path base_dir = path.parent_path();

    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
                                 fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;
    // fastgltf::Options::LoadExternalImages;

    fastgltf::GltfDataBuffer data;
    data.loadFromFile(path);

    fastgltf::Asset gltf;

    auto type = fastgltf::determineGltfFileType(&data);
    if (type == fastgltf::GltfType::glTF)
    {
        auto load = parser.loadGltf(&data, path.parent_path(), gltfOptions);
        if (load)
        {
            gltf = std::move(load.get());
        }
        else
        {
            spdlog::error("Failed to load glTF: {}", fastgltf::to_underlying(load.error()));
            return {};
        }
    }
    else if (type == fastgltf::GltfType::GLB)
    {
        auto load = parser.loadGltfBinary(&data, path.parent_path(), gltfOptions);
        if (load)
        {
            gltf = std::move(load.get());
        }
        else
        {
            spdlog::error("Failed to load glTF: {}", fastgltf::to_underlying(load.error()));
            return {};
        }
    }
    else
    {
        spdlog::error("Failed to determine glTF container");
        return {};
    }

    //> load samplers
    for (fastgltf::Sampler& gltf_sampler : gltf.samplers)
    {
        auto sampler = context->createSampler(
            {.minFilter = extract_filter(gltf_sampler.minFilter.value_or(fastgltf::Filter::Nearest)),
             .magFilter = extract_filter(gltf_sampler.magFilter.value_or(fastgltf::Filter::Nearest)),
             .mipMap = extract_mipmap_mode(gltf_sampler.minFilter.value_or(fastgltf::Filter::Nearest)),
             .debugName = "Sampler: linear"},
            nullptr);

        model.samplers.push_back(std::move(sampler));
    }

    //> create a white texture
    auto white_texture = create_resource<TextureResource>();
    const uint32_t pixel = 0xFFFFFFFF;
    white_texture->texture = context->createTexture(
        {
            .type = lvk::TextureType_2D,
            .format = lvk::Format_R_UN8,
            .dimensions = {1, 1},
            .usage = lvk::TextureUsageBits_Sampled,
            .swizzle = {lvk::Swizzle_1, lvk::Swizzle_1, lvk::Swizzle_1, lvk::Swizzle_1},
            .data = &pixel,
            .debugName = "Texture: 1x1 white",
        },
        nullptr);

    //> load all textures
    for (fastgltf::Image& gltf_image : gltf.images)
    {
        auto texture = load_texture(context, base_dir, gltf, gltf_image);
        if (texture)
        {
            model.textures.push_back(texture);
        }
        else
        {
            model.textures.push_back(white_texture);
            spdlog::error("gltf failed to load texture {}", gltf_image.name);
        }
    }

    //> load_material
    for (fastgltf::Material& gltf_material : gltf.materials)
    {
        auto material = create_resource<MaterialResource>();
        material->name = gltf_material.name;
        model.materials.push_back(material);

        material->base_color_factor.x = gltf_material.pbrData.baseColorFactor[0];
        material->base_color_factor.y = gltf_material.pbrData.baseColorFactor[1];
        material->base_color_factor.z = gltf_material.pbrData.baseColorFactor[2];
        material->base_color_factor.w = gltf_material.pbrData.baseColorFactor[3];
        material->metallic_factor = gltf_material.pbrData.metallicFactor;
        material->roughness_factor = gltf_material.pbrData.roughnessFactor;
        material->base_color_texture = white_texture;
        material->metallic_roughness_texture = white_texture;
        material->alpha_mode = static_cast<AlphaMode>(gltf_material.alphaMode);
        material->alpha_cutoff = gltf_material.alphaCutoff;
        material->double_sided = gltf_material.doubleSided;
        if (gltf_material.pbrData.baseColorTexture.has_value())
        {
            auto image_index = *gltf.textures[gltf_material.pbrData.baseColorTexture->textureIndex].imageIndex;
            material->base_color_texture = model.textures[image_index];
        }
        if (gltf_material.pbrData.metallicRoughnessTexture.has_value())
        {
            auto image_index = *gltf.textures[gltf_material.pbrData.metallicRoughnessTexture->textureIndex].imageIndex;
            material->metallic_roughness_texture = model.textures[image_index];
        }

        GpuMaterial gpu_material{};
        gpu_material.base_color_factor = material->base_color_factor;
        gpu_material.metallic_factor = material->metallic_factor;
        gpu_material.roughness_factor = material->roughness_factor;
        gpu_material.base_color_texture = material->base_color_texture->texture.index();
        gpu_material.metallic_roughness_texture = material->metallic_roughness_texture->texture.index();
        material->uniform_buffer = device->get_persist_buffer()->alloc(gpu_material);
    }

    //> create a default material
    auto default_material = create_resource<MaterialResource>();
    default_material->base_color_texture = white_texture;
    default_material->metallic_roughness_texture = white_texture;
    GpuMaterial gpu_material{};
    gpu_material.base_color_texture = default_material->base_color_texture->texture.index();
    gpu_material.metallic_roughness_texture = default_material->metallic_roughness_texture->texture.index();
    default_material->uniform_buffer = device->get_persist_buffer()->alloc(gpu_material);

    //> load all meshes
    // use the same vectors for all meshes so that the memory doesnt reallocate as
    // often
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    for (fastgltf::Mesh& gltf_mesh : gltf.meshes)
    {
        auto mesh = create_resource<MeshResource>();
        model.meshes.push_back(mesh);
        mesh->name = gltf_mesh.name;

        // clear the mesh arrays each mesh, we dont want to merge them by error
        indices.clear();
        vertices.clear();

        for (auto&& p : gltf_mesh.primitives)
        {
            SubMesh sub_mesh;
            sub_mesh.index_offset = (uint32_t)indices.size();
            sub_mesh.index_count = (uint32_t)gltf.accessors[*p.indicesAccessor].count;

            size_t initial_vtx = vertices.size();

            // load indexes
            {
                fastgltf::Accessor& indices_accessor = gltf.accessors[*p.indicesAccessor];
                indices.reserve(indices.size() + indices_accessor.count);

                fastgltf::iterateAccessor<std::uint32_t>(
                    gltf, indices_accessor, [&](std::uint32_t idx) { indices.push_back(idx + initial_vtx); });
            }

            // load vertex positions
            {
                fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                vertices.resize(vertices.size() + posAccessor.count);

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor, [&](glm::vec3 v, size_t index) {
                    Vertex vertex{};
                    vertex.position = v;
                    vertex.normal = {1, 0, 0};
                    vertex.uv = {0, 0};
#if ASH_LOAD_VERTEX_COLORS
                    vertex.color = vec4{1.f};
#endif
                    vertices[initial_vtx + index] = vertex;
                });
            }

            // load vertex normals
            auto normals = p.findAttribute("NORMAL");
            if (normals != p.attributes.end())
            {

                fastgltf::iterateAccessorWithIndex<glm::vec3>(
                    gltf, gltf.accessors[(*normals).second],
                    [&](glm::vec3 v, size_t index) { vertices[initial_vtx + index].normal = v; });
            }

            // load UVs
            auto uv = p.findAttribute("TEXCOORD_0");
            if (uv != p.attributes.end())
            {

                fastgltf::iterateAccessorWithIndex<glm::vec2>(
                    gltf, gltf.accessors[(*uv).second],
                    [&](glm::vec2 v, size_t index) { vertices[initial_vtx + index].uv = v; });
            }

            // load vertex colors
#if ASH_LOAD_VERTEX_COLORS
            auto colors = p.findAttribute("COLOR_0");
            if (colors != p.attributes.end())
            {

                fastgltf::iterateAccessorWithIndex<glm::vec4>(
                    gltf, gltf.accessors[(*colors).second],
                    [&](glm::vec4 v, size_t index) { vertices[initial_vtx + index].color = v; });
            }
#endif

            if (p.materialIndex.has_value())
            {
                sub_mesh.material = model.materials[*p.materialIndex];
            }
            else
            {
                sub_mesh.material = default_material;
            }

            glm::vec3 minpos = vertices[initial_vtx].position;
            glm::vec3 maxpos = vertices[initial_vtx].position;
            for (int i = initial_vtx; i < vertices.size(); i++)
            {
                minpos = glm::min(minpos, vertices[i].position);
                maxpos = glm::max(maxpos, vertices[i].position);
            }

            sub_mesh.bounds.origin = (maxpos + minpos) / 2.f;
            sub_mesh.bounds.extents = (maxpos - minpos) / 2.f;
            sub_mesh.bounds.sphere_radius = glm::length(sub_mesh.bounds.extents);
            mesh->sub_meshes.push_back(sub_mesh);
        }
        mesh->vertex_buffer = context->createBuffer({.usage = lvk::BufferUsageBits_Vertex,
                                                     .storage = lvk::StorageType_Device,
                                                     .size = sizeof(Vertex) * vertices.size(),
                                                     .data = vertices.data(),
                                                     .debugName = "Buffer: vertex"},
                                                    nullptr);
        mesh->index_buffer = context->createBuffer({.usage = lvk::BufferUsageBits_Index,
                                                    .storage = lvk::StorageType_Device,
                                                    .size = sizeof(uint32_t) * indices.size(),
                                                    .data = indices.data(),
                                                    .debugName = "Buffer: index"},
                                                   nullptr);
    }

    //> load_nodes
    // load all nodes and their meshes
    for (fastgltf::Node& gltf_node : gltf.nodes)
    {
        auto game_object = world.create(gltf_node.name.c_str(), vec3{0.f});

        // find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the meshnode
        // class
        if (gltf_node.meshIndex.has_value())
        {
            game_object->add_component<MeshComponent>(model.meshes[*gltf_node.meshIndex]);
        }

        model.game_objects.push_back(game_object);

        std::visit(fastgltf::visitor{[&](fastgltf::Node::TransformMatrix gltf_matrix) {
                                         glm::mat4 matrix = glm::make_mat4(gltf_matrix.data());
                                         game_object->set_local_matrix(matrix);
                                     },
                                     [&](fastgltf::TRS transform) {
                                         glm::vec3 translation(transform.translation[0], transform.translation[1],
                                                               transform.translation[2]);
                                         glm::quat rotation(transform.rotation[3], transform.rotation[0],
                                                            transform.rotation[1], transform.rotation[2]);
                                         glm::vec3 scale(transform.scale[0], transform.scale[1], transform.scale[2]);
                                         game_object->set_local_matrix(mat4_compose(scale, rotation, translation));
                                     }},
                   gltf_node.transform);
    }

    //> load hierarchy
    // run loop again to setup transform hierarchy
    for (int i = 0; i < gltf.nodes.size(); i++)
    {
        fastgltf::Node& gltf_node = gltf.nodes[i];
        auto game_object = model.game_objects[i];
        for (auto& child_index : gltf_node.children)
        {
            game_object->add_child(model.game_objects[child_index]);
        }
    }

    // find the top nodes, with no parents
    for (auto& game_object : model.game_objects)
    {
        if (!game_object->get_parent())
        {
            model.top_game_objects.push_back(game_object);
        }
    }

    return model;
}
} // namespace ash
