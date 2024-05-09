#pragma once

#include "LVK.h"
#include "offsetAllocator.hpp"

namespace ash
{
class BufferRing
{
  public:
    BufferRing(lvk::IContext* context, uint32_t page_num, uint32_t page_size, const char* name);
    
    // Advance to the next page.
    void advance();
    
    // Allocate data on the free space of the buffer and return the offset relative to the buffer.
    template <typename T>
    uint64_t alloc(const T& data)
    {
        return alloc(&data, sizeof(T));
    }
    
    // Allocate data on the free space of the buffer and return the offset relative to the buffer.
    uint64_t alloc(const void* data, uint32_t size);
    
  private:
    lvk::IContext* context = nullptr;
    uint32_t page_num = 0;
    uint32_t page_size = 0;
    uint32_t page_index = 0;
    lvk::Holder<lvk::BufferHandle> buffer;
    uint64_t buffer_gpu_address = 0;
    uint32_t page_head = 0;
};
} // namespace ash
