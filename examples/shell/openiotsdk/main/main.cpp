/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/shell/Engine.h>
#include <lib/support/logging/CHIPLogging.h>

#include <ChipShellCollection.h>

#include "cmsis_os2.h"
#include "openiotsdk_platform.h"

using namespace ::chip;
using namespace ::chip::Shell;

static void app_thread(void * argument)
{
    int ret;

    if (openiotsdk_network_init(true))
    {
        ChipLogError(Shell, "Network initialization failed");
        goto exit;
    }

    // Initialize the default streamer that was linked.
    ret = Engine::Root().Init();
    if (ret)
    {
        ChipLogError(Shell, "Streamer initialization failed [%d]", ret);
        goto exit;
    }

    cmd_misc_init();

    ChipLogProgress(Shell, "Open IoT SDK shell example application run");

    Engine::Root().RunMainLoop();

exit:
    osThreadTerminate(osThreadGetId());
}

int main()
{
    ChipLogProgress(Shell, "Open IoT SDK shell example application start");

    if (openiotsdk_platform_init())
    {
        ChipLogError(Shell, "Open IoT SDK platform initialization failed");
        return EXIT_FAILURE;
    }

    if (openiotsdk_chip_init())
    {
        ChipLogError(Shell, "Open IoT SDK CHIP stack initialization failed");
        return EXIT_FAILURE;
    }

    static const osThreadAttr_t thread_attr = {
        .stack_size = 8 * 1024 // Allocate enough stack for app thread
    };

    osThreadId_t appThread = osThreadNew(app_thread, NULL, &thread_attr);
    if (appThread == NULL)
    {
        ChipLogError(Shell, "Failed to create app thread");
        return EXIT_FAILURE;
    }

    if (openiotsdk_platform_run())
    {
        ChipLogError(Shell, "Open IoT SDK platform run failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
