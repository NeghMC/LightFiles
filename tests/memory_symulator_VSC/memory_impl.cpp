#include <cstdint>
#include <cstring>
#include "memory_impl.hpp"

static struct {
    uint8_t *ptr;
    uint16_t blockCount;
    uint16_t blockSize;
} memoryConfig;

void memory_config(uint8_t *ptr, uint16_t blockCount, uint16_t blockSize)
{
    memoryConfig.ptr = ptr;
    memoryConfig.blockCount = blockCount;
    memoryConfig.blockSize = blockSize;
}

// ---------------- driver implementation ---------------------

lf_result_t lf_app_init(lf_memory_config *config)
{
    config->blockCount = memoryConfig.blockCount;
    config->blockSize = memoryConfig.blockSize;
    return LF_RESULT_SUCCESS;
}

lf_result_t lf_app_write(uint16_t block, uint16_t offset, void *buffer, unsigned int length, uint8_t flush)
{
    if(block >= memoryConfig.blockCount) return LF_RESULT_FAILED;
    uint8_t *p = memoryConfig.ptr + (block*memoryConfig.blockSize) + offset;
    for(int i = 0; i < length; ++i)
    {
        p[i] &= ((uint8_t*)buffer)[i];
    }
    return LF_RESULT_SUCCESS;
}

lf_result_t lf_app_read(uint16_t block, uint16_t offset, void *buffer, unsigned int length)
{
    if(block >= memoryConfig.blockCount) return LF_RESULT_FAILED;
    memcpy(buffer, memoryConfig.ptr + (block*memoryConfig.blockSize) + offset, length);
    return LF_RESULT_SUCCESS;
}

lf_result_t lf_app_delete(uint16_t block)
{
    if(block >= memoryConfig.blockCount) return LF_RESULT_FAILED;
    memset(memoryConfig.ptr + (block*memoryConfig.blockSize), 0xff, memoryConfig.blockSize);
    return LF_RESULT_SUCCESS;
}
