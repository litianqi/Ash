#pragma once

#include "LVK.h"
#include "offsetAllocator.hpp"

namespace ash
{
class StorageBuffer
{
  public:
    StorageBuffer(lvk::IContext* context, uint32_t size, const char* name);
    
    // Gets the GPU address of the buffer.
    uint64_t get_gpu_address() const
    {
        return gpu_address;
    }
    
    // Allocate data on the free space of the buffer and return the offset relative to the buffer.
    template <typename T>
    OffsetAllocator::Allocation allocate(const T& data)
    {
        return allocate(&data, sizeof(T));
    }

    // Allocate data on the free space of the buffer and return the offset relative to the buffer.
    OffsetAllocator::Allocation allocate(const void* data, uint32_t size);
    
    // Free the allocated space.
    void free(OffsetAllocator::Allocation allocation);
    
  private:
    lvk::IContext* context = nullptr;
    uint32_t size = 0;
    lvk::Holder<lvk::BufferHandle> buffer;
    uint64_t gpu_address = 0;
    OffsetAllocator::Allocator allocator;
};
} // namespace ash
