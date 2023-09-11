/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/shell/Engine.h>

#include <crypto/RandUtils.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ChipShellCollection.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::Logging;

CHIP_ERROR cmd_echo(int argc, char ** argv)
{
    for (int i = 0; i < argc; i++)
    {
        streamer_printf(streamer_get(), "%s ", argv[i]);
    }
    streamer_printf(streamer_get(), "\n\r");
    return CHIP_NO_ERROR;
}

CHIP_ERROR cmd_log(int argc, char ** argv)
{
    for (int i = 0; i < argc; i++)
    {
        ChipLogProgress(chipTool, "%s", argv[i]);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR cmd_rand(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "%d\n\r", static_cast<int>(chip::Crypto::GetRandU8()));
    return CHIP_NO_ERROR;
}

static shell_command_t cmds_misc[] = {
    { &cmd_echo, "echo", "Echo back provided inputs" },
    { &cmd_log, "log", "Logging utilities" },
    { &cmd_rand, "rand", "Random number utilities" },
};

void cmd_misc_init()
{
    Engine::Root().RegisterCommands(cmds_misc, ArraySize(cmds_misc));
}
