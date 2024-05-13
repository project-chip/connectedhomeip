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
#include "sdkconfig.h"
#if CONFIG_OPENTHREAD_CLI
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "openthread_cli_register.h"
#include "platform/ESP32/OpenthreadLauncher.h"
#include <lib/shell/Engine.h>
#include <memory>

#define CLI_INPUT_BUFF_LENGTH 256u
namespace chip {

static CHIP_ERROR OtcliHandler(int argc, char * argv[])
{
    /* the beginning of command "matter otcli" has already been removed */
    std::unique_ptr<char[]> cli_str(new char[CLI_INPUT_BUFF_LENGTH]);
    memset(cli_str.get(), 0, CLI_INPUT_BUFF_LENGTH);
    uint8_t len = 0;
    for (size_t i = 0; i < (size_t) argc; ++i)
    {
        len = len + strlen(argv[i]) + 1;
        if (len > CLI_INPUT_BUFF_LENGTH - 1)
        {
            return CHIP_ERROR_INTERNAL;
        }
        strcat(cli_str.get(), argv[i]);
        strcat(cli_str.get(), " ");
    }

    if (cli_transmit_task_post(std::move(cli_str)) != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

void RegisterOpenThreadCliCommands()
{
    static const chip::Shell::shell_command_t cmds[] = {
        {
            .cmd_func = OtcliHandler,
            .cmd_name = "otcli",
            .cmd_help = "OpenThread cli commands",
        },
    };
    int cmds_num = sizeof(cmds) / sizeof(chip::Shell::shell_command_t);
    chip::Shell::Engine::Root().RegisterCommands(cmds, cmds_num);
}
} // namespace chip
#endif // CONFIG_OPENTHREAD_CLI
