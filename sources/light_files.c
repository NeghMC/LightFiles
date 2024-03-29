#include "light_files.h"
#include "light_files_config.h"

/*
general idea: file name == key (unsigned int)
assumption: block size is bigger than single batch of data to write at once.
limitation: 
    * you can only work at one file at a time
    * library does not control max file size - user should add it to the file content

Block structure
1B info
    1b - is first (MSb)
    7b - key
    special values
        0xff - block is free
2B next block
    special values
        0xff - NONE
2B size
    special values
        0xff - file not closed
*/

#define LF_BLOCK_HEADER_SIZE (5)
#define LF_CONTENT_MAX_SIZE (sBlockSize - LF_BLOCK_HEADER_SIZE)
#define LF_BLOCK_NONE ((uint16_t)0xffff)
#define LF_KEY_FREE ((uint8_t)127)
#define LF_KEY_MAX ((uint8_t)126)
#define LF_INFO_LEADING_MASK (0x80)

// memory info
static uint16_t sBlockSize = 0;
static uint16_t sBlockCount = 0;

// file info
uint16_t sFirstBlock = LF_BLOCK_NONE;
uint16_t sCurrentBlock = LF_BLOCK_NONE;
uint16_t sNextBlock = LF_BLOCK_NONE;
uint16_t sCursor = 0xff;
uint16_t sSize = 0;
uint8_t sKey = LF_KEY_FREE;
enum {
    LF_MODE_NONE,
    LF_MODE_READING,
    LF_MODE_WRITING
} sEditMode = LF_MODE_NONE;

static uint16_t sBlock = 0;


static lf_result_t findBlock(uint16_t *foundBlock, uint8_t key)
{
    lf_result_t result;

    *foundBlock = LF_BLOCK_NONE;
    uint16_t startBlock = sBlock;

    while(1)
    {
        uint8_t currentKey;
        result = lf_app_read(sBlock, 0, &currentKey, 1);
        if(result != LF_RESULT_SUCCESS)
        {
            break;
        }

        if(currentKey & LF_INFO_LEADING_MASK) // is first
        {
            if((currentKey & ~LF_INFO_LEADING_MASK) == key)
            {
                *foundBlock = sBlock;
                sBlock++;
                if(sBlock >= sBlockCount)
                {
                    sBlock = 0;
                }
                break;
            }
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
    sFirstBlock = LF_BLOCK_NONE;
    sCurrentBlock = LF_BLOCK_NONE;
    sNextBlock = LF_BLOCK_NONE;
    sCursor = 0xff;
    sSize = 0;
    sKey = LF_KEY_FREE;
    sEditMode = LF_MODE_NONE;
    sBlock = 0;

    lf_result_t result = lf_app_init(&sBlockCount, &sBlockSize);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }
    if(sBlockSize <= LF_BLOCK_HEADER_SIZE || sBlockCount == 0 || sBlockCount == 0xffff)
    {
        return LF_RESULT_INVALID_CONFIG;
    }
    return result;
}

lf_result_t lf_exists(uint8_t key)
{
    if(key > LF_KEY_MAX)
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

// open for write
lf_result_t lf_create(uint8_t key)
{
    // validate state
    if(sEditMode != LF_MODE_NONE)
    {
        return LF_RESULT_INVALID_STATE;
    }

    // validate args
    if(key > LF_KEY_MAX)
    {
        return LF_RESULT_INVALID_ARGS;
    }

    uint16_t block;
    lf_result_t result = findBlock(&block, key);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }

    if(block != LF_BLOCK_NONE)
    {
        return LF_RESULT_ALREADY_EXISTS;
    }
        
    result = findBlock(&block, LF_KEY_FREE);

    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }

    if(block == LF_BLOCK_NONE)
    {
        return LF_RESULT_OUT_OF_MEMORY;
    }

    sFirstBlock = block;
    sCurrentBlock = block;
    sCursor = LF_BLOCK_HEADER_SIZE;
    sSize = 0;
    sKey = key;
    sEditMode = LF_MODE_WRITING;

    return result;
}

lf_result_t lf_write(void *content, uint16_t length)
{
    // validate state
    if(sEditMode != LF_MODE_WRITING)
    {
        return LF_RESULT_INVALID_STATE;
    }

    // validate args
    if(length > LF_CONTENT_MAX_SIZE)
    {
        return LF_RESULT_TOO_BIG_DATA_BATCH;
    }

    lf_result_t result = LF_RESULT_SUCCESS;

    // check if data fits in the current block
    uint16_t spaceLeft = sBlockSize - sCursor;
    if(spaceLeft >= length)
    {
        // write everything
        result = lf_app_write(sCurrentBlock, sCursor, content, length, 1);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }

        sCursor += length;
        return result;
    }
    else
    {
        // write what is possible
        result = lf_app_write(sCurrentBlock, sCursor, content, spaceLeft, 1);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }

        sCursor += spaceLeft;

        // find new block
        uint16_t block;
        result = findBlock(&block, LF_KEY_FREE);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }

        if(block == LF_BLOCK_NONE)
        {
            return LF_RESULT_OUT_OF_MEMORY;
        }

        // update current block header
        uint8_t header[LF_BLOCK_HEADER_SIZE];
        *((uint8_t*)(header)) = (sCurrentBlock == sFirstBlock) ? (sKey | LF_INFO_LEADING_MASK) : sKey;
        *((uint16_t*)(header+1)) = block;
        *((uint16_t*)(header+3)) = sCursor - LF_BLOCK_HEADER_SIZE;
        result = lf_app_write(sCurrentBlock, 0, header, LF_BLOCK_HEADER_SIZE, 1);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }

        // switch to the new block
        sCurrentBlock = block;
        sCursor = LF_BLOCK_HEADER_SIZE;

        // write the rest of the data
        uint16_t dataLeftSize = length - spaceLeft;
        result = lf_app_write(sCurrentBlock, sCursor, (uint8_t*)content + spaceLeft, dataLeftSize, 1);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }

        sCursor += dataLeftSize;
        return result;
    }
}

lf_result_t lf_save(void)
{
    // validate state
    if(sEditMode != LF_MODE_WRITING)
    {
        return LF_RESULT_INVALID_STATE;
    }

    lf_result_t result = LF_RESULT_SUCCESS;

    // update current block header
    uint8_t header[LF_BLOCK_HEADER_SIZE];
    *((uint8_t*)(header)) = (sCurrentBlock == sFirstBlock) ? (sKey | LF_INFO_LEADING_MASK) : sKey;
    *((uint16_t*)(header+1)) = LF_BLOCK_NONE;
    *((uint16_t*)(header+3)) = sCursor - LF_BLOCK_HEADER_SIZE;
    result = lf_app_write(sCurrentBlock, 0, header, LF_BLOCK_HEADER_SIZE, 1);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }

    sEditMode = LF_MODE_NONE;
    return result;
}

// open for read
lf_result_t lf_open(uint8_t key)
{
    // validate state
    if(sEditMode != LF_MODE_NONE)
    {
        return LF_RESULT_INVALID_STATE;
    }

    // validate key
    if(key > LF_KEY_MAX)
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
        return LF_RESULT_NOT_EXISTS;
    }

    // cache data
    uint8_t header[LF_BLOCK_HEADER_SIZE - 1];
    result = lf_app_read(block, 1, header, 4);
    if(result != LF_RESULT_SUCCESS)
    {
        return result;
    }
    
    sFirstBlock = block;
    sCurrentBlock = block;
    sNextBlock = *((uint16_t*)(header));
    sCursor = LF_BLOCK_HEADER_SIZE;
    sSize = *((uint16_t*)(header+2));
    sKey = key;
    sEditMode = LF_MODE_READING;

    return result;
}

lf_result_t lf_read(void *content, uint16_t length)
{
    // validate state
    if(sEditMode != LF_MODE_READING)
    {
        return LF_RESULT_INVALID_STATE;
    }

    if(length > LF_CONTENT_MAX_SIZE)
    {
        return LF_RESULT_TOO_MUCH_TO_READ;
    }

    lf_result_t result = LF_RESULT_SUCCESS;

    // check if current block contains enough data
    uint16_t dataInBlockSize = sSize - (sCursor - LF_BLOCK_HEADER_SIZE);
    if(length <= dataInBlockSize)
    {
        // read data
        result = lf_app_read(sCurrentBlock, sCursor, content, length);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }

        sCursor += length;
        return result;
    }
    else
    {
        // read what is possible
        result = lf_app_read(sCurrentBlock, sCursor, content, dataInBlockSize);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }

        sCursor += length;

        // check the next block
        if(sNextBlock == LF_BLOCK_NONE)
        {
            return LF_RESULT_TOO_MUCH_TO_READ;
        }
        
        // cache next block
        uint8_t header[LF_BLOCK_HEADER_SIZE - 1];
        result = lf_app_read(sNextBlock, 1, header, 4);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }
        sCurrentBlock = sNextBlock;
        sNextBlock = *((uint16_t*)(header));
        sCursor = LF_BLOCK_HEADER_SIZE;
        sSize = *((uint16_t*)(header+2));

        // check if enough data
        uint16_t dataLeftSize = length - dataInBlockSize;
        if(dataLeftSize > (sSize - (sCursor - LF_BLOCK_HEADER_SIZE)))
        {
            return LF_RESULT_TOO_MUCH_TO_READ;
        }

        // read the rest
        result = lf_app_read(sCurrentBlock, sCursor, (uint8_t*)content + dataInBlockSize, dataLeftSize);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }

        sCursor += dataLeftSize;
        return result;
    }
}

lf_result_t lf_close(void)
{
    if(sEditMode != LF_MODE_READING)
    {
        return LF_RESULT_INVALID_STATE;
    }

    sEditMode = LF_MODE_NONE;
    return LF_RESULT_SUCCESS;
}

lf_result_t lf_delete(uint8_t key)
{
    if(sEditMode != LF_MODE_NONE)
    {
        return LF_RESULT_INVALID_STATE;
    }

    if(key > LF_KEY_MAX)
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

    while(1)
    {
        uint16_t nextBlock;
        result = lf_app_read(block, 1, &nextBlock, 2);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }
        
        // erase block
        result = lf_app_delete(block);
        if(result != LF_RESULT_SUCCESS)
        {
            return result;
        }

        // find next block
        if(nextBlock != LF_BLOCK_NONE)
        {
            block = nextBlock;
        }
        else
        {
            break;
        }
    }

    return result;
}
