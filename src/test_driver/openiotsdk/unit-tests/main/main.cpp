/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <errno.h>
#include <mbedtls/platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <support/UnitTestRegistration.h>

using namespace ::chip;

extern "C" uint32_t __end__;
extern "C" uint32_t __HeapLimit;

// Turn off the errno macro and use actual global variable instead.
#undef errno
extern "C" int errno;

// The default _sbrk doesn't have the correct symbols to locate the heap, to provide them
// we use that fact that _sbrk has a weak attribute which allows users to override it.
extern "C" caddr_t _sbrk(int incr)
{
    static uint32_t heap = (uint32_t) &__end__;
    uint32_t prev_heap   = heap;
    uint32_t new_heap    = heap + incr;

    /* __HeapLimit is end of heap section */
    if (new_heap > (uint32_t) &__HeapLimit)
    {
        errno = ENOMEM;
        return (caddr_t) -1;
    }

    heap = new_heap;
    return (caddr_t) prev_heap;
}

static void test_thread(void * argument)
{
    printf("Open IoT SDK unit-tests run...\r\n");
    int status = RunRegisteredUnitTests();
    printf("Test status: %d\r\n", status);
    printf("Open IoT SDK unit-tests completed\r\n");
    exit(status);
}

int main()
{
    osStatus_t ret = osKernelInitialize();
    if (ret != osOK)
    {
        printf("osKernelInitialize failed: %d\r\n", ret);
        return EXIT_FAILURE;
    }

    if (mbedtls_platform_setup(NULL))
    {
        return EXIT_FAILURE;
    }

    static const osThreadAttr_t thread_attr = {
        .stack_size = 10240 // Allocate our threads with enough stack for printf
    };
    osThreadId_t thread = osThreadNew(test_thread, NULL, &thread_attr);
    if (thread == NULL)
    {
        printf("Failed to create thread\r\n");
        return EXIT_FAILURE;
    }

    osKernelState_t state = osKernelGetState();
    if (state == osKernelReady)
    {
        printf("Starting kernel\r\n");
        ret = osKernelStart();
        if (ret != osOK)
        {
            printf("Failed to start kernel: %d\r\n", ret);
            return EXIT_FAILURE;
        }
    }
    else
    {
        printf("Kernel not ready: %d\r\n", state);
        return EXIT_FAILURE;
    }

    return 0;
}
