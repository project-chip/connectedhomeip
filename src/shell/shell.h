/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <stdarg.h>
#include <stddef.h>

#include "streamer.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Counts number of elements inside the array
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * @brief Callback for "extended" shell commands.
 *
 * @param cmd                   The shell command being executed.
 * @param argc                  Number of arguments passed.
 * @param argv                  Array of option strings. First option is always
 *                                  command name.
 * @param streamer              The streamer to write shell output to.
 *
 * @return                      0 on success; SYS_E[...] on failure.
 */
typedef int chip_shell_cmd_func_t(int argc, char * argv[]);

struct chip_shell_cmd
{
    chip_shell_cmd_func_t * cmd_func;
    const char * cmd_name;
    const char * cmd_help;
};

int chip_shell_register(const struct chip_shell_cmd * command_set, unsigned count);
void chip_shell_task(void * arg);

typedef int on_command_t(const struct chip_shell_cmd * command, void * arg);

void chip_shell_command_foreach(on_command_t * on_command, void * arg);

#ifdef __cplusplus
}
#endif
