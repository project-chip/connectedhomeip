/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "launch_shell.h"

#include "FreeRTOS.h"
#include "task.h"

#include <lib/shell/Engine.h>
#include <platform/CHIPDeviceLayer.h>

namespace {

void MatterShellTask(void * args)
{
    chip::Shell::Engine::Root().RunMainLoop();
}

} // namespace

namespace chip {

void LaunchShell()
{
    if (chip::Shell::Engine::Root().Init() < 0)
    {
        ChipLogError(DeviceLayer, "Failed to initialize shell engine!");
        return;
    }
    xTaskCreate(MatterShellTask, "matter_shell", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
}

} // namespace chip
