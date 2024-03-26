#include "light_files.h"
#include "light_files_config.h"

/*
General idea:
    key (unsigned) : content (binary)
    max data size is block size - header

Block structure
2B key
    special keys
        0xffff - block free
2B size
*/

#define LF_BLOCK_HEADER_SIZE (4)
#define LF_CONTENT_MAX_SIZE (sBlockSize - LF_BLOCK_HEADER_SIZE)
#define LF_BLOCK_NONE ((uint16_t)0xffff)
#define LF_KEY_FREE (0xffff)
#define LF_KEY_DELETED (0x0)

// memory info
static uint16_t sBlockSize = 0;
static uint16_t sBlockCount = 0;

static uint16_t sBlock = 0;


static lf_result_t findBlock(uint16_t *foundBlock, uint16_t key)
{
    lf_result_t result;
    static uint16_t block = 0;

    *foundBlock = LF_BLOCK_NONE;
    uint16_t startBlock = sBlock;

    while(1)
    {
        uint16_t blockKey;
        result = lf_app_read(sBlock, 0, &blockKey, sizeof(blockKey));
        if(result != LF_RESULT_SUCCESS)
        {
            break;
        }

        if(blockKey == key)
        {
            *foundBlock = sBlock;
            sBlock++;
            if(sBlock >= sBlockCount)
            {
                sBlock = 0;
            }
            break;
        }

        sBlock++;
        if(sBlock >= sBlockCount)
        {
            sBlock = 0;
        }

        if(sBlock == startBlock)
        {
            // scanned entire memory
            break;
        }
    }

    return result;
}

lf_result_t lf_init(void)
{
    sBlock = 0;
    lf_result_t result = lf_app_init(&sBlockCount, &sBlockSize);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }
    if(sBlockCount == 0xffff)
    {
        return LF_RESULT_INVALID_CONFIG;
    }
    return result;
}

lf_result_t lf_exists(uint16_t key)
{
    if(key == 0xffff)
    {
        return LF_RESULT_INVALID_ARGS;
    }

    uint16_t block;
    lf_result_t result = findBlock(&block, key);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }
    if(block == LF_BLOCK_NONE)
    {
        result = LF_RESULT_NOT_EXISTS;
    }

    return result; 
}

lf_result_t lf_open(lf_file_cache *file, uint16_t key, lf_open_mode mode)
{
    if(key == 0xffff || file == NULL || mode == LF_MODE_NONE)
    {
        return LF_RESULT_INVALID_ARGS;
    }

    uint16_t newBlock;
    lf_result_t result = findBlock(&newBlock, key);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }

    uint16_t size = 0;

    if(mode == LF_MODE_READ)
    {
        if(newBlock == LF_BLOCK_NONE)
        {
            return LF_RESULT_NOT_EXISTS;
        }
        // cache file size
        result = lf_app_read(newBlock, 2, &size, 2);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }
    }

    if(mode == LF_MODE_WRITE)
    {
        if(newBlock != LF_BLOCK_NONE)
        {
            return LF_RESULT_ALREADY_EXISTS;
        }
        lf_result_t result = findBlock(&newBlock, LF_KEY_FREE);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }
        if(newBlock == LF_BLOCK_NONE)
        {
            return LF_RESULT_OUT_OF_MEMORY;
        }
        // claim new block
        result = lf_app_write(newBlock, 0, &key, 2, 1);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }
    }
    
    file->block = newBlock;
    file->cursor = LF_BLOCK_HEADER_SIZE;
    file->key = key;
    file->mode = mode;
    file->size = size;

    return result;
}

lf_result_t lf_write(lf_file_cache *file, void *content, uint16_t length)
{
    // validate args
    if(file == NULL || file->mode == LF_MODE_READ)
    {
        return LF_RESULT_INVALID_ARGS;
    }
    if((file->cursor + length) > sBlockSize)
    {
        return LF_RESULT_TOO_BIG_CONTENT;
    }

    lf_result_t result = lf_app_write(file->block, file->cursor, content, length, 1);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }

    file->cursor += length;
    return result;
}

lf_result_t lf_close(lf_file_cache *file)
{
    if(file == NULL)
    {
        return LF_RESULT_INVALID_ARGS;
    }

    lf_result_t result = LF_RESULT_SUCCESS;
    if(file->mode == LF_MODE_WRITE)
    {
        uint16_t size = file->cursor - LF_BLOCK_HEADER_SIZE;
        result = lf_app_write(file->block, 2, &size, 2, 1);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }
    }

    file->mode = LF_MODE_NONE;
    return result;
}

lf_result_t lf_read(lf_file_cache *file, void *content, uint16_t length)
{
    // validate args
    if(file == NULL || file->mode == LF_MODE_WRITE)
    {
        return LF_RESULT_INVALID_ARGS;
    }

    if(length > (file->size - (file->cursor - LF_BLOCK_HEADER_SIZE)))
    {
        return LF_RESULT_TOO_MUCH_TO_READ;
    }

    // read data
    lf_result_t result = lf_app_read(file->block, file->cursor, content, length);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }

    file->cursor += length;
    return result;
}

lf_result_t lf_delete(uint16_t key)
{
    // validate args
    if(key == 0xffff)
    {
        return LF_RESULT_INVALID_ARGS;
    }

    // find block
    uint16_t block;
    lf_result_t result = findBlock(&block, key);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }
    if(block == LF_BLOCK_NONE)
    {
        return LF_RESULT_NOT_EXISTS;
    }

    // erase block
    result = lf_app_delete(block);

    return result;
}
