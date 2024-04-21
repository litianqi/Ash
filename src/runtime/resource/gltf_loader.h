#pragma once

#include <filesystem>
#include "world/game_object.h"
#include "mesh_resource.h"

namespace fs = std::filesystem;
namespace lvk
{
class IContext;
} // namespace lvk

namespace ash
{
class World;

struct GltfModel
{
    std::vector<lvk::Holder<lvk::SamplerHandle>> samplers;
    std::vector<TexturePtr> textures;
    std::vector<MaterialPtr> materials;
    std::vector<MeshPtr> meshes;
    std::vector<GameObjectPtr> game_objects;
    std::vector<GameObjectPtr> top_game_objects;
};

// Load a glTF file into world and return a list of (root) game objects.
std::optional<GltfModel> load_gltf(const fs::path& path, World& world);
} // namespace ash
