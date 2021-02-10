/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *      Source implementation of default shell commands for CHIP examples.
 */

#include "CHIPVersion.h"

#include "shell.h"
#include <support/CodeUtils.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace chip {
namespace Shell {

int cmd_exit(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "Goodbye\n\r");
    exit(0);
    return 0;
}

int cmd_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_help(int argc, char ** argv)
{
    shell_command_foreach(cmd_help_iterator, nullptr);
    return 0;
}

int cmd_version(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "CHIP %s\n\r", CHIP_VERSION_STRING);
    return 0;
}

static shell_command_t cmds[] = {
    { &cmd_exit, "exit", "Exit the shell application" },
    { &cmd_help, "help", "List out all top level commands" },
    { &cmd_version, "version", "Output the software version" },
};

void Shell::RegisterDefaultCommands()
{
    RegisterCommands(cmds, ArraySize(cmds));
}

} // namespace Shell
} // namespace chip
