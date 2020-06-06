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
 *      Source implementation for a generic shell API for CHIP examples.
 */

#pragma once

#include "shell.h"

namespace chip {
namespace Shell {

/**
 * Registers a set of default commands with the shell:
 *    help      - list the top-level brief of all registered commands
 *    echo      - echo back all argument characters passed
 *    exit      - quit out of the shell
 *    version   - return the version of the CHIP library
 */
void shell_default_cmds_init();

} // namespace Shell
} // namespace chip
