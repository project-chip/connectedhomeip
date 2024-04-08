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

#include "matter_shell.h"
#include <ChipShellCollection.h>
#include <cmsis_os2.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Engine.h>
#include <sl_cmsis_os2_common.h>

using namespace ::chip;
using chip::Shell::Engine;

namespace {

constexpr uint32_t kShellProcessFlag   = 1;
constexpr uint32_t kShellTaskStackSize = 2048;
uint8_t shellTaskStack[kShellTaskStackSize];
osThread_t shellTaskControlBlock;
constexpr osThreadAttr_t kShellTaskAttr = { .name       = "shell",
                                            .attr_bits  = osThreadDetached,
                                            .cb_mem     = &shellTaskControlBlock,
                                            .cb_size    = osThreadCbSize,
                                            .stack_mem  = shellTaskStack,
                                            .stack_size = kShellTaskStackSize,
                                            .priority   = osPriorityBelowNormal };
osThreadId_t shellTaskHandle;

void MatterShellTask(void * args)
{
    chip::Shell::Engine::Root().RunMainLoop();
}

} // namespace

namespace chip {

void NotifyShellProcess()
{
    // This function may be called from Interrupt Service Routines.
    osThreadFlagsSet(shellTaskHandle, kShellProcessFlag);
}

void WaitForShellActivity()
{
    osThreadFlagsWait(kShellProcessFlag, osFlagsWaitAny, osWaitForever);
}

void startShellTask()
{
    int status = chip::Shell::Engine::Root().Init();
    assert(status == 0);

    // For now also register commands from shell_common (shell app).
    // TODO move at least OTCLI to default commands in lib/shell/commands
    cmd_misc_init();
    cmd_otcli_init();

    shellTaskHandle = osThreadNew(MatterShellTask, nullptr, &kShellTaskAttr);
    VerifyOrDie(shellTaskHandle);
}

} // namespace chip
