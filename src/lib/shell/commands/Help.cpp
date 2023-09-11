/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Header that defines default shell commands for CHIP examples
 */

#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/shell/streamer.h>

namespace chip {
namespace Shell {

CHIP_ERROR PrintCommandHelp(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\r\n", command->cmd_name, command->cmd_help);
    return CHIP_NO_ERROR;
}

} // namespace Shell
} // namespace chip
