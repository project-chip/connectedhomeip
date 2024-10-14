/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/shell/CommandSet.h>
#include <lib/shell/streamer.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Shell {

CHIP_ERROR CommandSet::ExecCommand(int argc, char * argv[]) const
{
    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        ShowHelp();
        return CHIP_NO_ERROR;
    }

    for (const Command & command : mCommands)
    {
        if (strcmp(argv[0], command.cmd_name) == 0)
        {
            return command.cmd_func(argc - 1, argv + 1);
        }
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

void CommandSet::ShowHelp() const
{
    for (const Command & command : mCommands)
    {
        streamer_printf(streamer_get(), "  %-15s %s\r\n", command.cmd_name, command.cmd_help);
    }
}

} // namespace Shell
} // namespace chip
