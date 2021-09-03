/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/core/CHIPCore.h>
#include <lib/shell/Engine.h>
#include <platform/mbed/Logging.h>
#include <support/logging/CHIPLogging.h>

#include "ChipShellMbedCollection.h"
#include "rtos/Thread.h"
#include <ChipShellCollection.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace ::chip::Shell;
using namespace ::chip::Platform;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging::Platform;

rtos::Thread shellThread{ osPriorityNormal, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE,
                          /* memory provided */ nullptr, "Shell" };

int main()
{
    mbed_logging_init();

    MemoryInit();
    PlatformMgr().InitChipStack();
    PlatformMgr().StartEventLoopTask();
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    ConnectivityManagerImpl().StartWiFiManagement();
#endif

    // Initialize the default streamer that was linked.
    const int rc = streamer_init(streamer_get());
    if (rc != 0)
    {
        ChipLogError(Shell, "Streamer initialization failed: %d", rc);
        return rc;
    }

    cmd_misc_init();
    cmd_otcli_init();
    cmd_ping_init();
    cmd_send_init();
    cmd_mbed_init();

    auto error = shellThread.start([] { Engine::Root().RunMainLoop(); });
    if (error != osOK)
    {
        ChipLogError(Shell, "Run shell thread failed: %d", (int) error);
        return error;
    }

    return 0;
}
