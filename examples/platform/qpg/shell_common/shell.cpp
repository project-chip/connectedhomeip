/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell.h"

#include "task.h"
#include <FreeRTOS.h>

#include <ChipShellCollection.h>
#include <lib/shell/Engine.h>

#include <lib/support/CodeUtils.h>

#define APP_TASK_STACK_SIZE 8192
#define APP_TASK_PRIORITY 3

namespace {
TaskHandle_t sShellTaskHandle;

StackType_t shellStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t shellTaskStruct;
} // namespace

ShellTask ShellTask::sShellTask;

// using namespace ::chip;

int ShellTask::Init(void)
{
    int err = 0;

    err = (int) chip::Shell::streamer_init(chip::Shell::streamer_get());
    if (err != 0)
    {
        return err;
    }

    cmd_misc_init();
    cmd_otcli_init();
#if CHIP_SHELL_ENABLE_CMD_SERVER
    cmd_app_server_init();
#endif // CHIP_SHELL_ENABLE_CMD_SERVER

    return err;
}

void ShellTask::Main(void * args)
{

    int err = sShellTask.Init();
    if (err != 0)
    {
        // ChipLogError(NotSpecified, "Shell task init failed");
        return;
    }

    chip::Shell::Engine::Root().RunMainLoop();
}

int ShellTask::Start(void)
{

    // Start App task.
    sShellTaskHandle =
        xTaskCreateStatic(ShellTask::Main, "SHELL_TASK", ArraySize(shellStack), nullptr, 1, shellStack, &shellTaskStruct);
    if (sShellTaskHandle == nullptr)
    {
        return 1;
    }

    return 0;
}
