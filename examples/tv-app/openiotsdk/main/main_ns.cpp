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

#include <lib/support/logging/CHIPLogging.h>

#include "openiotsdk_platform.h"

#include "AppTv.h"

#ifdef CHIP_TV_APP_SHELL_CMD_ENABLE
#include "AppTvShellCommands.h"
#include <lib/shell/Engine.h>
using namespace ::chip::Shell;
#endif // CHIP_TV_APP_SHELL_CMD_ENABLE

using namespace ::chip;

int main()
{
    if (openiotsdk_platform_init())
    {
        ChipLogError(NotSpecified, "Open IoT SDK platform initialization failed");
        return EXIT_FAILURE;
    }

    if (openiotsdk_chip_init())
    {
        ChipLogError(NotSpecified, "Open IoT SDK CHIP stack initialization failed");
        return EXIT_FAILURE;
    }

    ChipLogProgress(NotSpecified, "Open IoT SDK tv-app example application start");

    if (openiotsdk_network_init(true))
    {
        ChipLogError(NotSpecified, "Network initialization failed");
        return EXIT_FAILURE;
    }

    if (openiotsdk_chip_run())
    {
        ChipLogError(NotSpecified, "CHIP stack run failed");
        return EXIT_FAILURE;
    }

#ifdef CHIP_TV_APP_SHELL_CMD_ENABLE
    // Initialize the default streamer that was linked.
    int ret = Engine::Root().Init();
    if (ret)
    {
        ChipLogError(Shell, "Streamer initialization failed [%d]", ret);
        return EXIT_FAILURE;
    }

    RegisterAppTvCommands();
#endif // CHIP_TV_APP_SHELL_CMD_ENABLE

    CHIP_ERROR err = AppTvInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "App TV initialization failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    ChipLogProgress(NotSpecified, "Open IoT SDK tv-app example application run");

#ifdef CHIP_TV_APP_SHELL_CMD_ENABLE
    Engine::Root().RunMainLoop();
#else
    while (true)
    {
        // Add forever delay to ensure proper workload for this thread
        osDelay(osWaitForever);
    }
#endif // CHIP_TV_APP_SHELL_CMD_ENABLE

    openiotsdk_chip_shutdown();

    return EXIT_SUCCESS;
}
