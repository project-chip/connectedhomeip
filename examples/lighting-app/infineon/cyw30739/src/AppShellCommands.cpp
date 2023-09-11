/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LightingManager.h"
#include <lib/shell/commands/Help.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::Shell;

static CHIP_ERROR AppCommandHelpHandler(int argc, char * argv[]);
static CHIP_ERROR AppCommandLightingHandler(int argc, char * argv[]);
static CHIP_ERROR AppCommandDispatch(int argc, char * argv[]);

static chip::Shell::Engine sAppSubcommands;

void RegisterAppShellCommands(void)
{
    static const shell_command_t sAppSubCommands[] = {
        {
            .cmd_func = AppCommandHelpHandler,
            .cmd_name = "help",
            .cmd_help = "Usage: app <subcommand>",
        },
        {
            .cmd_func = AppCommandLightingHandler,
            .cmd_name = "light",
            .cmd_help = "Usage: app light [on|off|toggle]",
        },
    };

    static const shell_command_t sAppCommand = {
        .cmd_func = AppCommandDispatch,
        .cmd_name = "app",
        .cmd_help = "App commands",
    };

    sAppSubcommands.RegisterCommands(sAppSubCommands, ArraySize(sAppSubCommands));

    Engine::Root().RegisterCommands(&sAppCommand, 1);
}

CHIP_ERROR AppCommandHelpHandler(int argc, char * argv[])
{
    sAppSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppCommandLightingHandler(int argc, char * argv[])
{
    if (argc == 0)
    {
        streamer_printf(streamer_get(), "The light is %s\n", LightMgr().IsLightOn() ? "on" : "off");
    }
    else if (strcmp(argv[0], "on") == 0)
    {
        streamer_printf(streamer_get(), "Turning the light on ...\n");
        LightMgr().InitiateAction(LightingManager::ACTOR_APP_CMD, LightingManager::ON_ACTION, 0);
    }
    else if (strcmp(argv[0], "off") == 0)
    {
        streamer_printf(streamer_get(), "Turning the light off ...\n");
        LightMgr().InitiateAction(LightingManager::ACTOR_APP_CMD, LightingManager::OFF_ACTION, 0);
    }
    else if (strcmp(argv[0], "toggle") == 0)
    {
        streamer_printf(streamer_get(), "Toggling the light ...\n");
        if (LightMgr().IsLightOn())
            LightMgr().InitiateAction(LightingManager::ACTOR_APP_CMD, LightingManager::OFF_ACTION, 0);
        else
            LightMgr().InitiateAction(LightingManager::ACTOR_APP_CMD, LightingManager::ON_ACTION, 0);
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
