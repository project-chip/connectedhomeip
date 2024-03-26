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
#include "sl_component_catalog.h"
#include <ChipShellCollection.h>
#include <FreeRTOS.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Engine.h>
#include <task.h>
#ifdef SL_CATALOG_CLI_PRESENT
#include "sl_cli.h"
#include "sl_cli_config.h"
#include "sli_cli_io.h"
static char sSilabsTXCLI[SL_CLI_OUTPUT_BUFFER_SIZE];
static constexpr uint16_t sSilabsTXCLILenght = SL_CLI_OUTPUT_BUFFER_SIZE;
#endif

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
    const TickType_t xDelay = pdMS_TO_TICKS(1000 * seconds);
    vTaskDelay(xDelay);
    return 0;
}

namespace chip {

void NotifyShellProcess()
{
    xTaskNotifyGive(shellTaskHandle);
}

void NotifyShellProcessFromISR()
{
    BaseType_t yieldRequired = pdFALSE;
    if (shellTaskHandle != NULL)
    {
        vTaskNotifyGiveFromISR(shellTaskHandle, &yieldRequired);
    }
    portYIELD_FROM_ISR(yieldRequired);
}

void WaitForShellActivity()
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

#ifdef SL_CATALOG_CLI_PRESENT

CHIP_ERROR cmd_silabs_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    char buff[SL_CLI_INPUT_BUFFER_SIZE] = { 0 };
    char * buff_ptr                     = buff;
    int i                               = 0;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    for (i = 0; i < argc; i++)
    {
        size_t arg_len = strlen(argv[i]);

        /* Make sure that the next argument won't overflow the buffer */
        VerifyOrExit(buff_ptr + arg_len < buff + kMaxLineLength, error = CHIP_ERROR_BUFFER_TOO_SMALL);

        strncpy(buff_ptr, argv[i], arg_len);
        buff_ptr += arg_len;

        /* Make sure that there is enough buffer for a space char */
        if (buff_ptr + sizeof(char) < buff + kMaxLineLength)
        {
            strncpy(buff_ptr, " ", sizeof(char));
            buff_ptr++;
        }
    }
    buff_ptr = 0;
    sl_cli_handle_input(sl_cli_default_handle, buff);
exit:
    return error;
}

static const Shell::shell_command_t cmds_silabs_root = { &cmd_silabs_dispatch, "silabs", "Dispatch Silicon Labs CLI command" };

void cmd_silabs_init()
{
    // Register the root otcli command with the top-level shell.
    Engine::Root().RegisterCommands(&cmds_silabs_root, 1);
}

extern "C" int sli_cli_io_printf(const char * format, ...)
{
    va_list argptr;
    int rval = 0;
    va_start(argptr, format);
    rval = vsnprintf(sSilabsTXCLI, sSilabsTXCLILenght, format, argptr);
    va_end(argptr);

    if (rval >= 0 && rval < sSilabsTXCLILenght)
    {
        chip::Shell::streamer_write(chip::Shell::streamer_get(), (const char *) sSilabsTXCLI, rval);
        // Line feed correction
        uint16_t size = strlen(sSilabsTXCLI);
        if (size < 2)
        {
            return 0;
        }
        if (sSilabsTXCLI[size - 1] == '\n' && sSilabsTXCLI[size - 2] != '\r')
        {
            chip::Shell::streamer_write(chip::Shell::streamer_get(), (const char *) "\r", 2);
        }
        else if (sSilabsTXCLI[size - 1] == '\r' && sSilabsTXCLI[size - 2] != '\n')
        {
            chip::Shell::streamer_write(chip::Shell::streamer_get(), (const char *) "\n", 2);
        }

        return 0;
    }
    return SL_STATUS_FAIL;
}

#endif // SL_CATALOG_CLI_PRESENT

void startShellTask()
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
