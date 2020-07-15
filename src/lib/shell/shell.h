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

#include <stdarg.h>
#include <stddef.h>

#ifndef CHIP_SHELL_PROMPT
#define CHIP_SHELL_PROMPT "> "
#endif // CHIP_SHELL_PROMPT

#ifndef CHIP_SHELL_MAX_MODULES
#define CHIP_SHELL_MAX_MODULES 10
#endif // CHIP_SHELL_MAX_MODULES

#ifndef CHIP_SHELL_MAX_LINE_SIZE
#define CHIP_SHELL_MAX_LINE_SIZE 256
#endif // CHIP_SHELL_MAX_LINE_SIZE

#ifndef CHIP_SHELL_MAX_TOKENS
#define CHIP_SHELL_MAX_TOKENS 10
#endif // CHIP_SHELL_MAX_TOKENS

namespace chip {
namespace Shell {

/// Counts number of elements inside the array
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * Callback to execute an individual shell command.
 *
 * @param argc                  Number of arguments passed.
 * @param argv                  Array of option strings. The command name is not included.
 *
 * @return                      0 on success; CHIP_ERROR[...] on failure.
 */
typedef int shell_command_fn(int argc, char * argv[]);

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
 * @return                      0 continue iteration; 1 break iteration.
 */
typedef int shell_command_iterator_t(shell_command_t * command, void * arg);

class Shell
{
protected:
    static Shell theShellRoot;

    shell_command_t * _commandSet[CHIP_SHELL_MAX_MODULES];
    unsigned _commandSetSize[CHIP_SHELL_MAX_MODULES];
    unsigned _commandSetCount;

    /**
     * Registers a set of defaults commands (help) for all Shell and sub-Shell instances.
     *
     *    help      - list the top-level brief of all registered commands
     *    echo      - echo back all argument characters passed
     *    exit      - quit out of the shell
     *    version   - return the version of the CHIP library
     */
    void RegisterDefaultCommands();

public:
    Shell() {}

    /** Return the root singleton for the Shell command hierarchy. */
    static Shell & Root() { return theShellRoot; }

    /**
     * Execution callback for a shell command.
     *
     * @param on_command            An iterator callback to be called for each command.
     * @param arg                   A context variable to be passed to each command iterated.
     * @param streamer              The streamer to write shell output to.
     */
    void ForEachCommand(shell_command_iterator_t * on_command, void * arg);

    /**
     * Dispatch and execute the command for the given argument list.
     *
     * @param argc                  Number of arguments in argv.
     * @param argv                  Array of arguments in the tokenized command line to execute.
     *
     * @return                      0 on success; CHIP_ERROR[...] on failure.
     */
    int ExecCommand(int argc, char * argv[]);

    /**
     * Registers a command set, or array of commands with the shell.
     *
     * @param command_set           An array of commands to add to the shell.
     * @param count                 The number of commands in the command set array.
     */
    void RegisterCommands(shell_command_t * command_set, unsigned count);

    /**
     * Utility function for converting a raw line typed into a shell into an array of words or tokens.
     *
     * @param buffer                String of the raw line typed into shell.
     * @param tokens                Array of words to be created by the tokenizer.
     *                              This array will point to the same memory as passed in
     *                              via buffer.  Spaces will be replaced with NULL characters.
     * @param max_tokens            Maximum size of token array.
     *
     * @return                      Number of tokens generated (argc).
     */
    static int TokenizeLine(char * buffer, char ** tokens, int max_tokens);

    /**
     * Main loop for shell.
     *
     * @param arg                   Unused context block for shell task to comply with task function syntax.
     */
    static void TaskLoop(void * arg);
};

/** Utility macro for running ForEachCommand on root shell. */
static inline void shell_command_foreach(shell_command_iterator_t * on_command, void * arg)
{
    return Shell::Root().ForEachCommand(on_command, arg);
}

/** Utility macro for running ForEachCommand on Root shell. */
static inline void shell_register(shell_command_t * command_set, unsigned count)
{
    return Shell::Root().RegisterCommands(command_set, count);
}

/** Utility macro for to tokenize an input line. */
static inline int shell_line_tokenize(char * buffer, char ** tokens, int max_tokens)
{
    return Shell::TokenizeLine(buffer, tokens, max_tokens);
}

/** Utility macro to run main shell task loop. */
static inline void shell_task(void * arg)
{
    return Shell::TaskLoop(arg);
}

} // namespace Shell
} // namespace chip
