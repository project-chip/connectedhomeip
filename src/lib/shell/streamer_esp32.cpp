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
    size_t len = strlen(buf);

    // If there is a trailing space, remove it for completion lookup
    bool trailing_space = false;
    if (buf[len - 1] == ' ')
    {
        trailing_space = true;
        memset((char *) &buf[len - 1], 0, 1);
        len -= 1;
    }

    const char * matter_prefix = "matter";

    // the commands are prefixed with "matter"
    if (len >= 6 && strncmp(buf, matter_prefix, 6) == 0)
    {
        // remove "matter " prefix for completion lookup as the command
        // registeration in general are done without "matter" prefix
        char * line = new char[len - strlen(matter_prefix) + trailing_space + 1];
        strcpy((char *) line, &buf[7]);

        cmd_completion_context context = cmd_completion_context(line);
        CHIP_ERROR err_code            = Engine::GetCommandCompletions(&context);
        if (err_code == CHIP_NO_ERROR)
        {
            for (size_t i = 0; i < context.cmdc; i++)
            {

                // example: "matter" + " " + "config" + " " + "productid" + " " + '\0'
                char * cmd_completion =
                    new char[strlen(matter_prefix) + 1 + strlen(context.ret_prefix) + 1 + strlen(context.cmdv[i]->cmd_name) + 1];

                // Write "matter "
                size_t pos = 0;
                strncpy(&cmd_completion[pos], matter_prefix, strlen(matter_prefix) + 1);
                pos += strlen(matter_prefix);
                strncpy(&cmd_completion[pos], " ", 2);
                pos += 1;

                // Write "config "
                strncpy(&cmd_completion[pos], context.ret_prefix, strlen(context.ret_prefix) + 1);
                pos += strlen(context.ret_prefix);
                if (strcmp(context.ret_prefix, "") != 0)
                {
                    strncpy(&cmd_completion[pos], " ", 2);
                    pos += 1;
                }

                // write "productid"
                strncpy(&cmd_completion[pos], context.cmdv[i]->cmd_name, strlen(context.cmdv[i]->cmd_name) + 1);
                pos += strlen(context.cmdv[i]->cmd_name);

                linenoiseAddCompletion(lc, cmd_completion);
                free((char *) cmd_completion);
            }
        }
        free((char *) line);
    }
    else
    {
        // When the command isn't prefixed with "matter", call the default esp
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
    linenoiseSetCompletionCallback(&get_command_completion);

    if (linenoiseProbe())
    {
        // Set if terminal does not recognize escape sequences.
        linenoiseSetDumbMode(1);
    }

    esp_console_cmd_t command = { .command = "matter", .help = "Matter utilities", .func = chip_command_handler };
    ESP_ERROR_CHECK(esp_console_cmd_register(&command));
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
