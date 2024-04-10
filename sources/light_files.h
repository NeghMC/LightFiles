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

#ifndef LIGHT_FILES
#define LIGHT_FILES

#include <stdint.h>
#include <stddef.h>

typedef enum {
    LF_RESULT_SUCCESS,
    LF_RESULT_FAILED,
    LF_RESULT_ALREADY_EXISTS,
    LF_RESULT_NOT_EXISTS,
    LF_RESULT_OUT_OF_MEMORY,
    LF_RESULT_INVALID_ARGS,
    LF_RESULT_TOO_BIG_DATA_BATCH,
    LF_RESULT_TOO_MUCH_TO_READ,
    LF_RESULT_INVALID_CONFIG,
    LF_RESULT_INVALID_STATE,
    LF_RESULT_END_OF_FILE
} lf_result_t;

typedef struct {
    uint16_t blockCount;
    uint16_t blockSize;
} lf_memory_config;

#ifdef __cplusplus
extern "C" {
#endif

// general
lf_result_t lf_init(void);
lf_result_t lf_delete(uint8_t key); // deletes the file
lf_result_t lf_exists(uint8_t key); // checks if file exists

// write
lf_result_t lf_create(uint8_t key); // starts writing mode
lf_result_t lf_write(void *content, size_t length); // writes data
lf_result_t lf_save(void); // ends writing mode

// read
lf_result_t lf_open(uint8_t key); // starts reading mode
lf_result_t lf_read(void *content, size_t length); // reads data, or skips it if 'content' is null
lf_result_t lf_close(void); // ends reading mode

// TO IMPLEMENT!!! Shall return LF_RESULT_SUCCESS or LF_RESULT_FAILED
lf_result_t lf_app_init(lf_memory_config *config);
    // shall update the configuration
lf_result_t lf_app_write(uint16_t block, uint16_t offset, void *buffer, size_t length, uint8_t flush);
    // shall write 'length' number of bytes from 'buffer' to the block number 'block' starting on 'offset' byte
lf_result_t lf_app_read(uint16_t block, uint16_t offset, void *buffer, size_t length);
    // shall read 'length' number of bytes to 'buffer' from the block number 'block' starting on 'offset' byte
lf_result_t lf_app_delete(uint16_t block);
    // shall erase block number 'block'


#ifdef __cplusplus
}
#endif

#endif
