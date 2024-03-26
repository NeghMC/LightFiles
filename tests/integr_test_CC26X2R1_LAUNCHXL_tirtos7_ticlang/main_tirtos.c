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
 *  ======== test ========
 */
void *testThread(void *arg0)
{
    GPIO_toggle(GPIO_LED);

    // init the library
    lf_result_t result = lf_init();
    if(result != LF_RESULT_SUCCESS)
    {
        while(1);
    }

    // prepare data
    uint8_t exampleDataOut[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t exampleDataIn[sizeof(exampleDataOut)];
    uint16_t id = 0;
    lf_file_cache file1, file2;

    // check if file already exists
    result = lf_exists(id);
    if(result == LF_RESULT_SUCCESS)
    {
        // delete the file
        result = lf_delete(id);
        if(result != LF_RESULT_SUCCESS)
        {
            while(1);
        }
    }
    else if(result != LF_RESULT_NOT_EXISTS)
    {
        while(1);
    }

    // open to write
    result = lf_open(&file1, id, LF_MODE_WRITE);
    if(result != LF_RESULT_SUCCESS)
    {
        while(1);
    }

    // write
    result = lf_write(&file1, exampleDataOut, sizeof(exampleDataOut));
    if(result != LF_RESULT_SUCCESS)
    {
        while(1);
    }

    // close
    result = lf_close(&file1);
    if(result != LF_RESULT_SUCCESS)
    {
        while(1);
    }

    // open to read
    result = lf_open(&file2, id, LF_MODE_READ);
    if(result != LF_RESULT_SUCCESS)
    {
        while(1);
    }

    // read
    result = lf_read(&file2, exampleDataIn, sizeof(exampleDataIn));
    if(result != LF_RESULT_SUCCESS)
    {
        while(1);
    }

    // close
    result = lf_close(&file2);
    if(result != LF_RESULT_SUCCESS)
    {
        while(1);
    }

    // verify
    if(memcmp(exampleDataOut, exampleDataIn, sizeof(exampleDataIn)) != 0)
    {
        while(1);
    }

    GPIO_toggle(GPIO_LED);

    return (NULL);
}

/* Stack size in bytes */
#define THREADSTACKSIZE 1024

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
