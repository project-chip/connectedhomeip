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

#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>

#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"
#include <fcntl.h>
#include <lib/core/CHIPError.h>
#include <stdio.h>
#include <string.h>

namespace chip {
namespace Shell {

static int chip_command_handler(int argc, char ** argv)
{
    CHIP_ERROR err;
    if (argc > 0)
    {
        err = Engine::Root().ExecCommand(argc - 1, argv + 1);
    }
    else
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }
    return static_cast<int>(err.AsInteger());
}

void get_command_completion(const char * buf, linenoiseCompletions * lc)
{
    size_t len                 = strlen(buf);
    const char * matter_prefix = "matter ";

    if (len > 6 && strncmp(buf, matter_prefix, 6) == 0)
    {
        // remove "matter " prefix for suggestion lookup
        buf = &buf[7];
        len = len - 7;

        int last_space_idx = -1;

        for (int i = len - 1; i > -1; i--)
        {
            if (buf[i] == ' ')
            {
                last_space_idx = i;
                break;
            }
        }

        // Get the last token of user input which may be an incomplete command name
        // for comparing with completion candidates
        char * incomplete_cmd = new char[len - last_space_idx];

        // In case of cursor at the last space, there is no incomplete command to match,
        // len = 0, last_space_idx = -1 and strylcpy here will copy only 1 char from the
        // cursor position, that is the null terminator.
        strlcpy(incomplete_cmd, &buf[last_space_idx + 1], len - last_space_idx);

        char * prefix = new char[last_space_idx + 2];

        if (last_space_idx == -1)
        {
            // For root commands (when there is no space after the "matter " prefix
            // e.g. "matter config", "matter device"), look up with empty string ""
            strlcpy(prefix, "", 1);
        }
        else
        {
            // Get the user input until (not included) the last space in buf,
            // this will be use to look up completion candidates
            strlcpy(prefix, buf, last_space_idx + 1);
        }

        std::vector<shell_command_t *> cmdSuggestions = Engine::GetCommandSuggestions(prefix);

        for (auto suggestion : cmdSuggestions)
        {
            // When there is an incomplete command to match, find the matched candidates and add them;
            // When there isn't an incomplete command to match, compare size is zero so strncmp
            // returns 0 for all candidates, thus add all candidates.
            if (strncmp(incomplete_cmd, suggestion->cmd_name, strlen(incomplete_cmd)) == 0)
            {
                std::string cmd_completion = matter_prefix;
                if (strcmp(prefix, "") != 0)
                {
                    cmd_completion += prefix;
                    cmd_completion += " ";
                }
                cmd_completion += suggestion->cmd_name;
                linenoiseAddCompletion(lc, cmd_completion.c_str());
            }
        }
        delete incomplete_cmd;
        delete prefix;
    }
    else
    {
        // When the command isn't prefixed with "matter ", call the default esp
        // get completion function to get the non-matter registered commands
        return esp_console_get_completion(buf, lc);
    }
}

int streamer_esp32_init(streamer_t * streamer)
{
    fflush(stdout);
    fsync(fileno(stdout));
    setvbuf(stdin, NULL, _IONBF, 0);
    esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);
    if (!uart_is_driver_installed(CONFIG_ESP_CONSOLE_UART_NUM))
    {
        ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0));
    }
    uart_config_t uart_config = {
        .baud_rate           = 115200,
        .data_bits           = UART_DATA_8_BITS,
        .parity              = UART_PARITY_DISABLE,
        .stop_bits           = UART_STOP_BITS_1,
        .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk          = UART_SCLK_APB,
    };
    ESP_ERROR_CHECK(uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));
    esp_vfs_dev_uart_use_driver(0);
    esp_console_config_t console_config = {
        .max_cmdline_length = 256,
        .max_cmdline_args   = 32,
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));
    linenoiseSetMultiLine(1);
    linenoiseHistorySetMaxLen(100);

    if (linenoiseProbe())
    {
        // Set if terminal does not recognize escape sequences.
        linenoiseSetDumbMode(1);
    }

    esp_console_cmd_t matter_command = { .command = "matter", .help = "Matter utilities", .func = chip_command_handler };
    ESP_ERROR_CHECK(esp_console_cmd_register(&matter_command));
    linenoiseSetCompletionCallback(&get_command_completion);
    return 0;
}

ssize_t streamer_esp32_read(streamer_t * streamer, char * buf, size_t len)
{
    return 0;
}

ssize_t streamer_esp32_write(streamer_t * streamer, const char * buf, size_t len)
{
    return uart_write_bytes(CONFIG_ESP_CONSOLE_UART_NUM, buf, len);
}

static streamer_t streamer_stdio = {
    .init_cb  = streamer_esp32_init,
    .read_cb  = streamer_esp32_read,
    .write_cb = streamer_esp32_write,
};

streamer_t * streamer_get()
{
    return &streamer_stdio;
}

} // namespace Shell
} // namespace chip
