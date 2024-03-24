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
    LF_RESULT_INVALID_KEY,
    LF_RESULT_TOO_MUCH_TO_READ
}
lf_result_t;

#ifdef __cplusplus
extern "C" {
#endif

lf_result_t lf_init(void);
lf_result_t lf_write(uint16_t key, void *content, uint16_t length);
lf_result_t lf_read(uint16_t key, void *content, uint16_t length);
lf_result_t lf_delete(uint16_t key);

// TO IMPLEMENT!!! Shall return LF_RESULT_SUCCESS or LF_RESULT_FAILED
lf_result_t lf_app_init(uint16_t *blockCount, uint16_t *blockSize);
lf_result_t lf_app_write(uint16_t block, uint16_t offset, void *buffer, uint16_t length, uint8_t flush);
lf_result_t lf_app_read(uint16_t block, uint16_t offset, void *buffer, uint16_t length);
lf_result_t lf_app_delete(uint16_t block);


#ifdef __cplusplus
}
#endif

#endif
