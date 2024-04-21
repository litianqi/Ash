#pragma once

#include "resource.h"
#include "LVK.h"
#include "material_resource.h"

namespace ash
{
struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 uv;
#if ASH_LOAD_VERTEX_COLORS
    vec4 color;
#endif
};

struct Bounds {
    vec3 origin;
    float sphere_radius;
    vec3 extents;
};

struct SubMesh
{
    uint32_t index_offset;
    uint32_t index_count;
//    uint32_t vertex_offset;
//    uint32_t vertex_count;
    MaterialPtr material;
    Bounds bounds;
};

class MeshResource : public Resource
{
  public:
    lvk::Holder<lvk::BufferHandle> vertex_buffer;
    lvk::Holder<lvk::BufferHandle> index_buffer;
    std::vector<SubMesh> sub_meshes;
};

using MeshPtr = ResourcePtr<MeshResource>;
} // namespace ash
