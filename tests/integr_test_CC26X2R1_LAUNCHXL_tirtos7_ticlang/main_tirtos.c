/*
 * Copyright (c) 2016-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,

 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== main_tirtos.c ========
 */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>
#include <ti/drivers/Board.h>
#include <ti/drivers/GPIO.h>

/* Driver configuration */
#include "ti_drivers_config.h"

// LF header file
#include "light_files.h"

/*
 *  ======== tests ========
 */

#define T_ASSERT(a, b) if((a) != (b)) {error = __LINE__; goto error_handler;}

const int sBufferSize = 1024 * 3;
uint8_t sBuffer[sBufferSize];

// that is an simple test showing the example use of the library
void simpleTest()
{
    volatile int error = 0;

    // prepare data
    uint8_t exampleDataOut[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t exampleDataIn[sizeof(exampleDataOut)];
    uint16_t id = 0;

    // open to write
    T_ASSERT(lf_create(id), LF_RESULT_SUCCESS);

    // write
    T_ASSERT(lf_write(exampleDataOut, sizeof(exampleDataOut)), LF_RESULT_SUCCESS);

    // close
    T_ASSERT(lf_save(), LF_RESULT_SUCCESS);

    // open to read
    T_ASSERT(lf_open(id), LF_RESULT_SUCCESS);

    // read
    T_ASSERT(lf_read(exampleDataIn, sizeof(exampleDataIn)), LF_RESULT_SUCCESS);

    // close
    T_ASSERT(lf_close(), LF_RESULT_SUCCESS);

    // verify
    T_ASSERT(memcmp(exampleDataOut, exampleDataIn, sizeof(exampleDataIn)), 0);

    // check if file exists
    lf_result_t result = lf_exists(id);
    if(result == LF_RESULT_SUCCESS)
    {
        // delete the file
        T_ASSERT(lf_delete(id), LF_RESULT_SUCCESS);
    }
    else
    {
        T_ASSERT(result, LF_RESULT_NOT_EXISTS);
    }

    return;

    error_handler:
        GPIO_toggle(GPIO_LED);
        while(1);
}

// this test writes chunks of data generated by the pseudo random generator,
// then it reads that data back and verifies if its correct.
void complexTest()
{
    unsigned int seed = 1;
    volatile int error = 0;

    //const int blockSize = 4096;
    //const int blockCount = 256;

    // --- write data ---
    srand(seed);

    // 10 files in total
    for(uint8_t id = 0; id < 10; ++id)
    {
        // open a file
        T_ASSERT(lf_create(id), LF_RESULT_SUCCESS);

        // size of a batch depends on id - just to make it variable
        for(int batch = 0; batch < (id/2)+1; ++batch)
        {
            // populate batch with a random data
            for(int i = 0; i < sBufferSize; ++i)
            {
                volatile uint8_t random = rand() % 256;
                sBuffer[i] = random;
            }

            // write a batch
            T_ASSERT(lf_write(sBuffer, sBufferSize), LF_RESULT_SUCCESS);
        }

        // save the file
        T_ASSERT(lf_save(), LF_RESULT_SUCCESS);
    }

    // --- test ---
    srand(seed);

    // loops structure is the same as above
    for(uint8_t id = 0; id < 10; ++id)
    {
        // open the file
        T_ASSERT(lf_open(id), LF_RESULT_SUCCESS);

        for(int batch = 0; batch < (id/2)+1; ++batch)
        {
            // read batch
            T_ASSERT(lf_read(sBuffer, sBufferSize), LF_RESULT_SUCCESS);

            for(volatile int i = 0; i < sBufferSize; ++i)
            {
                volatile uint8_t random = rand() % 256;
                if(sBuffer[i] != random)
                {
                    goto error_handler;
                }
            }
        }

        // close the file
        T_ASSERT(lf_close(), LF_RESULT_SUCCESS);
    }

    // remove all the files
    for(uint8_t id = 0; id < 10; ++id)
    {
        T_ASSERT(lf_delete(id), LF_RESULT_SUCCESS);

        T_ASSERT(lf_open(id), LF_RESULT_NOT_EXISTS);
    }

    return;

    error_handler:
        GPIO_toggle(GPIO_LED);
        while(1);
}

// test thread
void *testThread(void *arg0)
{
    GPIO_toggle(GPIO_LED);
    sleep(1);
    GPIO_toggle(GPIO_LED);
    sleep(1);

    // init the library
    lf_result_t result = lf_init();
    if(result != LF_RESULT_SUCCESS)
    {
        while(1);
    }

    simpleTest();
    complexTest();

    while(1)
    {
        GPIO_toggle(GPIO_LED);
        sleep(1);
        GPIO_toggle(GPIO_LED);
        sleep(1);
    }

    return (NULL);
}

/* Stack size in bytes */
#define THREADSTACKSIZE (3*1024)

/*
 *  ======== main ========
 *
 */
int main(void)
{
    pthread_t thread;
    pthread_attr_t attrs;
    struct sched_param priParam;
    int retc;

    Board_init();
    GPIO_init();

    /* Configure the LED and button pins */
    GPIO_setConfig(GPIO_LED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = 1;
    retc                    = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0)
    {
        /* failed to set attributes */
        while (1) {}
    }

    retc = pthread_create(&thread, &attrs, testThread, NULL);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1) {}
    }

    BIOS_start();

    return (0);
}
