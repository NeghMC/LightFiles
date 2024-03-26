#ifndef LIGHT_FILES
#define LIGHT_FILES

#include <stdint.h>
#include <stddef.h>

typedef enum
{
    LF_RESULT_SUCCESS,
    LF_RESULT_FAILED,
    LF_RESULT_ALREADY_EXISTS,
    LF_RESULT_NOT_EXISTS,
    LF_RESULT_OUT_OF_MEMORY,
    LF_RESULT_TOO_BIG_CONTENT,
    LF_RESULT_INVALID_ARGS,
    LF_RESULT_TOO_MUCH_TO_READ,
    LF_RESULT_INVALID_CONFIG
}
lf_result_t;

typedef enum
{
    LF_MODE_NONE,
    LF_MODE_READ,
    LF_MODE_WRITE
}
lf_open_mode;

typedef struct
{
    uint16_t key;
    uint16_t block;
    uint16_t cursor;
    uint16_t size;
    lf_open_mode mode;
}
lf_file_cache;


#ifdef __cplusplus
extern "C" {
#endif

lf_result_t lf_init(void);
lf_result_t lf_delete(uint16_t key);
lf_result_t lf_exists(uint16_t key);

// write
lf_result_t lf_open(lf_file_cache *file, uint16_t key, lf_open_mode mode);
lf_result_t lf_write(lf_file_cache *file, void *content, uint16_t length);
lf_result_t lf_close(lf_file_cache *file);

// read
lf_result_t lf_read(lf_file_cache *file, void *content, uint16_t length);

// TO IMPLEMENT!!! Shall return LF_RESULT_SUCCESS or LF_RESULT_FAILED
lf_result_t lf_app_init(uint16_t *blockCount, uint16_t *blockSize);
lf_result_t lf_app_write(uint16_t block, uint16_t offset, void *buffer, uint16_t length, uint8_t flush);
lf_result_t lf_app_read(uint16_t block, uint16_t offset, void *buffer, uint16_t length);
lf_result_t lf_app_delete(uint16_t block);


#ifdef __cplusplus
}
#endif

#endif
