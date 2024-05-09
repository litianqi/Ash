#pragma once

#include "LVK.h"
#include "offsetAllocator.hpp"

namespace ash
{
class BufferPool;

class BufferSlice
{
  public:
    BufferSlice() = default;
    BufferSlice(BufferPool* pool, uint64_t gpu_address, OffsetAllocator::Allocation allocation);
    ~BufferSlice();
    
    BufferSlice(BufferSlice& other) = delete;
    BufferSlice& operator=(BufferSlice& other) = delete;
    
    BufferSlice(BufferSlice&& other) noexcept;
    BufferSlice& operator=(BufferSlice&& other) noexcept;

    uint64_t get_gpu_address() const
    {
        return gpu_address;
    }

    uint32_t get_offset() const
    {
        return offset;
    }

  private:
    BufferPool* pool = nullptr;
    uint64_t gpu_address = 0;
    uint32_t offset = 0;
    uint32_t metadata = 0;
    friend BufferPool;
};

class BufferPool
{
  public:
    BufferPool(lvk::IContext* context, uint32_t size, const char* name);

    // Allocate data on the free space of the buffer and return the offset relative to the buffer.
    template <typename T>
    BufferSlice alloc(const T& data)
    {
        return alloc(&data, sizeof(T));
    }

    // Allocate data on the free space of the buffer and return the offset relative to the buffer.
    BufferSlice alloc(const void* data, uint32_t size);

    // Free the allocated space.
    void free(const BufferSlice& slice);

  private:
    lvk::IContext* context = nullptr;
    uint32_t size = 0;
    lvk::Holder<lvk::BufferHandle> buffer;
    uint64_t buffer_gpu_address = 0;
    OffsetAllocator::Allocator allocator;
};
} // namespace ash
