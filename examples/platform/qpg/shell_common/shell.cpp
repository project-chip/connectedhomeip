/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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
        xTaskCreateStatic(ShellTask::Main, "SHELL_TASK", MATTER_ARRAY_SIZE(shellStack), nullptr, 1, shellStack, &shellTaskStruct);
    if (sShellTaskHandle == nullptr)
    {
        return 1;
    }

    return 0;
}
