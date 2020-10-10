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

/**
 *    @file
 *      The CLI interface of CHIP Lighting App
 */

#include "FreeRTOS.h"
#include "task.h"

#include <lib/shell/shell.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/RandUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/logging/CHIPLogging.h>

#include <ChipShellCollection.h>

#include <AppEvent.h>
#include <AppTask.h>
#include <LightingManager.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Shell;

namespace {

enum CLIEvents
{
    kCLIEvent_LightOn = 0,
    kCLIEvent_LightOff,
    kCLIEvent_LightToggle,

    kCLIEvent_FactoryReset,
};

void CLIAppEventHandler(AppEvent * event)
{
    switch (event->CLIEvent.Event)
    {
    case kCLIEvent_LightOn:
        LightingMgr().InitiateAction(LightingManager::ON_ACTION);
        break;
    case kCLIEvent_LightOff:
        LightingMgr().InitiateAction(LightingManager::OFF_ACTION);
        break;
    case kCLIEvent_LightToggle: {
        LightingManager::Action_t action;
        if (LightingMgr().IsTurnedOn())
        {
            action = LightingManager::OFF_ACTION;
        }
        else
        {
            action = LightingManager::ON_ACTION;
        }
        LightingMgr().InitiateAction(action);
        break;
    }
    case kCLIEvent_FactoryReset:
        ConfigurationMgr().InitiateFactoryReset();
        break;
    default:
        ChipLogError(Shell, "Unknown event received: %d", event->CLIEvent.Event);
        break;
    }
}

int cmd_light(int argc, char ** argv)
{
    // TODO: Add or remove multiple endpoints support.
    bool read_state = false;
    int end_point   = 1;
    AppEvent event;
    event.Type    = AppEvent::kEventType_CLI;
    event.Handler = CLIAppEventHandler;

    VerifyOrExit(argc > 0 && argc <= 2, streamer_printf(streamer_get(), "light on/off/toggle/state [endpoint=1]\n\r"));

    if (strcmp(argv[0], "on") == 0)
    {
        event.CLIEvent.Event = kCLIEvent_LightOn;
    }
    else if (strcmp(argv[0], "off") == 0)
    {
        event.CLIEvent.Event = kCLIEvent_LightOff;
    }
    else if (strcmp(argv[0], "toggle") == 0)
    {
        event.CLIEvent.Event = kCLIEvent_LightToggle;
    }
    else if (strcmp(argv[0], "state") == 0)
    {
        read_state = true;
    }

    if (argc == 2)
    {
        sscanf(argv[1], "%d", &end_point);
    }

    if (read_state)
    {
        streamer_printf(streamer_get(), "%d\n\r", LightingMgr().IsTurnedOn());
    }
    else
    {
        GetAppTask().PostEvent(&event);
    }
exit:
    streamer_printf(streamer_get(), "\n\r");
    return 0;
}

int cmd_app(int argc, char ** argv)
{
    int end_point = 1;
    AppEvent event;
    event.Type    = AppEvent::kEventType_CLI;
    event.Handler = CLIAppEventHandler;

    VerifyOrExit(argc > 0, streamer_printf(streamer_get(), "app subcommand|help\n\r"));

    if (strcmp(argv[0], "factoryrst") == 0)
    {
        event.CLIEvent.Event = kCLIEvent_FactoryReset;
    }
    else
    {
        streamer_printf(streamer_get(), "app factoryrst      : Do factory reset\n\r");
    }

    GetAppTask().PostEvent(&event);
exit:
    streamer_printf(streamer_get(), "\n\r");
    return 0;
}

shell_command_t cmd_lightingcli[] = {
    { &cmd_light, "light", "Lighting control" },
    { &cmd_app, "app", "App utilities" },
};

void cmd_app_init(void)
{
    shell_register(cmd_lightingcli, ArraySize(cmd_lightingcli));
}

} // namespace

namespace {

const size_t kShellTaskStackSize = 2048;
const int kShellTaskPriority     = 1;
TaskHandle_t sShellTaskHandle;

void LightingCLIMain(void * pvParameter)
{
    // Initialize the default streamer that was linked.
    const int rc = streamer_init(streamer_get());

    if (rc != 0)
    {
        ChipLogError(Shell, "Streamer initialization failed: %d", rc);
        return;
    }

    ChipLogDetail(Shell, "Initializing CHIP shell", rc);

    cmd_misc_init();
    cmd_app_init();
    cmd_btp_init();
    cmd_otcli_init();

    ChipLogDetail(Shell, "Run CHIP shell Task", rc);

    shell_task(NULL);
}

} // namespace

int StartShellTask()
{
    ret_code_t ret = NRF_SUCCESS;

    // Start App task.
    if (xTaskCreate(LightingCLIMain, "SHELL", kShellTaskStackSize / sizeof(StackType_t), NULL, kShellTaskPriority,
                    &sShellTaskHandle) != pdPASS)
    {
        ret = NRF_ERROR_NULL;
    }

    return ret;
}
