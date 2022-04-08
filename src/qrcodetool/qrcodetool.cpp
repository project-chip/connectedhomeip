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

#include <lib/support/logging/CHIPLogging.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "qrcodetool_command_manager.h"

static int match_command(const char * command_name, const char * name)
{
    return !strncmp(command_name, name, strlen(name));
}

static int help(int argc, char ** argv)
{
    qrcodetool_command_t * cmd = nullptr;
    for (cmd = commands; cmd->c_name != nullptr; cmd++)
    {
        ChipLogDetail(chipTool, "%s\t%s\n", cmd->c_name, cmd->c_help);
    }
    return 0;
}

static int usage(const char * prog_name)
{
    ChipLogDetail(chipTool,
                  "Usage: %s [-h] [command] [opt ...]\n"
                  "%s commands are:\n",
                  prog_name, prog_name);
    help(0, nullptr);
    return 2;
}

static int execute_command(int argc, char ** argv)
{
    if (argc == 0)
    {
        return -1;
    }
    const qrcodetool_command_t * command_to_execute = nullptr;
    bool found                                      = false;

    for (command_to_execute = commands; command_to_execute->c_name; command_to_execute++)
    {
        if (match_command(command_to_execute->c_name, argv[0]))
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        ChipLogDetail(chipTool, "Executing cmd %s\n", command_to_execute->c_name);
        return command_to_execute->c_func(argc, argv);
    }

    return help(0, nullptr);
}

int main(int argc, char ** argv)
{
    int result  = 0;
    int do_help = 0;
    int ch;

    /* Remember my name. */
    char * prog_name = strrchr(argv[0], '/');
    prog_name        = prog_name ? prog_name + 1 : argv[0];
    /* Do getopt stuff for global options. */
    optind = 1;

    while ((ch = getopt(argc, argv, "h")) != -1)
    {
        switch (ch)
        {
        case 'h':
            do_help = 1;
            break;

        case '?':
        default:
            return usage(prog_name);
        }
    }

    argc -= optind;
    argv += optind;

    if (do_help)
    {
        /* Munge argc/argv so that argv[0] is something. */
        result = help(0, nullptr);
    }
    else if (argc > 0)
    {
        result = execute_command(argc, argv);
    }
    else
    {
        result = usage(prog_name);
    }
    return result;
}
