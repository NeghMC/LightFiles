#ifndef LIGHT_FILES
#define LIGHT_FILES

#include <stdint.h>
#include <stddef.h>

typedef enum
{
    LF_MODE_EXISTING_FOR_READ,
    LF_MODE_EXISTING_FOR_WRITE_OVERWRITE,
    LF_MODE_EXISTING_FOR_WRITE_APPEND,
    LF_MODE_NEW_FOR_WRITE,
}
lf_openMode_t;

// 0, 0 - 

typedef enum
{
    LF_RESULT_SUCCESS,
    LF_RESULT_FAILED,
    LF_RESULT_INVALID_PARAMETER,
    LF_RESULT_ALREADY_OPENED,
    LF_RESULT_ALREADY_EXISTS,
    LF_RESULT_NOT_EXISTS,
    LF_RESULT_NOT_OPENED,
    LF_RESULT_READ_ONLY,
    LF_RESULT_WRITE_ONLY,
}
lf_result_t;

// after creation clear flags
typedef struct
{
    uint16_t id;
    uint8_t mode:2;
    uint8_t flags:6;
    uint32_t size;
    uint32_t cursor;
}
lf_descriptor_t;

#ifdef __cplusplus
extern "C" {
#endif

lf_result_t lf_open(lf_descriptor_t *file, uint16_t id, lf_openMode_t mode);
lf_result_t lf_write(lf_descriptor_t *file, void *buffer, uint16_t length);
lf_result_t lf_read(lf_descriptor_t *file, void *buffer, uint16_t length);
lf_result_t lf_close(lf_descriptor_t *file);

// CALLBACKS!
lf_result_t lf_onSearchComplete(lf_descriptor_t *file, lf_result_t result);
lf_result_t lf_onWriteComplete(lf_descriptor_t *file, lf_result_t result);
lf_result_t lf_onReadComplete(lf_descriptor_t *file, lf_result_t result);
lf_result_t lf_onFlushComplete(lf_descriptor_t *file, lf_result_t result);


// TO IMPLEMENT!!! Shall return LF_RESULT_SUCCESS or LF_RESULT_FAILED, the same with the callback
lf_result_t lf_app_open(lf_descriptor_t *file);
lf_result_t lf_app_write(lf_descriptor_t *file, void *buffer, uint16_t length);
lf_result_t lf_app_read(lf_descriptor_t *file, void *buffer, uint16_t length);
lf_result_t lf_app_flush(lf_descriptor_t *file);

#ifdef __cplusplus
}
#endif

#endif