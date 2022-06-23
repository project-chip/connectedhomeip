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

#include "NlTestLogger.h"

constexpr nl_test_output_logger_t NlTestLogger::nl_test_logger;

using namespace ::chip;

static void test_thread(void * argument)
{
    ChipLogProgress(NotSpecified, "Open IoT SDK unit-tests run...");
    int status = RunRegisteredUnitTests();
    ChipLogProgress(NotSpecified, "Test status: %d", status);
    ChipLogProgress(NotSpecified, "Open IoT SDK unit-tests completed");
    exit(status);
}

int main()
{
    osStatus_t ret = osKernelInitialize();
    if (ret != osOK)
    {
        ChipLogError(NotSpecified, "osKernelInitialize failed: %d", ret);
        return EXIT_FAILURE;
    }

    if (mbedtls_platform_setup(NULL))
    {
        return EXIT_FAILURE;
    }

    nlTestSetLogger(&NlTestLogger::nl_test_logger);

    static const osThreadAttr_t thread_attr = {
        .stack_size = 10240 // Allocate our threads with enough stack for printf
    };
    osThreadId_t thread = osThreadNew(test_thread, NULL, &thread_attr);
    if (thread == NULL)
    {
        ChipLogError(NotSpecified, "Failed to create thread");
        return EXIT_FAILURE;
    }

    osKernelState_t state = osKernelGetState();
    if (state == osKernelReady)
    {
        ChipLogError(NotSpecified, "Starting kernel");
        ret = osKernelStart();
        if (ret != osOK)
        {
            ChipLogError(NotSpecified, "Failed to start kernel: %d", ret);
            return EXIT_FAILURE;
        }
    }
    else
    {
        ChipLogError(NotSpecified, "Kernel not ready: %d", state);
        return EXIT_FAILURE;
    }
    return 0;
}
