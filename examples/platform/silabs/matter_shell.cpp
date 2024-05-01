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
#include <cmsis_os2.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Engine.h>
#include <sl_cmsis_os2_common.h>
#ifdef SL_CATALOG_CLI_PRESENT
#include "sl_cli.h"
#include "sl_cli_config.h"
#include "sli_cli_io.h"
#endif

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

#ifdef SL_CATALOG_CLI_PRESENT

CHIP_ERROR CmdSilabsDispatch(int argc, char ** argv)
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
        VerifyOrExit(buff_ptr + arg_len < buff + SL_CLI_INPUT_BUFFER_SIZE, error = CHIP_ERROR_BUFFER_TOO_SMALL);

        strncpy(buff_ptr, argv[i], arg_len);
        buff_ptr += arg_len;

        /* Make sure that there is enough buffer for a space char */
        if (buff_ptr + sizeof(char) < buff + SL_CLI_INPUT_BUFFER_SIZE)
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

static const Shell::shell_command_t cmds_silabs_root = { &CmdSilabsDispatch, "silabs", "Dispatch Silicon Labs CLI command" };

void cmdSilabsInit()
{
    // Register the root otcli command with the top-level shell.
    Engine::Root().RegisterCommands(&cmds_silabs_root, 1);
}

#endif // SL_CATALOG_CLI_PRESENT

void startShellTask()
{
    int status = chip::Shell::Engine::Root().Init();
    assert(status == 0);

    // For now also register commands from shell_common (shell app).
    // TODO move at least OTCLI to default commands in lib/shell/commands
    cmd_misc_init();
#ifndef SL_WIFI
    cmd_otcli_init();
#endif

#ifdef SL_CATALOG_CLI_PRESENT
    cmdSilabsInit();
#endif

    shellTaskHandle = osThreadNew(MatterShellTask, nullptr, &kShellTaskAttr);
    VerifyOrDie(shellTaskHandle);
}

} // namespace chip
