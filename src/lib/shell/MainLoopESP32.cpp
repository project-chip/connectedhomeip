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

#include <lib/shell/Engine.h>
#include <platform/CHIPDeviceLayer.h>

using chip::DeviceLayer::PlatformMgr;
using chip::Shell::Engine;

namespace {

class ShellLineArgs
{
public:
    ShellLineArgs(char * line, TaskHandle_t source_task) : m_line(line), m_source_task(source_task) {}
    char * GetLine() { return m_line; }
    void WaitShellProcessDone() { ulTaskNotifyTake(pdTRUE, portMAX_DELAY); }
    void NotifyShellProcessDone() { xTaskNotifyGive(m_source_task); }

private:
    char * m_line;
    TaskHandle_t m_source_task;
};

void ProcessShellLine(intptr_t context)
{
    ShellLineArgs * shellArgs = reinterpret_cast<ShellLineArgs *>(context);
    int ret;
    esp_console_run(shellArgs->GetLine(), &ret);
    if (ret)
    {
        printf("Error: %d\r\n", ret);
    }
    else
    {
        printf("Done\r\n");
    }

    shellArgs->NotifyShellProcessDone();
}

} // namespace

namespace chip {
namespace Shell {

void Engine::RunMainLoop()
{
    int ret = chip::Shell::streamer_init(chip::Shell::streamer_get());
    assert(ret == 0);

    Engine::Root().RegisterDefaultCommands();
    while (true)
    {
        const char * prompt = LOG_COLOR_I "> " LOG_RESET_COLOR;
        char * line         = linenoise(prompt);
        printf("\r\n");
        if (line == NULL || strlen(line) == 0)
        {
            if (line)
            {
                linenoiseFree(line);
            }
            continue;
        }
        ShellLineArgs shellArgs(line, xTaskGetCurrentTaskHandle());
        linenoiseHistoryAdd(line);
        PlatformMgr().ScheduleWork(ProcessShellLine, reinterpret_cast<intptr_t>(&shellArgs));
        shellArgs.WaitShellProcessDone();
        linenoiseFree(line);
    }
}

} // namespace Shell
} // namespace chip
