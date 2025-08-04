/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "AppTask.h"
#include "ButtonManager.h"
#include <app/server/Server.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    InitCommonParts();

#if CONFIG_CHIP_LIB_SHELL
    int rc = Engine::Root().Init();
    if (rc != 0)
    {
        ChipLogError(AppServer, "Streamer initialization failed: %d", rc);
        return 1;
    }

    cmd_misc_init();
    cmd_otcli_init();
#endif

#if CHIP_SHELL_ENABLE_CMD_SERVER
    cmd_app_server_init();
#endif

#if CONFIG_CHIP_LIB_SHELL
    Engine::Root().RunMainLoop();
#endif

    return CHIP_NO_ERROR;
}

void AppTask::LinkButtons(ButtonManager & buttonManager)
{
    buttonManager.addCallback(FactoryResetButtonEventHandler, 0, true);
    buttonManager.addCallback(ExampleActionButtonEventHandler, 1, true);
    buttonManager.addCallback(StartBleAdvButtonEventHandler, 2, true);
}
