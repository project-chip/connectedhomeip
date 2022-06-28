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
#include <iotsdk/ip_network_api.h>
#include <mbedtls/platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <support/UnitTestRegistration.h>

#include "NlTestLogger.h"

constexpr nl_test_output_logger_t NlTestLogger::nl_test_logger;

using namespace ::chip;

static void test_thread(void * argument)
{
    ChipLogProgress(Test, "Open IoT SDK unit-tests run...");
    int status = RunRegisteredUnitTests();
    ChipLogProgress(Test, "Test status: %d", status);
    ChipLogProgress(Test, "Open IoT SDK unit-tests completed");
    exit(status);
}

static void network_event_callback(network_state_callback_event_t event)
{
    static osThreadId_t test_thread_id = NULL;
    if (event == NETWORK_UP)
    {
        ChipLogProgress(Test, "Network up");

        if (test_thread_id)
        {
            ChipLogError(Test, "Tests already running");
            exit(EXIT_FAILURE);
        }

        static const osThreadAttr_t thread_attr = { .stack_size = 1024 * 20 };

        osThreadId_t test_thread_id = osThreadNew(test_thread, NULL, &thread_attr);
    }
    else
    {
        ChipLogError(Test, "Network down, aborting tests");
        exit(EXIT_FAILURE);
    }
}

static void init_thread(void * argument)
{
    ChipLogProgress(Test, "Initialising network");
    osStatus_t res = start_network_task(network_event_callback, 10240);
    if (res != osOK)
    {
        ChipLogError(Test, "Failed to start lwip");
        exit(res);
    }
    osThreadTerminate(osThreadGetId());
}

int main()
{
    osStatus_t ret = osKernelInitialize();
    if (ret != osOK)
    {
        ChipLogError(Test, "osKernelInitialize failed: %d", ret);
        return EXIT_FAILURE;
    }

    if (mbedtls_platform_setup(NULL))
    {
        return EXIT_FAILURE;
    }

    nlTestSetLogger(&NlTestLogger::nl_test_logger);

    osThreadId_t thread = osThreadNew(init_thread, NULL, NULL);

    if (thread == NULL)
    {
        ChipLogError(Test, "Failed to create thread");
        return EXIT_FAILURE;
    }

    osKernelState_t state = osKernelGetState();
    if (state == osKernelReady)
    {
        ChipLogProgress(Test, "Starting kernel");
        ret = osKernelStart();
        if (ret != osOK)
        {
            ChipLogError(Test, "Failed to start kernel: %d", ret);
            return EXIT_FAILURE;
        }
    }
    else
    {
        ChipLogError(Test, "Kernel not ready: %d", state);
        return EXIT_FAILURE;
    }
    return 0;
}
