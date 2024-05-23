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
#include <lib/shell/streamer.h>
#include <lib/shell/streamer_zephyr.h>
#include <platform/CHIPDeviceLayer.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {

K_MUTEX_DEFINE(sShellMutex);
K_CONDVAR_DEFINE(sCommandResultCondVar);
CHIP_ERROR sCommandResult;

// RAII helper for synchronizing access to resources shared between the Zephyr's shell thread,
// which reads and parses the user input, and the the Matter thread, which executes a Matter
// shell command and reports the result back to the shell thread.
class ShellGuard
{
public:
    ShellGuard() { k_mutex_lock(&sShellMutex, K_FOREVER); }
    ~ShellGuard() { k_mutex_unlock(&sShellMutex); }

    CHIP_ERROR WaitForCommandResult()
    {
        k_condvar_wait(&sCommandResultCondVar, &sShellMutex, K_FOREVER);
        return sCommandResult;
    }

    void PutCommandResult(CHIP_ERROR error)
    {
        sCommandResult = error;
        k_condvar_signal(&sCommandResultCondVar);
    }
};

void ExecCommandInMatterThread(intptr_t argvAsInt)
{
    char ** argv = reinterpret_cast<char **>(argvAsInt);
    int argc     = 0;

    while (argv[argc] != nullptr)
    {
        argc++;
    }

    ShellGuard shellGuard;
    shellGuard.PutCommandResult(Shell::Engine::Root().ExecCommand(argc, argv));
}

int ExecCommandInShellThread(const struct shell * shell, size_t argc, char ** argv)
{
    const CHIP_ERROR error = [shell, argv]() -> CHIP_ERROR {
        ShellGuard shellGuard;
        Shell::streamer_set_shell(shell);
        ReturnErrorOnFailure(PlatformMgr().ScheduleWork(ExecCommandInMatterThread, reinterpret_cast<intptr_t>(argv + 1)));

        return shellGuard.WaitForCommandResult();
    }();

    if (error != CHIP_NO_ERROR)
    {
        Shell::streamer_printf(Shell::streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", error.Format());
    }
    else
    {
        Shell::streamer_printf(Shell::streamer_get(), "Done\r\n");
    }

    return error == CHIP_NO_ERROR ? 0 : -ENOEXEC;
}

int RegisterCommands()
{
    Shell::Engine::Root().RegisterDefaultCommands();
    return 0;
}

} // namespace

SYS_INIT(RegisterCommands, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
SHELL_CMD_ARG_REGISTER(matter, NULL, "Matter commands", ExecCommandInShellThread, 1, CHIP_SHELL_MAX_TOKENS);

namespace chip {
namespace Shell {

void Engine::RunMainLoop()
{
    // Intentionally empty as Zephyr has its own thread handling shell.
}

} // namespace Shell
} // namespace chip
