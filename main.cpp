#include <cstdio>
#include "light_files.h"
#include "memory_impl.hpp"
#include <cstring>

#define MEM_SIZE (1000u)

#define TABLE_SIZE(tab) (sizeof(tab)/sizeof(tab[0]))

typedef uint8_t (*testFunction)(void);

void fd_clear(lf_descriptor_t *fd)
{
    memset(fd, 0xff, sizeof(fd));
    fd->flags = 0;
}

// ------------------------------------------------------------------------------------------------------
uint8_t test0()
{   // simple small data test
    lf_result_t result = LF_RESULT_SUCCESS;
    
    lf_descriptor_t fd, *file = &fd;

    uint8_t buffer1[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t buffer2[8];
    
    do
    {
        fd_clear(file);

        result = lf_open(file, 0, LF_MODE_NEW_FOR_WRITE);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_write(file, buffer1, 8);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_close(file);
        if(result != LF_RESULT_SUCCESS){continue;}

        fd_clear(file);

        result = lf_open(file, 0, LF_MODE_EXISTING_FOR_READ);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_read(file, buffer2, 8);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_close(file);
        if(result != LF_RESULT_SUCCESS){continue;}      

        if(memcmp(buffer1, buffer2, 8) != 0)
        {
            result = LF_RESULT_FAILED;
        }

    }
    while (0);
    
    return result;
}

uint8_t test1()
{   // append
    lf_result_t result = LF_RESULT_SUCCESS;
    
    lf_descriptor_t fd, *file = &fd;

    uint8_t buffer1[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t buffer2[16];
    
    do
    {
        fd_clear(file);

        result = lf_open(file, 0, LF_MODE_NEW_FOR_WRITE);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_write(file, buffer1, 8);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_close(file);
        if(result != LF_RESULT_SUCCESS){continue;}

        fd_clear(file);

        result = lf_open(file, 0, LF_MODE_EXISTING_FOR_WRITE_APPEND);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_write(file, buffer1, 8);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_close(file);
        if(result != LF_RESULT_SUCCESS){continue;}

        fd_clear(file);

        result = lf_open(file, 0, LF_MODE_EXISTING_FOR_READ);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_read(file, buffer2, 16);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_close(file);
        if(result != LF_RESULT_SUCCESS){continue;}      

        if(memcmp(buffer1, buffer2, 8) != 0 || memcmp(buffer1, buffer2+8, 8) != 0)
        {
            result = LF_RESULT_FAILED;
        }

    }
    while (0);
    
    return result;
}

uint8_t test2()
{   // multiple files
    lf_result_t result = LF_RESULT_SUCCESS;
    
    lf_descriptor_t fd, *file = &fd;

    struct
    {
        uint8_t bufferW[8];
        uint8_t bufferR[8];
    }
    tests[] =
    {
        {{1, 2, 3, 4, 5, 6, 7, 8}},
        {{9, 8, 7, 6, 5, 4, 3, 2}},
        {{11, 12, 13, 14, 15, 16, 17}},
        {{16, 15, 14, 13, 12, 11, 10}}
    };
    
    do
    {
        for(int i = 0; i < TABLE_SIZE(tests); ++i)
        {
            fd_clear(file);

            result = lf_open(file, i, LF_MODE_NEW_FOR_WRITE);
            if(result != LF_RESULT_SUCCESS){break;}

            result = lf_write(file, tests[i].bufferW, 8);
            if(result != LF_RESULT_SUCCESS){break;}

            result = lf_close(file);
            if(result != LF_RESULT_SUCCESS){break;}
        }

        if(result != LF_RESULT_SUCCESS){continue;}

        for(int i = 0; i < TABLE_SIZE(tests); ++i)
        {
            fd_clear(file);

            result = lf_open(file, i, LF_MODE_EXISTING_FOR_READ);
            if(result != LF_RESULT_SUCCESS){break;}

            result = lf_read(file, tests[i].bufferR, 8);
            if(result != LF_RESULT_SUCCESS){break;}

            result = lf_close(file);
            if(result != LF_RESULT_SUCCESS){break;}      

            if(memcmp(tests[i].bufferR, tests[i].bufferW, 8) != 0)
            {
                result = LF_RESULT_FAILED;
            }
        }

    }
    while (0);
    
    return result;
}

uint8_t test3()
{   // log test
    lf_result_t result = LF_RESULT_SUCCESS;

    lf_descriptor_t fd, *file = &fd;
    
    char buffer[30];
    const char *str = "To jest log no %d\n\r";
    int counter = 0;

    do
    {
        fd_clear(file);

        // === roskminić jakoś na lepiej ===
        result = lf_open(file, 0, LF_MODE_NEW_FOR_WRITE);
        if(result != LF_RESULT_SUCCESS){continue;}
    
        result = lf_close(file);
        if(result != LF_RESULT_SUCCESS){continue;}
        // =================================
        
        for(int i = 0; i < 10; ++i)
        {
            fd_clear(file);

            result = lf_open(file, 0, LF_MODE_EXISTING_FOR_WRITE_APPEND);
            if(result != LF_RESULT_SUCCESS){break;}

            for(int j = 0; j < 10; ++j)
            {
                int size = snprintf(buffer, TABLE_SIZE(buffer), str, counter++);
                result = lf_write(file, buffer, size);
                if(result != LF_RESULT_SUCCESS){break;}
            }

            result = lf_close(file);
            if(result != LF_RESULT_SUCCESS){break;}
        }

        if(result != LF_RESULT_SUCCESS){continue;}


        fd_clear(file);

        result = lf_open(file, 0, LF_MODE_EXISTING_FOR_READ);
        if(result != LF_RESULT_SUCCESS){continue;}

        char *buf = new char[file->size+1];
        if(buf == NULL)
        {
            result = LF_RESULT_FAILED;
            continue;
        }

        lf_read(file, buf, file->size);
        if(result != LF_RESULT_SUCCESS){break;}

        lf_close(file);
        if(result != LF_RESULT_SUCCESS){break;}

        printf("%.*s\n\r", file->size, buf);
        delete [] buf;

    } 
    while (0);
    
    return result;
}

uint8_t test4()
{   // overwrite test
    lf_result_t result = LF_RESULT_SUCCESS;
    
    lf_descriptor_t fd = {}, *file = &fd;

    uint8_t buffer1[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t buffer2[] = {6, 5, 4, 3, 2, 1, 0};
    uint8_t buffer3[7];
    
    do
    {
        result = lf_open(file, 0, LF_MODE_NEW_FOR_WRITE);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_write(file, buffer1, 8);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_close(file);
        if(result != LF_RESULT_SUCCESS){continue;}

        fd = {};

        result = lf_open(file, 0, LF_MODE_EXISTING_FOR_WRITE_OVERWRITE);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_write(file, buffer2, 7);
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_close(file);
        if(result != LF_RESULT_SUCCESS){continue;}

        fd = {};

        result = lf_open(file, 0, LF_MODE_EXISTING_FOR_READ);
        if(result != LF_RESULT_SUCCESS){continue;}

        if(file->size != TABLE_SIZE(buffer2))
        {
            result = LF_RESULT_FAILED;
            continue;
        }

        result = lf_read(file, buffer3, TABLE_SIZE(buffer3));
        if(result != LF_RESULT_SUCCESS){continue;}

        result = lf_close(file);
        if(result != LF_RESULT_SUCCESS){continue;}      

        if(memcmp(buffer2, buffer3, TABLE_SIZE(buffer3)) != 0)
        {
            result = LF_RESULT_FAILED;
        }

    }
    while (0);
    
    return result;
}

int main()
{
    testFunction tests[] =
    {
        test0,
        test1,
        test2,
        test3,
        test4,
    };

    for(int i = 0; i < TABLE_SIZE(tests); ++i)
    {
        int result = tests[i]();
        memory_clr();

        if(result)
        {
            printf("test %d failed\n\r", i);
            break;
        }
        else
        {
            printf("test %d passed\n\r", i);
        }
    }
    return 0;
}