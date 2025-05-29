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

#include <lib/core/CHIPError.h>
#include <lib/shell/Engine.h>

extern "C" {
#include <shell.h>
}

using chip::Shell::Engine;

void cmd_matter(size_t argc, char ** argv)
{
    Engine::Root().ExecCommand(argc - 1, argv + 1);
}

SHELL_CMD_EXPORT_ALIAS(cmd_matter, matter, matter command);

namespace chip {
namespace Shell {

void Engine::RunMainLoop()
{
    // Intentionally empty as Zephyr has own thread handling shell
}

} // namespace Shell
} // namespace chip
