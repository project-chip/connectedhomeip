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

#include <lib/support/Span.h>

namespace chip {
namespace Shell {

/**
 * Shell command descriptor structure.
 *
 * Typically a set of commands is defined as an array of this structure and registered
 * at the shell root using the @c Shell::Engine::Root().RegisterCommands() method, or
 * used to construct a @c CommandSet.
 *
 * Usage example:
 *
 * @code
 * static Shell::Command cmds[] = {
 *   { &cmd_echo, "echo", "Echo back provided inputs" },
 *   { &cmd_exit, "exit", "Exit the shell application" },
 *   { &cmd_help, "help", "List out all top level commands" },
 *   { &cmd_version, "version", "Output the software version" },
 * };
 * @endcode
 */
struct Command
{
    /**
     * Shell command handler function type.
     *
     * @param argc  Number of arguments in argv.
     * @param argv  Array of arguments in the tokenized command line to execute.
     */
    using Handler = CHIP_ERROR (*)(int argc, char * argv[]);

    Handler cmd_func;
    const char * cmd_name;
    const char * cmd_help;
};

// DEPRECATED:
// shell_command_t is used in many examples, so keep it for backwards compatibility
using shell_command_t = const Command;

} // namespace Shell
} // namespace chip
