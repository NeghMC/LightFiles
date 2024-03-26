#include <cstdio>
#include "light_files.h"
#include "memory_impl.hpp"
#include "light_files_config.h"
#include <cstring>

// helper macro
#define TABLE_SIZE(tab) (sizeof(tab)/sizeof(tab[0]))

//#define WRITE_SIZE (LF_MEMORY_BLOCK_SIZE/2)

// test function prototype
typedef uint8_t (*testFunction)(void);

// ------------------------------------------------------------------------------------------------------

// This test writes an entry in the memory and checks if
// it is correctly written.
uint8_t singleWriteTest()
{
    lf_result_t result = LF_RESULT_SUCCESS;

    // prepare memory

    const uint16_t blockSize = 20;
    const uint16_t blockCount = 5;
    const uint16_t memorySize = blockSize * blockCount;
    uint8_t memoryIn[memorySize];
    memset(memoryIn, 0xff, memorySize);
    memory_config(memoryIn, blockCount, blockSize);
    
    // prepare input buffer

    const int dataSize = 10;
    uint8_t bufferIn[dataSize];
    for(int i = 0; i < dataSize; ++i)
    {
        bufferIn[i] = 10 + i;
    }

    // perform the test
    uint16_t key = 0;

    result = lf_init();
    if(result != LF_RESULT_SUCCESS){return 1;}

    lf_file_cache file;
    result = lf_open(&file, key, LF_MODE_WRITE);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_write(&file, bufferIn, dataSize);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_close(&file);
    if(result != LF_RESULT_SUCCESS){return 1;}

    // verify

    uint8_t expectedMemory[memorySize];
    memset(expectedMemory, 0xff, memorySize);
    uint8_t header[] = {0, 0, 10, 0};
    memcpy(expectedMemory, header, sizeof(header));
    memcpy(expectedMemory + sizeof(header), bufferIn, sizeof(bufferIn));

    if(memcmp(memoryIn, expectedMemory, memorySize) != 0)
    {
        return 1;
    }
    
    return 0;
}

// This test writes an entry in the memory twice and checks if
// it is correctly written.
uint8_t doubleWriteTest()
{
    lf_result_t result = LF_RESULT_SUCCESS;

    // prepare memory

    const uint16_t blockSize = 30;
    const uint16_t blockCount = 5;
    const uint16_t memorySize = blockSize * blockCount;
    uint8_t memoryIn[memorySize];
    memset(memoryIn, 0xff, memorySize);
    memory_config(memoryIn, blockCount, blockSize);
    
    // prepare input buffer

    const int dataSize = 10;
    uint8_t bufferIn[dataSize];
    for(int i = 0; i < dataSize; ++i)
    {
        bufferIn[i] = 10 + i;
    }

    // perform the test
    uint16_t key = 0;

    result = lf_init();
    if(result != LF_RESULT_SUCCESS){return 1;}

    lf_file_cache file;
    result = lf_open(&file, key, LF_MODE_WRITE);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_write(&file, bufferIn, dataSize);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_write(&file, bufferIn, dataSize);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_close(&file);
    if(result != LF_RESULT_SUCCESS){return 1;}

    // verify

    uint8_t expectedMemory[memorySize];
    memset(expectedMemory, 0xff, memorySize);
    uint8_t header[] = {0, 0, 2*sizeof(bufferIn), 0};
    memcpy(expectedMemory, header, sizeof(header));
    memcpy(expectedMemory + sizeof(header), bufferIn, sizeof(bufferIn));
    memcpy(expectedMemory + sizeof(header) + sizeof(bufferIn), bufferIn, sizeof(bufferIn));

    if(memcmp(memoryIn, expectedMemory, memorySize) != 0)
    {
        return 1;
    }
    
    return 0;
}

// this function reads an entry from the memory, and validates its integrity
uint8_t singleReadTest()
{
    lf_result_t result = LF_RESULT_SUCCESS;

    // prepare memory

    const uint16_t blockSize = 20;
    const uint16_t blockCount = 5;
    const uint16_t memorySize = blockSize * blockCount;
    uint8_t memoryIn[memorySize];
    memset(memoryIn, 0xff, memorySize);
    memory_config(memoryIn, blockCount, blockSize);

    uint8_t content[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint8_t header[] = {0, 0, 10, 0};
    memcpy(memoryIn, header, sizeof(header));
    memcpy(memoryIn + sizeof(header), content, sizeof(content));

    // perform the test
    
    uint16_t key = 0;
    uint8_t buffer[sizeof(content)];

    result = lf_init();
    if(result != LF_RESULT_SUCCESS){return 1;}

    lf_file_cache file;
    result = lf_open(&file, key, LF_MODE_READ);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_read(&file, buffer, sizeof(buffer));
    if(result != LF_RESULT_SUCCESS){return 1;}

    if(memcmp(buffer, content, sizeof(buffer)))
    {
        return 1;
    }

    return 0;
}

// this function reads an entry from the memory twice, and validates its integrity
uint8_t doubleReadTest()
{
    lf_result_t result = LF_RESULT_SUCCESS;

    // prepare memory

    const uint16_t blockSize = 30;
    const uint16_t blockCount = 5;
    const uint16_t memorySize = blockSize * blockCount;
    uint8_t memoryIn[memorySize];
    memset(memoryIn, 0xff, memorySize);
    memory_config(memoryIn, blockCount, blockSize);

    uint8_t content[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint8_t header[] = {0, 0, 20, 0};
    memcpy(memoryIn, header, sizeof(header));
    memcpy(memoryIn + sizeof(header), content, sizeof(content));
    memcpy(memoryIn + sizeof(header) + sizeof(content), content, sizeof(content));

    // perform the test
    
    uint16_t key = 0;
    uint8_t buffer1[sizeof(content)];
    uint8_t buffer2[sizeof(content)];

    result = lf_init();
    if(result != LF_RESULT_SUCCESS){return 1;}

    lf_file_cache file;
    result = lf_open(&file, key, LF_MODE_READ);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_read(&file, buffer1, sizeof(buffer1));
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_read(&file, buffer2, sizeof(buffer2));
    if(result != LF_RESULT_SUCCESS){return 1;}

    if(memcmp(buffer1, content, sizeof(buffer1)) || memcmp(buffer2, content, sizeof(buffer2)))
    {
        return 1;
    }

    return 0;
}

// delete test
uint8_t deleteTest()
{
    lf_result_t result = LF_RESULT_SUCCESS;

    // prepare memory

    const uint16_t blockSize = 20;
    const uint16_t blockCount = 5;
    const uint16_t memorySize = blockSize * blockCount;
    uint8_t memoryIn[memorySize];
    memset(memoryIn, 0xff, memorySize);
    memory_config(memoryIn, blockCount, blockSize);

    uint8_t content[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint8_t header[] = {0, 0, 10, 0};
    memcpy(memoryIn, header, sizeof(header));
    memcpy(memoryIn + sizeof(header), content, sizeof(content));

    uint8_t expectedMemory[memorySize];
    memset(expectedMemory, 0xff, memorySize);

    // perform the test
    
    uint16_t key = 0;

    result = lf_init();
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_delete(key);
    if(result != LF_RESULT_SUCCESS){return 1;}

    if(memcmp(memoryIn, expectedMemory, memorySize))
    {
        return 1;
    }

    return 0;
}

// This test writes multiple files, checks if the content is correct, than reads all the files and checks integrity
uint8_t multipleWritesAndReadsTest()
{
    lf_result_t result = LF_RESULT_SUCCESS;

    // prepare memory

    const uint16_t blockSize = 30;
    const uint16_t blockCount = 3;
    const uint16_t memorySize = blockSize * blockCount;
    uint8_t memoryIn[memorySize];
    memset(memoryIn, 0xff, memorySize);
    memory_config(memoryIn, blockCount, blockSize);
    
    // prepare input buffer

    const int dataSize = 10;
    struct {
        uint8_t bufferIn[dataSize];
    } testData[blockCount + 1];

    for(int i = 0; i < blockCount + 1; ++i)
    {
        for(int j = 0; j < dataSize; j++)
        {
            testData[i].bufferIn[j] = ((i + 1) * 10) + j;
        }
    }

    // perform the subtest

    lf_file_cache file;

    result = lf_init();
    if(result != LF_RESULT_SUCCESS){return 1;}

    for(int i = 0; i < blockCount; ++i)
    {
        result = lf_open(&file, i, LF_MODE_WRITE);
        if(result != LF_RESULT_SUCCESS){return 1;}

        result = lf_write(&file, testData[i].bufferIn, dataSize);
        if(result != LF_RESULT_SUCCESS){return 1;}

        result = lf_close(&file);
        if(result != LF_RESULT_SUCCESS){return 1;}
    }

    // verify subtest

    uint8_t expectedMemory[memorySize];
    memset(expectedMemory, 0xff, memorySize);
    for(int i = 0; i < blockCount; ++i)
    {
        uint8_t header[] = {(uint8_t)i, 0, dataSize, 0};
        memcpy(expectedMemory + (i * blockSize), header, sizeof(header));
        memcpy(expectedMemory + (i * blockSize) + sizeof(header), testData[i].bufferIn, dataSize);
    }


    if(memcmp(memoryIn, expectedMemory, memorySize) != 0)
    {
        return 1;
    }

    // perform the subtest

    result = lf_delete(1);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_open(&file, 3, LF_MODE_WRITE);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_write(&file, testData[3].bufferIn, dataSize);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_close(&file);
    if(result != LF_RESULT_SUCCESS){return 1;}

    // verify subtest

    uint8_t header[] = {3, 0, dataSize, 0};
    memcpy(expectedMemory + (blockSize), header, sizeof(header));
    memcpy(expectedMemory + (blockSize) + sizeof(header), testData[3].bufferIn, dataSize);

    if(memcmp(memoryIn, expectedMemory, memorySize) != 0)
    {
        return 1;
    }

    // perform the subtest

    result = lf_delete(0);
    if(result != LF_RESULT_SUCCESS){return 1;}

    result = lf_delete(2);
    if(result != LF_RESULT_SUCCESS){return 1;}

    uint8_t bufferOut[dataSize];

    result = lf_open(&file, 3, LF_MODE_READ);
    if(result != LF_RESULT_SUCCESS){return 1;}
    
    result = lf_read(&file, bufferOut, dataSize);
    if(result != LF_RESULT_SUCCESS){return 1;}

    // verify subtest
    memset(expectedMemory, 0xff, blockSize);
    memset(expectedMemory + (2 * blockSize), 0xff, blockSize);

    if(memcmp(memoryIn, expectedMemory, memorySize) != 0)
    {
        return 1;
    }

    if(memcmp(bufferOut, testData[3].bufferIn, dataSize))
    {
        return 1;
    }
    
    return 0;
}

// ---- error tests ----

// LF_RESULT_ALREADY_EXISTS
uint8_t alreadyExistsErrorTest()
{
    lf_result_t result = LF_RESULT_SUCCESS;

    // prepare memory

    const uint16_t blockSize = 20;
    const uint16_t blockCount = 2;
    const uint16_t memorySize = blockSize * blockCount;
    uint8_t memoryIn[memorySize];
    memset(memoryIn, 0xff, memorySize);
    memory_config(memoryIn, blockCount, blockSize);

    uint8_t header[] = {0, 0, 0, 0}; // ID = 0, size = 0, no content
    memcpy(memoryIn, header, sizeof(header));


    // perform the test

    uint16_t key = 0;

    result = lf_init();
    if(result != LF_RESULT_SUCCESS){return 1;}

    lf_file_cache file;
    result = lf_open(&file, key, LF_MODE_WRITE);
    if(result != LF_RESULT_ALREADY_EXISTS){return 1;}

    return 0;
}

// // LF_RESULT_NOT_EXISTS
// uint8_t notExistsErrorTest()
// {
//     lf_result_t result = LF_RESULT_SUCCESS;

//     // prepare memory

//     const uint16_t blockSize = 20;
//     const uint16_t blockCount = 2;
//     const uint16_t memorySize = blockSize * blockCount;
//     uint8_t memoryIn[memorySize];
//     memset(memoryIn, 0xff, memorySize);
//     memory_config(memoryIn, blockCount, blockSize);

//     // create dummy file, to make the task harder
//     uint8_t header[] = {0, 0, 0, 0}; // ID = 0, size = 0, no content
//     memcpy(memoryIn, header, sizeof(header));


//     // perform the test

//     uint16_t key = 1;
//     uint8_t buffer[0];

//     result = lf_init();
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     result = lf_read(key, buffer, sizeof(buffer));
//     if(result != LF_RESULT_NOT_EXISTS){return 1;}

//     return 0;
// }

// // LF_RESULT_OUT_OF_MEMORY
// uint8_t outOfMemoryErrorTest()
// {
//     lf_result_t result = LF_RESULT_SUCCESS;

//     // prepare memory

//     const uint16_t blockSize = 20;
//     const uint16_t blockCount = 2;
//     const uint16_t memorySize = blockSize * blockCount;
//     uint8_t memoryIn[memorySize];
//     memset(memoryIn, 0xff, memorySize);
//     memory_config(memoryIn, blockCount, blockSize);

//     // create dummy file, to make the task harder
//     uint8_t header1[] = {0, 0, 0, 0}; // ID = 0, size = 0, no content
//     memcpy(memoryIn, header1, sizeof(header1)); // first file
//     uint8_t header2[] = {1, 0, 0, 0}; // ID = 1, size = 0, no content
//     memcpy(memoryIn + blockSize, header2, sizeof(header2)); // second file


//     // perform the test
    
//     uint16_t key = 2;
//     uint8_t buffer[0];

//     result = lf_init();
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     result = lf_write(key, buffer, sizeof(buffer));
//     if(result != LF_RESULT_OUT_OF_MEMORY){return 1;}

//     return 0;
// }

// // LF_RESULT_TOO_BIG_CONTENT
// uint8_t toBigContentErrorTest()
// {
//     lf_result_t result = LF_RESULT_SUCCESS;

//     // prepare memory

//     const uint16_t blockSize = 10;
//     const uint16_t blockCount = 2;
//     const uint16_t memorySize = blockSize * blockCount;
//     uint8_t memoryIn[memorySize];
//     memset(memoryIn, 0xff, memorySize);
//     memory_config(memoryIn, blockCount, blockSize);


//     // perform the test
    
//     uint16_t key = 0;
//     uint8_t buffer[7];

//     result = lf_init();
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     result = lf_write(key, buffer, sizeof(buffer));
//     if(result != LF_RESULT_TOO_BIG_CONTENT){return 1;}

//     return 0;
// }

// // LF_RESULT_INVALID_KEY
// uint8_t invalidKeyErrorTest()
// {
//     lf_result_t result = LF_RESULT_SUCCESS;

//     // prepare memory

//     const uint16_t blockSize = 10;
//     const uint16_t blockCount = 2;
//     const uint16_t memorySize = blockSize * blockCount;
//     uint8_t memoryIn[memorySize];
//     memset(memoryIn, 0xff, memorySize);
//     memory_config(memoryIn, blockCount, blockSize);


//     // perform the test
    
//     uint16_t key = 0xffff;
//     uint8_t buffer[1];

//     result = lf_init();
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     result = lf_write(key, buffer, sizeof(buffer));
//     if(result != LF_RESULT_INVALID_KEY){return 1;}

//     return 0;
// }

// // LF_RESULT_TOO_MUCH_TO_READ
// uint8_t toMuchToReadErrorTest()
// {
//     lf_result_t result = LF_RESULT_SUCCESS;

//     // prepare memory

//     const uint16_t blockSize = 20;
//     const uint16_t blockCount = 5;
//     const uint16_t memorySize = blockSize * blockCount;
//     uint8_t memoryIn[memorySize];
//     memset(memoryIn, 0xff, memorySize);
//     memory_config(memoryIn, blockCount, blockSize);

//     uint8_t content[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
//     uint8_t header[] = {0, 0, 10, 0};
//     memcpy(memoryIn, header, sizeof(header));
//     memcpy(memoryIn + sizeof(header), content, sizeof(content));

//     // perform the test
    
//     uint16_t key = 0;
//     uint8_t buffer[sizeof(content) + 1];

//     result = lf_init();
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     result = lf_read(key, buffer, sizeof(buffer));
//     if(result != LF_RESULT_TOO_MUCH_TO_READ){return 1;}

//     return 0;
// }

int main()
{
    testFunction tests[] =
    {
        singleWriteTest,
        doubleWriteTest,
        singleReadTest,
        doubleReadTest,
        deleteTest,
        multipleWritesAndReadsTest,
        alreadyExistsErrorTest,
        // notExistsErrorTest,
        // outOfMemoryErrorTest,
        // toBigContentErrorTest,
        // invalidKeyErrorTest,
        // toMuchToReadErrorTest
    };

    for(int i = 0; i < TABLE_SIZE(tests); ++i)
    {
        int result = tests[i]();

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
