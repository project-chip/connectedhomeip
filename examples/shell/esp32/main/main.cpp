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

#include "esp_console.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "linenoise/linenoise.h"
#include "nvs_flash.h"
#include "support/CHIPMem.h"

#include <ChipShellCollection.h>
#include <lib/shell/shell_core.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

static void chip_shell_task(void * args)
{
    chip::Platform::MemoryInit();
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();
    int ret = chip::Shell::streamer_init(chip::Shell::streamer_get());
    assert(ret == 0);
    cmd_ping_init();
    while (true)
    {
        const char * prompt = LOG_COLOR_I "> " LOG_RESET_COLOR;
        char * line         = linenoise(prompt);
        if (line == NULL || strlen(line) == 0)
        {
            continue;
        }
        linenoiseHistoryAdd(line);
        int ret;
        esp_console_run(line, &ret);
        if (ret)
        {
            char errorStr[160];
            bool errorStrFound = chip::FormatCHIPError(errorStr, sizeof(errorStr), ret);
            if (!errorStrFound)
            {
                errorStr[0] = 0;
            }
            printf("Error: %s\n", errorStr);
        }
        else
        {
            printf("Done\n");
        }

        linenoiseFree(line);
    }
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    xTaskCreate(&chip_shell_task, "chip_shell", 8192, NULL, 5, NULL);
}
