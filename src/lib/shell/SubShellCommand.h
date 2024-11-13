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

#pragma once

#include <lib/shell/CommandSet.h>

namespace chip {
namespace Shell {

/**
 * Templatized shell command handler that runs one of the provided subcommands.
 *
 * The array of subcommands is provided as a non-type template parameter.
 *
 * The first argument is used to select the subcommand to be executed and
 * the remaining arguments are forwarded to the subcommand's handler.
 * If no argument has been provided or the first argument is "help", then
 * the function prints help text for each subcommand and returns no error.
 *
 * Usage example:
 * @code
 * constexpr Command subCommands[3] = {
 *   {handler_a, "cmd_a", "command a help text"},
 *   {handler_b, "cmd_b", "command b help text"},
 *   {handler_c, "cmd_c", "command c help text"},
 * };
 *
 * // Execute the matching subcommand
 * SubShellCommand<3, subCommands>(argc, argv);
 * @endcode
 *
 * @param argc  Number of arguments in argv.
 * @param argv  Array of arguments in the tokenized command line to execute.
 */
template <size_t N, const Command (&C)[N]>
inline CHIP_ERROR SubShellCommand(int argc, char ** argv)
{
    static constexpr CommandSet commandSet(C);

    return commandSet.ExecCommand(argc, argv);
}

} // namespace Shell
} // namespace chip
