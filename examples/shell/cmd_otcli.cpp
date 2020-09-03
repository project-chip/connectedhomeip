/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <core/CHIPCore.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CHIP_ENABLE_OPENTHREAD

#include <stdio.h>

#include <platform/ThreadStackManager.h>
#include <shell/shell.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>

#if CHIP_TARGET_STYLE_EMBEDDED
#include <openthread/cli.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/thread.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace chip;
using namespace chip::Shell;
using namespace chip::Platform;
using namespace chip::DeviceLayer;
using namespace chip::Logging;
using namespace chip::ArgParser;

static chip::Shell::Shell sShellOtcliSubcommands;

int cmd_otcli_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_otcli_help(int argc, char ** argv)
{
    sShellOtcliSubcommands.ForEachCommand(cmd_otcli_help_iterator, NULL);
    return 0;
}

#if CHIP_TARGET_STYLE_EMBEDDED

int cmd_otcli_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

// From OT CLI internal lib, kMaxLineLength = 128
#define kMaxLineLength 128
    char buff[kMaxLineLength] = { 0 };
    char * buff_ptr           = buff;
    int i                     = 0;

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

    otCliConsoleInputLine(buff, buff_ptr - buff);
exit:
    return error;
}

#elif CHIP_TARGET_STYLE_UNIX

int cmd_otcli_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    int pid;
    uid_t euid         = geteuid();
    char ctl_command[] = "/usr/local/sbin/ot-ctl";

    // Must run as sudo.
    if (euid != 0)
    {
        streamer_printf(streamer_get(), "Error otcli: requires running chip-shell as sudo\n\r");
        error = CHIP_ERROR_INCORRECT_STATE;
        ExitNow();
    }

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    // Fork and execute the command.
    pid = fork();
    VerifyOrExit(pid != -1, error = CHIP_ERROR_INCORRECT_STATE);

    if (pid == 0)
    {
        // Child process to execute the command with provided arguments
        --argv; // Restore access to entry [0] containing the command;
        argv[0] = ctl_command;
        if (execvp(ctl_command, argv) < 0)
        {
            streamer_printf(streamer_get(), "Error exec %s: %s\n", ctl_command, strerror(errno));
        }
        exit(errno);
    }
    else
    {
        // Parent process to wait on child.
        int status;
        wait(&status);
        error = (status) ? CHIP_ERROR_INCORRECT_STATE : CHIP_NO_ERROR;
    }

exit:
    return error;
}

#endif // CHIP_TARGET_STYLE_UNIX

static const shell_command_t cmds_otcli_root = { &cmd_otcli_dispatch, "otcli", "Dispatch OpenThread CLI command" };

#if CHIP_TARGET_STYLE_EMBEDDED
static int OnOtCliInitialized(const char * aBuf, uint16_t aBufLength, void * aContext)
{
    ChipLogProgress(chipTool, "%s", aBuf);
    return 0;
}
#endif

#endif // CHIP_ENABLE_OPENTHREAD

void cmd_otcli_init(void)
{
#if CHIP_ENABLE_OPENTHREAD
#if CHIP_TARGET_STYLE_EMBEDDED
    otCliConsoleInit(GetOtInstance(), &OnOtCliInitialized, NULL);
#endif

    // Register the root otcli command with the top-level shell.
    shell_register(&cmds_otcli_root, 1);
#endif // CHIP_ENABLE_OPENTHREAD
}
