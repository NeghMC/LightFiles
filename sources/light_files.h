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
lf_result_t lf_read(void *content, size_t length); // reads data
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
