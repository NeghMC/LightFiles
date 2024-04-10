/*
 * LightFiles
 * 
 * Author: Grzegorz Świstak (NeghMC)
 * 
 * DISCLAIMER: This software is provided 'as-is', without any express
 * or implied warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "light_files.h"

/*
general idea: file name == key (uint8_t)
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
#define LF_CONTENT_MAX_SIZE (sMemory.blockSize - LF_BLOCK_HEADER_SIZE)
#define LF_BLOCK_NONE ((uint16_t)0xffff)
#define LF_KEY_FREE ((uint8_t)0xff)
#define LF_KEY_MAX ((uint8_t)126)
#define LF_INFO_LEADING_MASK (0x80)

// error macro
#define LF_ASSERT(cond, ret) if(cond) {return ret;}

// memory info
static lf_memory_config sMemory;

// file info
static uint16_t sFirstBlock = LF_BLOCK_NONE;
static uint16_t sCurrentBlock = LF_BLOCK_NONE;
static uint16_t sNextBlock = LF_BLOCK_NONE;
static uint16_t sCursor = 0;
static uint16_t sSize = 0;
static uint8_t sKey = LF_KEY_FREE;
static enum {
    LF_MODE_NONE,
    LF_MODE_READING,
    LF_MODE_WRITING
} sEditMode = LF_MODE_NONE;

static uint16_t sBlock = 0;
static uint16_t sLastFreeBlock = LF_BLOCK_NONE;

typedef struct {
    uint16_t block;
    uint16_t nextBlock;
    uint16_t size;
} block_info_t;

static void blockIncrement(void)
{
    sBlock = (sBlock >= sMemory.blockCount - 1) ? 0 : (sBlock + 1);
}

static lf_result_t findFreeBlock(uint16_t *freeBlock)
{
    lf_result_t result;

    *freeBlock = LF_BLOCK_NONE;
    uint16_t startBlock = sBlock;

    do
    {
        if(sBlock != sLastFreeBlock)
        {
            uint8_t info;
            result = lf_app_read(sBlock, 0, &info, 1);
            LF_ASSERT(result != LF_RESULT_SUCCESS, result);

            if(info == LF_KEY_FREE)
            {
                *freeBlock = sLastFreeBlock = sBlock;
                break;
            }
        }

        // increment
        blockIncrement();
    }
    while(sBlock != startBlock);

    return result;
}

static lf_result_t findBlock(block_info_t *info, uint8_t key, uint8_t cacheAll)
{
    lf_result_t result;

    info->block = LF_BLOCK_NONE;
    uint16_t startBlock = sBlock;

    do
    {
        uint8_t header[LF_BLOCK_HEADER_SIZE];
        result = lf_app_read(sBlock, 0, header, cacheAll ? LF_BLOCK_HEADER_SIZE : 1);
        LF_ASSERT(result != LF_RESULT_SUCCESS, result);

        if( (header[0] & LF_INFO_LEADING_MASK) && ((header[0] & ~LF_INFO_LEADING_MASK) == key) )
        {
            info->block = sBlock;
            if(cacheAll)
            {
                info->nextBlock = *((uint16_t*)(header+1));
                info->size = *((uint16_t*)(header+3));
            }
            break;
        }

        // increment
        blockIncrement();
    }
    while(sBlock != startBlock);

    return result;
}

static lf_result_t save_current_block()
{
    // update current block header
    uint8_t header[LF_BLOCK_HEADER_SIZE];
    *((uint8_t*)(header)) = (sCurrentBlock == sFirstBlock) ? (sKey | LF_INFO_LEADING_MASK) : sKey;
    *((uint16_t*)(header+1)) = sNextBlock;
    *((uint16_t*)(header+3)) = sCursor;
    return lf_app_write(sCurrentBlock, 0, header, LF_BLOCK_HEADER_SIZE, 1);
}

lf_result_t lf_init(void)
{
    sBlock = 0;
    sFirstBlock = LF_BLOCK_NONE;
    sCurrentBlock = LF_BLOCK_NONE;
    sNextBlock = LF_BLOCK_NONE;
    sCursor = 0;
    sSize = 0;
    sKey = LF_KEY_FREE;
    sEditMode = LF_MODE_NONE;
    sBlock = 0;
    sLastFreeBlock = LF_BLOCK_NONE;

    lf_result_t result = lf_app_init(&sMemory);
    LF_ASSERT(result != LF_RESULT_SUCCESS, result);
    LF_ASSERT(sMemory.blockSize <= LF_BLOCK_HEADER_SIZE || sMemory.blockCount == 0 || sMemory.blockCount == 0xffff, LF_RESULT_INVALID_CONFIG);
    return result;
}

lf_result_t lf_exists(uint8_t key)
{
    LF_ASSERT(key > LF_KEY_MAX, LF_RESULT_INVALID_ARGS);

    block_info_t info;
    lf_result_t result = findBlock(&info, key, 0);
    LF_ASSERT(result != LF_RESULT_SUCCESS, result);
    if(info.block == LF_BLOCK_NONE)
    {
        result = LF_RESULT_NOT_EXISTS;
    }

    return result; 
}

// open for write
lf_result_t lf_create(uint8_t key)
{
    // validate state
    LF_ASSERT(sEditMode != LF_MODE_NONE, LF_RESULT_INVALID_STATE);

    // validate args
    LF_ASSERT(key > LF_KEY_MAX, LF_RESULT_INVALID_ARGS);

    block_info_t info;
    lf_result_t result = findBlock(&info, key, 0);
    LF_ASSERT(result != LF_RESULT_SUCCESS, result);
    LF_ASSERT(info.block != LF_BLOCK_NONE, LF_RESULT_ALREADY_EXISTS);
        
    result = findFreeBlock(&info.block);
    LF_ASSERT(result != LF_RESULT_SUCCESS, result);
    LF_ASSERT(info.block == LF_BLOCK_NONE, LF_RESULT_OUT_OF_MEMORY);

    sFirstBlock = info.block;
    sCurrentBlock = info.block;
    sCursor = 0;
    sSize = 0;
    sKey = key;
    sEditMode = LF_MODE_WRITING;

    return result;
}

lf_result_t lf_write(void *content, size_t length)
{
    // validate state
    LF_ASSERT(sEditMode != LF_MODE_WRITING, LF_RESULT_INVALID_STATE);

    if(length == 0)
    {
        return LF_RESULT_SUCCESS;
    }

    lf_result_t result = LF_RESULT_SUCCESS;
    size_t contentOffset = 0;

    while(1)
    {
        size_t spaceLeft = sMemory.blockSize - sCursor - LF_BLOCK_HEADER_SIZE;
        if(spaceLeft == 0)
        {
            // find new block
            block_info_t info;
            result = findFreeBlock(&info.block);
            LF_ASSERT(result != LF_RESULT_SUCCESS, result);
            LF_ASSERT(info.block == LF_BLOCK_NONE, LF_RESULT_OUT_OF_MEMORY);

            // update current block header
            sNextBlock = info.block;
            result = save_current_block();
            LF_ASSERT(result != LF_RESULT_SUCCESS, result);

            // switch to the new block
            sCurrentBlock = info.block;
            sCursor = 0;

            spaceLeft = sMemory.blockSize - LF_BLOCK_HEADER_SIZE;
        }

        size_t toSaveSize = (length > spaceLeft) ? spaceLeft : length;
        result = lf_app_write(sCurrentBlock, LF_BLOCK_HEADER_SIZE + sCursor, (uint8_t*)content + contentOffset, toSaveSize, 0);
        LF_ASSERT(result != LF_RESULT_SUCCESS, result);
        sCursor += toSaveSize;

        // if thats all
        if(toSaveSize == length)
        {
            break;
        }
        
        contentOffset += toSaveSize;
        length -= toSaveSize;
    }

    return result;
}

lf_result_t lf_save(void)
{
    // validate state
    if(sEditMode != LF_MODE_WRITING)
    {
        return LF_RESULT_INVALID_STATE;
    }

    // update current block header
    sNextBlock = LF_BLOCK_NONE;
    lf_result_t result = save_current_block();
    LF_ASSERT(result != LF_RESULT_SUCCESS, result);
    sEditMode = LF_MODE_NONE;

    return result;
}

// open for read
lf_result_t lf_open(uint8_t key)
{
    // validate state
    LF_ASSERT(sEditMode != LF_MODE_NONE, LF_RESULT_INVALID_STATE);

    // validate key
    LF_ASSERT(key > LF_KEY_MAX, LF_RESULT_INVALID_ARGS);

    block_info_t info;
    lf_result_t result = findBlock(&info, key, 1);
    LF_ASSERT(result != LF_RESULT_SUCCESS, result);
    LF_ASSERT(info.block == LF_BLOCK_NONE, LF_RESULT_NOT_EXISTS);
    
    sFirstBlock = info.block;
    sCurrentBlock = info.block;
    sNextBlock = info.nextBlock;
    sCursor = 0;
    sSize = info.size;
    sKey = key;
    sEditMode = LF_MODE_READING;

    return result;
}

lf_result_t lf_read(void *content, size_t length)
{
    // validate state
    LF_ASSERT(sEditMode != LF_MODE_READING, LF_RESULT_INVALID_STATE);

    if(length == 0)
    {
        return LF_RESULT_SUCCESS;
    }

    lf_result_t result = LF_RESULT_SUCCESS;
    size_t contentOffset = 0;

    while(1)
    {
        // check if current block contains enough data
        size_t dataLeftSize;
        while(1)
        {
            dataLeftSize = sSize - sCursor;
            if(dataLeftSize == 0)
            {
                if(sNextBlock == LF_BLOCK_NONE)
                {
                    return LF_RESULT_END_OF_FILE;
                }
                else
                {
                    // cache next block
                    uint8_t header[LF_BLOCK_HEADER_SIZE - 1];
                    result = lf_app_read(sNextBlock, 1, header, 4);
                    LF_ASSERT(result != LF_RESULT_SUCCESS, result);
                    sCurrentBlock = sNextBlock;
                    sNextBlock = *((uint16_t*)(header));
                    sCursor = 0;
                    sSize = *((uint16_t*)(header+2));
                    dataLeftSize = sSize - sCursor;
                }
            }
            else
            {
                break;
            }
        }

        size_t toReadSize = (length > dataLeftSize) ? dataLeftSize : length;
        if(content != NULL)
        {
            result = lf_app_read(sCurrentBlock, LF_BLOCK_HEADER_SIZE + sCursor, (uint8_t*)content + contentOffset, toReadSize);
            LF_ASSERT(result != LF_RESULT_SUCCESS, result);
        }
        sCursor += toReadSize;

        // if thats all
        if(toReadSize == length)
        {
            break;
        }

        contentOffset += toReadSize;
        length -= toReadSize;
    }

    return result;
}

lf_result_t lf_close(void)
{
    LF_ASSERT(sEditMode != LF_MODE_READING, LF_RESULT_INVALID_STATE);
    sEditMode = LF_MODE_NONE;
    return LF_RESULT_SUCCESS;
}

lf_result_t lf_delete(uint8_t key)
{
    LF_ASSERT(sEditMode != LF_MODE_NONE, LF_RESULT_INVALID_STATE);
    LF_ASSERT(key > LF_KEY_MAX, LF_RESULT_INVALID_ARGS);

    // find block
    block_info_t info;
    lf_result_t result = findBlock(&info, key, 1);
    LF_ASSERT(result != LF_RESULT_SUCCESS, result);
    LF_ASSERT(info.block == LF_BLOCK_NONE, LF_RESULT_NOT_EXISTS);
    sCurrentBlock = info.block;
    sNextBlock = info.nextBlock;

    while(1)
    {
        // erase block
        result = lf_app_delete(sCurrentBlock);
        LF_ASSERT(result != LF_RESULT_SUCCESS, result);

        if(sNextBlock == LF_BLOCK_NONE)
        {
            break;
        }

        sCurrentBlock = sNextBlock;
        result = lf_app_read(sCurrentBlock, 1, &sNextBlock, 2);
        LF_ASSERT(result != LF_RESULT_SUCCESS, result);
    }

    return result;
}
