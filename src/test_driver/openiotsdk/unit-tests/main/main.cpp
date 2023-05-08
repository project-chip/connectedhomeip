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

#include <stdio.h>
#include <stdlib.h>

#include <lib/support/UnitTestRegistration.h>
#include <platform/CHIPDeviceLayer.h>

#include <NlTestLogger.h>

#include "cmsis_os2.h"
#include "openiotsdk_platform.h"

constexpr nl_test_output_logger_t NlTestLogger::nl_test_logger;

using namespace ::chip;

static void test_thread(void * argument)
{
    int status;
    CHIP_ERROR err;

    if (openiotsdk_network_init(true))
    {
        ChipLogAutomation("ERROR: Network initialization failed");
        goto exit;
    }

    err = DeviceLayer::PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogAutomation("Chip stack initialization failed: %s", err.AsString());
        goto exit;
    }

    ChipLogAutomation("Open IoT SDK unit-tests run...");
    status = RunRegisteredUnitTests();
    ChipLogAutomation("Test status: %d", status);
    ChipLogAutomation("Open IoT SDK unit-tests completed");
exit:
    osThreadTerminate(osThreadGetId());
}

int main()
{
    ChipLogAutomation("Open IoT SDK unit-tests start");

    if (openiotsdk_platform_init())
    {
        ChipLogAutomation("ERROR: Open IoT SDK platform initialization failed");
        return EXIT_FAILURE;
    }

    nlTestSetLogger(&NlTestLogger::nl_test_logger);

    static const osThreadAttr_t thread_attr = {
        .stack_size = 20 * 1024 // Allocate enough stack for app thread
    };

    osThreadId_t testThread = osThreadNew(test_thread, NULL, &thread_attr);
    if (testThread == NULL)
    {
        ChipLogAutomation("ERROR: Failed to create app thread");
        return EXIT_FAILURE;
    }

    if (openiotsdk_platform_run())
    {
        ChipLogAutomation("ERROR: Open IoT SDK platform run failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
