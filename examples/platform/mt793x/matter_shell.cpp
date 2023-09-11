/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "matter_shell.h"
#include <ChipShellCollection.h>
#include <FreeRTOS.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Engine.h>
#include <task.h>

using namespace ::chip;
using chip::Shell::Engine;

namespace {

#define SHELL_TASK_STACK_SIZE 2048
#define SHELL_TASK_PRIORITY 5
TaskHandle_t shellTaskHandle;
StackType_t shellStack[SHELL_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t shellTaskStruct;

void MatterShellTask(void * args)
{
    chip::Shell::Engine::Root().RunMainLoop();
}

} // namespace

extern "C" unsigned int sleep(unsigned int seconds)
{
    const TickType_t xDelay = 1000 * seconds / portTICK_PERIOD_MS;
    vTaskDelay(xDelay);
    return 0;
}

namespace chip {

void NotifyShellProcess()
{
    xTaskNotifyGive(shellTaskHandle);
}

void NotifyShellProcessFromISR(void)
{
    BaseType_t yieldRequired = pdFALSE;
    if (shellTaskHandle != NULL)
    {
        vTaskNotifyGiveFromISR(shellTaskHandle, &yieldRequired);
    }
    portYIELD_FROM_ISR(yieldRequired);
}

void WaitForShellActivity(void)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void startShellTask(void)
{
    int status = chip::Shell::Engine::Root().Init();
    assert(status == 0);

    // For now also register commands from shell_common (shell app).
    // TODO move at least OTCLI to default commands in lib/shell/commands
    cmd_misc_init();
    cmd_otcli_init();

    shellTaskHandle = xTaskCreateStatic(MatterShellTask, "matter_cli", ArraySize(shellStack), NULL, SHELL_TASK_PRIORITY, shellStack,
                                        &shellTaskStruct);
}

} // namespace chip
