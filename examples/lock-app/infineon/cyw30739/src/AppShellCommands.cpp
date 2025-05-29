/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include <LockManager.h>
#include <lib/shell/commands/Help.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::Shell;

static CHIP_ERROR AppCommandHelpHandler(int argc, char * argv[]);
static CHIP_ERROR AppCommandLockHandler(int argc, char * argv[]);
static CHIP_ERROR AppCommandDispatch(int argc, char * argv[]);

static chip::Shell::Engine sAppSubcommands;

chip::EndpointId endpointId = DOOR_LOCK_SERVER_ENDPOINT;

void RegisterAppShellCommands(void)
{
    static const shell_command_t sAppSubCommands[] = {
        {
            .cmd_func = AppCommandHelpHandler,
            .cmd_name = "help",
            .cmd_help = "Usage: app <subcommand>",
        },
        {
            .cmd_func = AppCommandLockHandler,
            .cmd_name = "lock",
            .cmd_help = "Usage: app lock [on|off|toggle]",
        },
    };

    static const shell_command_t sAppCommand = {
        .cmd_func = AppCommandDispatch,
        .cmd_name = "app",
        .cmd_help = "App commands",
    };

    sAppSubcommands.RegisterCommands(sAppSubCommands, MATTER_ARRAY_SIZE(sAppSubCommands));

    Engine::Root().RegisterCommands(&sAppCommand, 1);
}

CHIP_ERROR AppCommandHelpHandler(int argc, char * argv[])
{
    sAppSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppCommandLockHandler(int argc, char * argv[])
{
    if (argc == 0)
    {
        streamer_printf(streamer_get(), "The lock is %s\n", LockMgr().NextState() ? "unlocked" : "locked");
    }
    else if (strcmp(argv[0], "on") == 0)
    {
        streamer_printf(streamer_get(), "Lock ...\n");
        LockMgr().InitiateAction(AppEvent::kEventType_app, LockManager::LOCK_ACTION);
    }
    else if (strcmp(argv[0], "off") == 0)
    {
        streamer_printf(streamer_get(), "Unlock ...\n");
        LockMgr().InitiateAction(AppEvent::kEventType_app, LockManager::UNLOCK_ACTION);
    }
    else if (strcmp(argv[0], "toggle") == 0)
    {
        streamer_printf(streamer_get(), "Toggling the lock ...\n");
        if (LockMgr().NextState())
            LockMgr().InitiateAction(AppEvent::kEventType_app, LockManager::LOCK_ACTION);
        else
            LockMgr().InitiateAction(AppEvent::kEventType_app, LockManager::UNLOCK_ACTION);
    }
    else if (strcmp(argv[0], "open") == 0)
    {
        streamer_printf(streamer_get(), "open ...\n");
        LockMgr().SetDoorState(endpointId, DoorStateEnum::kDoorOpen);
    }
    else if (strcmp(argv[0], "close") == 0)
    {
        streamer_printf(streamer_get(), "close ...\n");
        LockMgr().SetDoorState(endpointId, DoorStateEnum::kDoorClosed);
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppCommandDispatch(int argc, char * argv[])
{
    if (argc == 0)
    {
        AppCommandHelpHandler(argc, argv);
        return CHIP_NO_ERROR;
    }
    return sAppSubcommands.ExecCommand(argc, argv);
}
