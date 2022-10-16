#include <cstdint>
#include <map>
#include <vector>
#include <cstring>
#include "memory_impl.hpp"

using memBuffer = std::vector<uint8_t>;
using memPair = std::pair<int, memBuffer>;
using memMap = std::map<int, memBuffer>;

memMap memory;

lf_result_t lf_app_open(lf_descriptor_t *file)
{
    lf_result_t result = LF_RESULT_SUCCESS;

    // file exists
    memMap::iterator it = memory.find(file->id);

    if(file->mode == LF_MODE_NEW_FOR_WRITE)
    {
        if(memory.empty() || it == memory.end())
        {
            file->cursor = 0;
            file->size = 0;
            memory.insert(memPair(file->id, memBuffer()));
        }
        else
        {
            result = LF_RESULT_ALREADY_EXISTS;
        }
    }
    else // existing
    {
        if(it == memory.end())
        {
            result = LF_RESULT_NOT_EXISTS;
        }
        else
        {
            if(file->mode == LF_MODE_EXISTING_FOR_WRITE_APPEND)
            {
                file->size = it->second.size();
                file->cursor = file->size;
            }
            else if(file->mode == LF_MODE_EXISTING_FOR_READ)
            {
                file->size = it->second.size();
                file->cursor = 0;
            }
            else
            {
                it->second.clear();
                file->size = 0;
                file->cursor = 0;
            }
        }
    }

    lf_onSearchComplete(file, result);

    return result;
}

lf_result_t lf_app_write(lf_descriptor_t *file, void *buffer, uint16_t length)
{
    memBuffer &buf = memory.at(file->id);
    for(int i = 0; i < length; ++i)
    {
        buf.push_back(((uint8_t*)buffer)[i]);
    }
    file->size = buf.size();

    lf_onWriteComplete(file, LF_RESULT_SUCCESS);
    return LF_RESULT_SUCCESS;
}

lf_result_t lf_app_read(lf_descriptor_t *file, void *buffer, uint16_t length)
{
    memBuffer &buf = memory.at(file->id);
    if(file->cursor + length > file->size)
    {
        length = file->size - file->cursor;
    }

    for(int i = 0; i < length; ++i)
    {
        ((uint8_t*)buffer)[i] = buf[file->cursor++];
    }

    lf_onReadComplete(file, LF_RESULT_SUCCESS);
    return LF_RESULT_SUCCESS;
}

lf_result_t lf_app_flush(lf_descriptor_t *file)
{
    lf_onFlushComplete(file, LF_RESULT_SUCCESS);
    return LF_RESULT_SUCCESS;
}

void memory_clr()
{
    memory.clear();
}