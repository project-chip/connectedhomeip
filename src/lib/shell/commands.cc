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
 *      Source implementation for a generic shell API for CHIP examples.
 */

#include "CHIPVersion.h"

#include "commands.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace chip {
namespace Shell {

int cmd_echo(int argc, char ** argv)
{
    int i = 0;
    while (i < argc)
    {
        streamer_printf(streamer_get(), "%s ", argv[i++]);
    }
    streamer_printf(streamer_get(), "\n\r");
    return 0;
}

int cmd_exit(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "Goodbye\n\r");
    exit(0);
    return 0;
}

int cmd_help_iterator(const struct shell_cmd * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_help(int argc, char ** argv)
{
    shell_command_foreach(cmd_help_iterator, NULL);
    return 0;
}

int cmd_version(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "CHIP %s\n\r", CHIP_VERSION_STRING);
    return 0;
}

static const struct shell_cmd cmds[] = {
    { &cmd_echo, "echo", "Echo back provided inputs" },
    { &cmd_exit, "exit", "Exit the shell application" },
    { &cmd_help, "help", "List out all top level commands" },
    { &cmd_version, "version", "Output the software version" },
};

void shell_default_cmds_init()
{
    shell_register(cmds, ARRAY_SIZE(cmds));
}

} // namespace Shell
} // namespace chip
