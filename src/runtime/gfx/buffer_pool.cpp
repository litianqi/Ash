#include "buffer_pool.h"

namespace ash
{
BufferPool::BufferPool(lvk::IContext* context, uint32_t size, const char* name)
    : context(context), size(size), allocator(size)
{
    assert(context != nullptr);
    buffer = context->createBuffer(
        {.usage = lvk::BufferUsageBits_Storage ,//| lvk::BufferUsageBits_Index | lvk::BufferUsageBits_Vertex,
         .storage = lvk::StorageType_Device,
         .size = size,
         .debugName = name},
        nullptr);
    buffer_gpu_address = context->gpuAddress(buffer);
}

BufferSlice::BufferSlice(BufferSlice&& other) noexcept
{
    pool = std::exchange(other.pool, nullptr);
    gpu_address = std::exchange(other.gpu_address, 0);
    offset = std::exchange(other.offset, 0);
    metadata = std::exchange(other.metadata, 0);
}

BufferSlice& BufferSlice::operator=(BufferSlice&& other) noexcept
{
    pool = std::exchange(other.pool, nullptr);
    gpu_address = std::exchange(other.gpu_address, 0);
    offset = std::exchange(other.offset, 0);
    metadata = std::exchange(other.metadata, 0);
    return *this;
}

BufferSlice BufferPool::alloc(const void* data, uint32_t data_size)
{
    auto allocation = allocator.allocate(data_size);
    assert(context != nullptr);
    context->upload(buffer, data, data_size, allocation.offset);
    return BufferSlice(this, buffer_gpu_address + allocation.offset, allocation);
}

void BufferPool::free(const BufferSlice& slice)
{
    // TODO: defer free
    allocator.free(OffsetAllocator::Allocation{.offset = slice.offset, .metadata = slice.metadata});
}

BufferSlice::BufferSlice(BufferPool* pool, uint64_t gpu_address, OffsetAllocator::Allocation allocation)
    : pool(pool), gpu_address(gpu_address), offset(allocation.offset), metadata(allocation.metadata)
{
}

BufferSlice::~BufferSlice()
{
    if (pool)
    {
        pool->free(*this);
    }
}
} // namespace ash
