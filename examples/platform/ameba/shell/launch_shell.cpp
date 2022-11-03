/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "launch_shell.h"

#include "FreeRTOS.h"
#include "task.h"

#include <lib/shell/Engine.h>

namespace {

void MatterShellTask(void * args)
{
    chip::Shell::Engine::Root().RunMainLoop();
}

} // namespace

namespace chip {

void LaunchShell()
{
    chip::Shell::Engine::Root().Init();
    xTaskCreate(MatterShellTask, "matter_shell", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
}

} // namespace chip
