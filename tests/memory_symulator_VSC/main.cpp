#include <iostream>
#include "light_files.h"
#include "memory_impl.hpp"
#include <cstring>

int test()
{
    lf_result_t result = LF_RESULT_SUCCESS;

    // prepare memory
    const uint16_t blockSize = 20;
    const uint16_t blockCount = 10;
    const uint16_t memorySize = blockSize * blockCount;
    uint8_t memoryIn[memorySize];
    memset(memoryIn, 0xff, memorySize);
    memory_config(memoryIn, blockCount, blockSize);

    uint8_t expectedMemory[memorySize];
    memset(expectedMemory, 0xff, memorySize);

    // ----------------------------- WRITE TESTS -------------------------------------
    {
        // ---- small file test ----
        {
            const int dataSize = 10;
            uint8_t bufferIn[dataSize];
            for(int i = 0; i < dataSize; ++i)
            {
                bufferIn[i] = 10 + i;
            }

            // perform the test
            uint8_t key = 0;

            result = lf_init();
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_create(key);
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_write(bufferIn, dataSize);
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_save();
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            // verify

            uint8_t header[] = {0x80, 0xff, 0xff, 10, 0};
            memcpy(expectedMemory, header, sizeof(header));
            memcpy(expectedMemory + sizeof(header), bufferIn, sizeof(bufferIn));

            if(memcmp(memoryIn, expectedMemory, memorySize) != 0)
            {
                return __LINE__;
            }
        }

        // ---- big file test ----
        {
            const int dataSize = 12;
            uint8_t bufferIn[dataSize];
            for(int i = 0; i < dataSize; ++i)
            {
                bufferIn[i] = 10 + i;
            }

            // perform the test
            uint8_t key = 1;

            result = lf_init();
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_create(key);
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_write(bufferIn, dataSize); // problem - when asked to find a new block it finds the same block
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_write(bufferIn, dataSize);
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_write(bufferIn, dataSize);
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_save();
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            // verify
            uint8_t header0[] = {0x80 | key, 2, 0, 15, 0};
            uint8_t header1[] = {key, 3, 0, 15, 0};
            uint8_t header2[] = {key, 0xff, 0xff, 6, 0};
            memcpy(expectedMemory + blockSize, header0, sizeof(header0));
            memcpy(expectedMemory + blockSize + sizeof(header0), bufferIn, 12);
            memcpy(expectedMemory + blockSize + sizeof(header0) + 12, bufferIn, 3);
            memcpy(expectedMemory + 2 * blockSize, header1, sizeof(header1));
            memcpy(expectedMemory + 2 * blockSize + sizeof(header1), bufferIn + 3, 9);
            memcpy(expectedMemory + 2 * blockSize + sizeof(header1) + 9, bufferIn, 6);
            memcpy(expectedMemory + 3 * blockSize, header2, sizeof(header2));
            memcpy(expectedMemory + 3 * blockSize + sizeof(header2), bufferIn + 6, 6);

            if(memcmp(memoryIn, expectedMemory, memorySize) != 0)
            {
                return __LINE__;
            }
        }
    }
    
    // ----------------------------- READ TESTS -------------------------------------
    {
        // ---- small file test ----
        {
            uint8_t key = 9;

            uint8_t content[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
            uint8_t header[] = {0x80 | key, 0xff, 0xff, 10, 0};
            memcpy(memoryIn + 4 * blockSize, header, sizeof(header));
            memcpy(memoryIn + 4 * blockSize + sizeof(header), content, sizeof(content));
            memcpy(expectedMemory + 4 * blockSize, header, sizeof(header));
            memcpy(expectedMemory + 4 * blockSize + sizeof(header), content, sizeof(content));

            // perform the test

            uint8_t buffer[sizeof(content)];

            result = lf_init();
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_open(key);
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_read(buffer, sizeof(buffer));
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_close();
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            if(memcmp(buffer, content, sizeof(buffer)))
            {
                return __LINE__;
            }
        }

        // ---- large file size ----
        {
            uint8_t key = 10;
            uint8_t content[] = {12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
            uint8_t header0[] = {0x80 | key, 6, 0, 15, 0};
            uint8_t header1[] = {key, 7, 0, 15, 0};
            uint8_t header2[] = {key, 0xff, 0xff, 6, 0};
            memcpy(memoryIn + 5 * blockSize, header0, sizeof(header0));
            memcpy(memoryIn + 5 * blockSize + sizeof(header0), content, sizeof(content));
            memcpy(memoryIn + 5 * blockSize + sizeof(header0) + sizeof(content), content, 3);
            memcpy(memoryIn + 6 * blockSize, header1, sizeof(header1));
            memcpy(memoryIn + 6 * blockSize + sizeof(header1), content + 3, 9);
            memcpy(memoryIn + 6 * blockSize + sizeof(header1) + 9, content, 6);
            memcpy(memoryIn + 7 * blockSize, header2, sizeof(header2));
            memcpy(memoryIn + 7 * blockSize + sizeof(header2), content + 6, 6);
            memcpy(expectedMemory + 5 * blockSize, header0, sizeof(header0));
            memcpy(expectedMemory + 5 * blockSize + sizeof(header0), content, sizeof(content));
            memcpy(expectedMemory + 5 * blockSize + sizeof(header0) + sizeof(content), content, 3);
            memcpy(expectedMemory + 6 * blockSize, header1, sizeof(header1));
            memcpy(expectedMemory + 6 * blockSize + sizeof(header1), content + 3, 9);
            memcpy(expectedMemory + 6 * blockSize + sizeof(header1) + 9, content, 6);
            memcpy(expectedMemory + 7 * blockSize, header2, sizeof(header2));
            memcpy(expectedMemory + 7 * blockSize + sizeof(header2), content + 6, 6);

            // perform the test
            uint8_t buffer1[sizeof(content)];
            uint8_t buffer2[sizeof(content)];
            uint8_t buffer3[sizeof(content)];

            result = lf_init();
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_open(key);
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_read(buffer1, sizeof(buffer1));
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_read(buffer2, sizeof(buffer2));
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_read(buffer3, sizeof(buffer3));
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_close();
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            if(memcmp(buffer1, content, sizeof(buffer1)) || memcmp(buffer2, content, sizeof(buffer2)) || memcmp(buffer3, content, sizeof(buffer3)))
            {
                return __LINE__;
            }

            // -- skip file test --
            uint8_t smallBuffer1[sizeof(content)/2];
            uint8_t smallBuffer2[sizeof(content)/2];

            result = lf_open(key);
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_read(smallBuffer1, sizeof(content)/2);
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_read(NULL, sizeof(content));
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_read(smallBuffer2, sizeof(content)/2);
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            result = lf_close();
            if(result != LF_RESULT_SUCCESS){return __LINE__;}

            if(memcmp(smallBuffer1, content, sizeof(smallBuffer1)) || memcmp(smallBuffer2, content + sizeof(content)/2, sizeof(smallBuffer2)))
            {
                return __LINE__;
            }
        }
    }   

    //  ---- delete test ----
    {
        uint16_t key = 1;

        result = lf_init();
        if(result != LF_RESULT_SUCCESS){return __LINE__;}

        result = lf_delete(key);
        if(result != LF_RESULT_SUCCESS){return __LINE__;}

        memset(expectedMemory + blockSize, 0xff, 3 * blockSize);

        if(memcmp(memoryIn, expectedMemory, memorySize))
        {
            return __LINE__;
        }
    }

    return 0;
}

// // This test writes multiple files, checks if the content is correct, than reads all the files and checks integrity
// uint8_t multipleWritesAndReadsTest()
// {
//     lf_result_t result = LF_RESULT_SUCCESS;

//     // prepare memory

//     const uint16_t blockSize = 20;
//     const uint16_t blockCount = 10;
//     const uint16_t memorySize = blockSize * blockCount;
//     uint8_t memoryIn[memorySize];
//     memset(memoryIn, 0xff, memorySize);
//     memory_config(memoryIn, blockCount, blockSize);
    
//     // prepare input buffer

//     const int dataSize = 10;
//     struct {
//         uint8_t bufferIn[dataSize];
//     } testData[blockCount + 1];

//     for(int i = 0; i < blockCount + 1; ++i)
//     {
//         for(int j = 0; j < dataSize; j++)
//         {
//             testData[i].bufferIn[j] = ((i + 1) * 10) + j;
//         }
//     }

//     // perform the subtest

//     lf_file_cache file;

//     result = lf_init();
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     for(int i = 0; i < blockCount; ++i)
//     {
//         result = lf_open(&file, i, LF_MODE_WRITE);
//         if(result != LF_RESULT_SUCCESS){return 1;}

//         result = lf_write(&file, testData[i].bufferIn, dataSize);
//         if(result != LF_RESULT_SUCCESS){return 1;}

//         result = lf_close(&file);
//         if(result != LF_RESULT_SUCCESS){return 1;}
//     }

//     // verify subtest

//     uint8_t expectedMemory[memorySize];
//     memset(expectedMemory, 0xff, memorySize);
//     for(int i = 0; i < blockCount; ++i)
//     {
//         uint8_t header[] = {(uint8_t)i, 0, dataSize, 0};
//         memcpy(expectedMemory + (i * blockSize), header, sizeof(header));
//         memcpy(expectedMemory + (i * blockSize) + sizeof(header), testData[i].bufferIn, dataSize);
//     }


//     if(memcmp(memoryIn, expectedMemory, memorySize) != 0)
//     {
//         return 1;
//     }

//     // perform the subtest

//     result = lf_delete(1);
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     result = lf_open(&file, 3, LF_MODE_WRITE);
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     result = lf_write(&file, testData[3].bufferIn, dataSize);
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     result = lf_close(&file);
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     // verify subtest

//     uint8_t header[] = {3, 0, dataSize, 0};
//     memcpy(expectedMemory + (blockSize), header, sizeof(header));
//     memcpy(expectedMemory + (blockSize) + sizeof(header), testData[3].bufferIn, dataSize);

//     if(memcmp(memoryIn, expectedMemory, memorySize) != 0)
//     {
//         return 1;
//     }

//     // perform the subtest

//     result = lf_delete(0);
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     result = lf_delete(2);
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     uint8_t bufferOut[dataSize];

//     result = lf_open(&file, 3, LF_MODE_READ);
//     if(result != LF_RESULT_SUCCESS){return 1;}
    
//     result = lf_read(&file, bufferOut, dataSize);
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     // verify subtest
//     memset(expectedMemory, 0xff, blockSize);
//     memset(expectedMemory + (2 * blockSize), 0xff, blockSize);

//     if(memcmp(memoryIn, expectedMemory, memorySize) != 0)
//     {
//         return 1;
//     }

//     if(memcmp(bufferOut, testData[3].bufferIn, dataSize))
//     {
//         return 1;
//     }
    
//     return 0;
// }

// // ---- error tests ----

// // LF_RESULT_ALREADY_EXISTS
// uint8_t alreadyExistsErrorTest()
// {
//     lf_result_t result = LF_RESULT_SUCCESS;

//     // prepare memory

//     const uint16_t blockSize = 20;
//     const uint16_t blockCount = 2;
//     const uint16_t memorySize = blockSize * blockCount;
//     uint8_t memoryIn[memorySize];
//     memset(memoryIn, 0xff, memorySize);
//     memory_config(memoryIn, blockCount, blockSize);

//     uint8_t header[] = {0, 0, 0, 0}; // ID = 0, size = 0, no content
//     memcpy(memoryIn, header, sizeof(header));


//     // perform the test

//     uint16_t key = 0;

//     result = lf_init();
//     if(result != LF_RESULT_SUCCESS){return 1;}

//     lf_file_cache file;
//     result = lf_open(&file, key, LF_MODE_WRITE);
//     if(result != LF_RESULT_ALREADY_EXISTS){return 1;}

//     return 0;
// }

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

using namespace std;

int main()
{
    int result = test();
    if(result == 0)
    {
        cout << "All test competed!" << endl;
    }
    else
    {
        cout << "Test failed at line " << result << "." << endl;
    }

    return 0;
}
