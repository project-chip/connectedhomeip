/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "nvs_flash.h"

#include <ChipShellCollection.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

using chip::Shell::Engine;

static void chip_shell_task(void * args)
{
    Engine::Root().RunMainLoop();
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    chip::Platform::MemoryInit();
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();

    int ret = Engine::Root().Init();
    VerifyOrDie(ret == 0);

    xTaskCreate(&chip_shell_task, "chip_shell", 2048, NULL, 5, NULL);
}
