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

#include <lib/core/CHIPConfig.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <ChipShellCollection.h>

#include "cmsis_os2.h"
#include "mbedtls/platform.h"

using namespace ::chip;
using namespace ::chip::Shell;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging::Platform;

static void app_thread(void * argument)
{
    // Initialize the default streamer that was linked.
    auto ret = Engine::Root().Init();
    if (ret)
    {
        ChipLogError(Shell, "Streamer initialization failed [%d]", ret);
        return;
    }

    cmd_misc_init();

    ChipLogProgress(Shell, "Open IoT SDK shell example application run");

#if NDEBUG
    chip::Logging::SetLogFilter(chip::Logging::LogCategory::kLogCategory_None);
#endif

    Engine::Root().RunMainLoop();
}

int main()
{
    int ret        = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

    osThreadId_t appThread;
    osKernelState_t state;

    ChipLogProgress(Shell, "Open IoT SDK shell example application start");

    ret = mbedtls_platform_setup(NULL);
    if (ret)
    {
        ChipLogError(Shell, "Mbed TLS platform initialization failed: %d", ret);
        return EXIT_FAILURE;
    }

    ret = osKernelInitialize();
    if (ret != osOK)
    {
        ChipLogError(Shell, "osKernelInitialize failed: %d", ret);
        return EXIT_FAILURE;
    }

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Shell, "Memory initialization failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Shell, "Chip stack initialization failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Shell, "Chip stack start failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    static const osThreadAttr_t thread_attr = {
        .stack_size = 8 * 1024 // Allocate enough stack for app thread
    };

    appThread = osThreadNew(app_thread, NULL, &thread_attr);
    if (appThread == NULL)
    {
        ChipLogError(Shell, "Failed to create app thread");
        return EXIT_FAILURE;
    }

    state = osKernelGetState();
    if (state == osKernelReady)
    {
        ret = osKernelStart();
        if (ret != osOK)
        {
            ChipLogError(Shell, "Failed to start kernel: %d", ret);
            return EXIT_FAILURE;
        }
    }
    else
    {
        ChipLogError(Shell, "Kernel not ready: %d", state);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
