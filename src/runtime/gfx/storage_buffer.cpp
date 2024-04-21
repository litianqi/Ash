#include "storage_buffer.h"

namespace ash
{
StorageBuffer::StorageBuffer(lvk::IContext* context, uint32_t size, const char* name)
    : context(context), size(size), allocator(size)
{
    assert(context != nullptr);
    buffer = context->createBuffer(
        {.usage = lvk::BufferUsageBits_Storage, .storage = lvk::StorageType_Device, .size = size, .debugName = name},
        nullptr);
    gpu_address = context->gpuAddress(buffer);
}

OffsetAllocator::Allocation StorageBuffer::allocate(const void* data, uint32_t data_size)
{
    auto allocation = allocator.allocate(data_size);
    assert(context != nullptr);
    context->upload(buffer, data, data_size, allocation.offset);
    return allocation;
}

void StorageBuffer::free(OffsetAllocator::Allocation allocation)
{
    if (allocation.offset != OffsetAllocator::Allocation::NO_SPACE)
    {
        allocator.free(allocation);
    }
}
} // namespace ash
