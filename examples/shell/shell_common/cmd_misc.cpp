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
    Engine::Root().RegisterCommands(cmds_misc, MATTER_ARRAY_SIZE(cmds_misc));
}
