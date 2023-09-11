/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Source implementation of meta shell commands for CHIP examples.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <CHIPVersion.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace Shell {

static CHIP_ERROR ExitHandler(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "Goodbye\r\n");
    exit(0);
    return CHIP_NO_ERROR;
}

static void AtExitShell()
{
    chip::DeviceLayer::PlatformMgr().Shutdown();
}

static CHIP_ERROR HelpHandler(int argc, char ** argv)
{
    Engine::Root().ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR VersionHandler(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "CHIP %s\r\n", CHIP_VERSION_STRING);
    return CHIP_NO_ERROR;
}

void RegisterMetaCommands()
{
    static shell_command_t sCmds[] = {
        { &ExitHandler, "exit", "Exit the shell application" },
        { &HelpHandler, "help", "List out all top level commands" },
        { &VersionHandler, "version", "Output the software version" },
    };

    std::atexit(AtExitShell);

    Engine::Root().RegisterCommands(sCmds, ArraySize(sCmds));
}

} // namespace Shell
} // namespace chip
