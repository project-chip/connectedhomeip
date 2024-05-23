/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
