/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Apache Software Foundation (ASF)
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
typedef int shell_cmd_func_t(int argc, char * argv[]);

/**
 * Descriptor structure for a single command.
 *
 * Typically a set of commands are defined as an array of this structure
 * and passed to the `shell_register()` during application initialization.
 *
 * An example command set definition follows:
 *
 * static const struct shell_cmd cmds[] = {
 *   { &cmd_echo, "echo", "Echo back provided inputs" },
 *   { &cmd_exit, "exit", "Exit the shell application" },
 *   { &cmd_help, "help", "List out all top level commands" },
 *   { &cmd_version, "version", "Output the software version" },
 * };
 */
typedef struct shell_cmd
{
    shell_cmd_func_t * cmd_func;
    const char * cmd_name;
    const char * cmd_help;
} shell_cmd_t;

/**
 * Execution callback for a shell command.
 *
 * @param command               The shell command being iterated.
 * @param arg                   A context variable passed to the iterator function.
 *
 * @return                      0 continue iteration; 1 break iteration.
 */
typedef int shell_cmd_iterator_t(const struct shell_cmd * command, void * arg);

/**
 * Execution callback for a shell command.
 *
 * @param on_command            An iterator callback to be called for each command.
 * @param arg                   A context variable to be passed to each command iterated.
 * @param streamer              The streamer to write shell output to.
 */
void shell_command_foreach(shell_cmd_iterator_t * on_command, void * arg);

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
int shell_line_tokenize(char * buffer, char ** tokens, int max_tokens);

/**
 * Registers a command set, or array of commands with the shell.
 *
 * @param command_set           An array of commands to add to the shell.
 * @param count                 The number of commands in the command set array.
 */
void shell_register(const struct shell_cmd * command_set, unsigned count);

/**
 * Main loop for shell.
 *
 * @param arg                   Unused context block for shell task to comply with task function syntax.
 */
void shell_task(void * arg);

} // namespace Shell
} // namespace chip
