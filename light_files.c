#include "light_files.h"

enum lf_flag
{
    LF_PRIV_OPENED,
};

lf_result_t lf_open(lf_descriptor_t *file, uint16_t id, lf_openMode_t mode)
{
    lf_result_t result = LF_RESULT_SUCCESS;

    do
    {
        if(file == NULL)
        {
            result = LF_RESULT_INVALID_PARAMETER;
            continue;
        }

        if(file->flags & (1<<LF_PRIV_OPENED))
        {
            result = LF_RESULT_ALREADY_OPENED;
            continue;
        }


        file->id = id;
        file->mode = mode;

        result = lf_app_open(file);
    }
    while(0);
    return result;
}

lf_result_t lf_onSearchComplete(lf_descriptor_t *file, lf_result_t result)
{
    if(result == LF_RESULT_SUCCESS)
    {
        file->flags |= (1<<LF_PRIV_OPENED);
    }
    return result;
}

lf_result_t lf_write(lf_descriptor_t *file, void *buffer, uint16_t length)
{
    lf_result_t result = LF_RESULT_SUCCESS;

    do
    {
        if(file == NULL || buffer == NULL)
        {
            result = LF_RESULT_INVALID_PARAMETER;
            continue;
        }

        if(!(file->flags & (1<<LF_PRIV_OPENED)))
        {
            result = LF_RESULT_NOT_OPENED;
            continue;
        }

        if(file->mode == LF_MODE_EXISTING_FOR_READ)
        {
            result = LF_RESULT_READ_ONLY;
            continue;
        }

        result = lf_app_write(file, buffer, length);
    }
    while(0);
    return result;
}

lf_result_t lf_onWriteComplete(lf_descriptor_t *file, lf_result_t result)
{
    return result;
}

lf_result_t lf_read(lf_descriptor_t *file, void *buffer, uint16_t length)
{
    lf_result_t result = LF_RESULT_SUCCESS;

    do
    {
        if(file == NULL || buffer == NULL)
        {
            result = LF_RESULT_INVALID_PARAMETER;
            continue;
        }

        if(!(file->flags & (1<<LF_PRIV_OPENED)))
        {
            result = LF_RESULT_NOT_OPENED;
            continue;
        }

        if(file->mode != LF_MODE_EXISTING_FOR_READ)
        {
            result = LF_RESULT_WRITE_ONLY;
            continue;
        }

        result = lf_app_read(file, buffer, length);
    }
    while(0);
    return result; 
}

lf_result_t lf_onReadComplete(lf_descriptor_t *file, lf_result_t result)
{
    return result;
}

lf_result_t lf_close(lf_descriptor_t *file)
{
    lf_result_t result = LF_RESULT_SUCCESS;

    do
    {
        if(file == NULL)
        {
            result = LF_RESULT_INVALID_PARAMETER;
            continue;
        }

        if(!(file->flags & (1<<LF_PRIV_OPENED)))
        {
            result = LF_RESULT_NOT_OPENED;
            continue;
        }

        result = lf_app_flush(file);
    }
    while(0);
    return result; 
}

lf_result_t lf_onFlushComplete(lf_descriptor_t *file, lf_result_t result)
{
    if(result == LF_RESULT_SUCCESS)
    {
        file->flags = 0;
    }
    return result;
} 
