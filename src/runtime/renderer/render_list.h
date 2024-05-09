#pragma once

#include "render_object.h"

namespace ash
{
struct RenderList
{
    std::vector<RenderObject> objects;
    
    static bool opaque_sort(const RenderObject& a, const RenderObject& b) {
        if (a.material == b.material) {
            return a.index_buffer.index() < b.index_buffer.index();
        }
        else {
            return a.material < b.material;
        }
    };

    template <typename Compare>
    void sort(Compare comp)
    {
        std::sort(objects.begin(), objects.end(), comp);
    }
};
} // namespace ash
