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
#include <init.h>
#include <shell/shell.h>

#include <lib/core/CHIPError.h>
#include <lib/shell/Engine.h>
#include <lib/shell/streamer_zephyr.h>

using chip::Shell::Engine;

static int cmd_matter(const struct shell * shell, size_t argc, char ** argv)
{
    chip::Shell::streamer_set_shell(shell);
    return (Engine::Root().ExecCommand(argc - 1, argv + 1) == CHIP_NO_ERROR) ? 0 : -ENOEXEC;
}

static int RegisterCommands(const struct device * dev)
{
    Engine::Root().RegisterDefaultCommands();
    return 0;
}

SYS_INIT(RegisterCommands, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

SHELL_CMD_ARG_REGISTER(matter, NULL, "Matter commands", cmd_matter, 1, 4);

namespace chip {
namespace Shell {

void Engine::RunMainLoop()
{
    // Intentionally empty as Zephyr has own thread handling shell
}

} // namespace Shell
} // namespace chip
