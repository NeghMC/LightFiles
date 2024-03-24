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
*  content
*/

#define LF_CONTENT_MAX_SIZE (sBlockSize - 4)
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

static lf_result_t validate(uint16_t key, uint16_t contentLength)
{
    if(key == 0xffff)
    {
        return LF_RESULT_INVALID_KEY;
    }
    if(contentLength > LF_CONTENT_MAX_SIZE)
    {
        return LF_RESULT_TOO_BIG_CONTENT;
    }
    return LF_RESULT_SUCCESS;
}

lf_result_t lf_init(void)
{
    sBlock = 0;
    return lf_app_init(&sBlockCount, &sBlockSize);
}

lf_result_t lf_write(uint16_t key, void *content, uint16_t length)
{
    // validate args
    lf_result_t result = validate(key, length);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }

    uint16_t newBlock;

#ifdef LF_CHECK_IF_EXISTS_BEFORE_CREATING
    result = findBlock(&newBlock, key);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }
    if(newBlock != LF_BLOCK_NONE)
    {
        return LF_RESULT_ALREADY_EXISTS;
    }
#endif

    // find new block
    result = findBlock(&newBlock, LF_KEY_FREE);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }
    if(newBlock == LF_BLOCK_NONE)
    {
        return LF_RESULT_OUT_OF_MEMORY;
    }

    // write header
    uint8_t header[4];
    *((uint16_t*)(header)) = key;
    *((uint16_t*)(header+2)) = length;
    result = lf_app_write(newBlock, 0, header, 4, 0);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }

    // write data
    result = lf_app_write(newBlock, 4, content, length, 1);

    return result;
}

lf_result_t lf_read(uint16_t key, void *content, uint16_t length)
{
    // validate args
    lf_result_t result = validate(key, length);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }

    // find block
    uint16_t block;
    result = findBlock(&block, key);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }
    if(block == LF_BLOCK_NONE)
    {
        return LF_RESULT_NOT_EXISTS;
    }

    // check length
    uint16_t currentDataLength;
    result = lf_app_read(block,2, &currentDataLength, 2);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }
    if(currentDataLength < length)
    {
        return LF_RESULT_TOO_MUCH_TO_READ;
    }

    // read data
    result = lf_app_read(block, 4, content, length);

    return result;
}

lf_result_t lf_delete(uint16_t key)
{
    // validate args
    lf_result_t result = validate(key, 0);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }

    // find block
    uint16_t block;
    result = findBlock(&block, key);
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
