#pragma once

#include "core/math.h"
#include "LVK.h"

namespace ash
{
struct RenderObject
{
    // SubMesh
    lvk::BufferHandle vertex_buffer;
    lvk::BufferHandle index_buffer;
    uint32_t index_offset = 0;
    uint32_t index_count = 0;
    Bounds bounds;
    
    // Material
    uint64_t material = 0;
    
    // Transform
    mat4 transform = mat4(1.0f);
};
} // namespace ash
