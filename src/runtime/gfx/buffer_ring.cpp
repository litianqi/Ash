#include "buffer_ring.h"

namespace ash
{
BufferRing::BufferRing(lvk::IContext* context, uint32_t page_num, uint32_t page_size, const char* name)
    : context(context), page_num(page_num), page_size(page_size)
{
    assert(context != nullptr);
    buffer = context->createBuffer({.usage = lvk::BufferUsageBits_Storage,
                                    .storage = lvk::StorageType_HostVisible,
                                    .size = page_num * page_size,
                                    .debugName = name},
                                   nullptr);
    buffer_gpu_address = context->gpuAddress(buffer);
}

void BufferRing::advance()
{
    page_index = (page_index + 1) % page_num;
    page_head = 0;
}

uint64_t BufferRing::alloc(const void* data, uint32_t data_size)
{
    auto offset = (page_index * page_size) + page_head;
    page_head += data_size;
    assert(page_head <= page_size);
    assert(context != nullptr);
    context->upload(buffer, data, data_size, offset);
    auto gpu_address = buffer_gpu_address + offset;
    return gpu_address;
}
} // namespace ash
