#pragma once

#include "render_object.h"

namespace ash
{
struct OpaqueSorter
{
    bool operator()(const RenderObject& a, const RenderObject& b) const
    {
        if (a.material == b.material)
        {
            return a.index_buffer.index() < b.index_buffer.index();
        }
        else
        {
            return a.material < b.material;
        }
    }

};

struct TransparentSorter
{
    TransparentSorter(const vec3& camera_location) : camera_location(camera_location) {}

    bool operator()(const RenderObject& a, const RenderObject& b) const
    {
        const float dist_a = glm::length(mat4_decompose_translation(a.transform) - camera_location);
        const float dist_b = glm::length(mat4_decompose_translation(b.transform) - camera_location);
        return dist_a > dist_b;
    }

    vec3 camera_location;
};

struct RenderList
{
    std::vector<RenderObject> objects;

    template <typename Sorter>
    void sort(Sorter sorter)
    {
        std::sort(objects.begin(), objects.end(), sorter);
    }
};
} // namespace ash
