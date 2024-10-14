/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Google LLC
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
 *      Header that defines a generic shell API for CHIP examples
 */

#pragma once

#include "streamer.h"

#include <lib/core/CHIPError.h>
#include <lib/shell/Command.h>

#include <stdarg.h>
#include <stddef.h>

#ifndef CHIP_SHELL_PROMPT
#define CHIP_SHELL_PROMPT "> "
#endif // CHIP_SHELL_PROMPT

#ifndef CHIP_SHELL_MAX_MODULES
#define CHIP_SHELL_MAX_MODULES 32
#endif // CHIP_SHELL_MAX_MODULES

#ifndef CHIP_SHELL_MAX_LINE_SIZE
#define CHIP_SHELL_MAX_LINE_SIZE 256
#endif // CHIP_SHELL_MAX_LINE_SIZE

#ifndef CHIP_SHELL_MAX_TOKENS
#define CHIP_SHELL_MAX_TOKENS 10
#endif // CHIP_SHELL_MAX_TOKENS

namespace chip {
namespace Shell {

/**
 * Execution callback for a shell command.
 *
 * @param command               The shell command being iterated.
 * @param arg                   A context variable passed to the iterator function.
 *
 * @return                      CHIP_NO_ERROR to continue iteration; anything else to break iteration.
 */
typedef CHIP_ERROR shell_command_iterator_t(shell_command_t * command, void * arg);

class Engine
{
protected:
    static Engine theEngineRoot;

    shell_command_t * _commandSet[CHIP_SHELL_MAX_MODULES];
    unsigned _commandSetSize[CHIP_SHELL_MAX_MODULES];
    unsigned _commandSetCount;

public:
    Engine() {}

    /** Return the root singleton for the Shell command hierarchy. */
    static Engine & Root() { return theEngineRoot; }

    /**
     * Registers a set of defaults commands (help) for all Shell and sub-Shell instances.
     *
     *    help      - list the top-level brief of all registered commands
     *    echo      - echo back all argument characters passed
     *    exit      - quit out of the shell
     *    version   - return the version of the CHIP library
     */
    void RegisterDefaultCommands();

    /**
     * Execution callback for a shell command.
     *
     * @param on_command            An iterator callback to be called for each command.
     * @param arg                   A context variable to be passed to each command iterated.
     */
    void ForEachCommand(shell_command_iterator_t * on_command, void * arg);

    /**
     * Dispatch and execute the command for the given argument list.
     *
     * @param argc                  Number of arguments in argv.
     * @param argv                  Array of arguments in the tokenized command line to execute.
     *
     * @return                      CHIP_NO_ERROR on success; CHIP_ERROR[...] on failure.
     */
    CHIP_ERROR ExecCommand(int argc, char * argv[]);

    /**
     * Registers a command set, or array of commands with the shell.
     *
     * @param command_set           An array of commands to add to the shell.
     * @param count                 The number of commands in the command set array.
     */
    void RegisterCommands(shell_command_t * command_set, unsigned count);

    /**
     * Runs the shell mainloop. Will display the prompt and enable interaction.
     *
     * @note This is a blocking call and will not return until user types "exit"
     *
     */
    void RunMainLoop();

    /**
     * Initialize the Shell::Engine.
     *
     * Activates the linked streamer, registers default commands, and sets up exit handlers.
     *
     * @return 0 for success, otherwise failed.
     */
    int Init();

private:
    static void ProcessShellLineTask(intptr_t context);
};

} // namespace Shell
} // namespace chip
