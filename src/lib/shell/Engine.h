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

#include <cstdio>
#include <cstring>
#include <lib/core/CHIPError.h>
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

#ifndef CHIP_SHELL_MAX_CMD_COMPLETIONS
#define CHIP_SHELL_MAX_CMD_COMPLETIONS 32
#endif // CHIP_SHELL_MAX_CMD_COMPLETIONS

namespace chip {
namespace Shell {

/**
 * Callback to execute an individual shell command.
 *
 * @param argc                  Number of arguments passed.
 * @param argv                  Array of option strings. The command name is not included.
 *
 * @return                      0 on success; CHIP_ERROR[...] on failure.
 */
typedef CHIP_ERROR shell_command_fn(int argc, char * argv[]);

/**
 * Descriptor structure for a single command.
 *
 * Typically a set of commands are defined as an array of this structure
 * and passed to the `shell_register()` during application initialization.
 *
 * An example command set definition follows:
 *
 * static shell_command_t cmds[] = {
 *   { &cmd_echo, "echo", "Echo back provided inputs" },
 *   { &cmd_exit, "exit", "Exit the shell application" },
 *   { &cmd_help, "help", "List out all top level commands" },
 *   { &cmd_version, "version", "Output the software version" },
 * };
 */
struct shell_command
{
    shell_command_fn * cmd_func;
    const char * cmd_name;
    const char * cmd_help;
};

typedef const struct shell_command shell_command_t;

/**
 * Execution callback for a shell command.
 *
 * @param command               The shell command being iterated.
 * @param arg                   A context variable passed to the iterator function.
 *
 * @return                      CHIP_NO_ERROR to continue iteration; anything else to break iteration.
 */
typedef CHIP_ERROR shell_command_iterator_t(shell_command_t * command, void * arg);

class Engine;

/**
 * A map of the Engine instances and the command prefix that they correspond to.
 * The struct itself is a linked list node.
 *
 *  prefix:     The command prefix (until n-1th token of full command) e.g. "device",
 *              or "dns resolve".
 *  enginev:    An array of the shells that have registered commands under the preifx.
 *  enginec:    The number of shells that have registered commands under the preifx.
 *  next:       The next shell in the list.
 *
 */
typedef struct shell_map
{
    const char * prefix;
    Engine * enginev[CHIP_SHELL_MAX_MODULES];
    size_t enginec = 0;
    shell_map * next;
} shell_map_t;

/**
 * A context object for passing request and receiving results with GetCmdCompletion.
 *
 *  line_buf:           The user input command to request for completion. If the applications prepends
 *                      "matter " to all matter commands, please remove the "matter " prefix from the buffer.
 *  ret_prefix:         The returned command prefix (up until the last space, not included).
 *  cmdv:               The command completion candidates unter the prefix in "ret_prefix".
 *  cmdc:               The number of command completion candidates in cmdv.
 *
 * Initialization:
 *
 *  cmd_completion_context context = cmd_completion_context("dns browse c");
 *
 */
typedef struct cmd_completion_context
{
    const char * line_buf;
    const char * ret_prefix;
    shell_command_t * cmdv[CHIP_SHELL_MAX_CMD_COMPLETIONS];
    size_t cmdc = 0;

    cmd_completion_context(){};
    cmd_completion_context(const char * _line_buf) { line_buf = _line_buf; };
} cmd_completion_context;

class Engine
{
protected:
    static Engine theEngineRoot;
    static shell_map_t * theShellMapListHead;
    shell_command_t * _commands[CHIP_SHELL_MAX_MODULES];
    unsigned _commandCount;

    static void InsertShellMap(char const * prefix, Engine * shell);

public:
    Engine(){};

    /** Return the root singleton for the Shell command hierarchy. */
    static Engine & Root() { return theEngineRoot; };

    /**
     * Get command completions by command prefix.
     *
     * @param context               The command completion context to pass request and receive results.
     *
     * @return                      CHIP_ERROR error code.
     */
    static CHIP_ERROR GetCommandCompletions(cmd_completion_context * context);

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
     * @param prefix                The prefix of this command set in the full command path.
     *                              e.g. "matter base64" is prefix for "endcode" and "decode".
     *                              Use double quoted empty string `""` or nullptr for root commands.
     */
    void RegisterCommands(shell_command_t * command_set, unsigned count, const char * prefix);

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
