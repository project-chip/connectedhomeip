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

#include <lib/core/CHIPCore.h>

#include <ChipShellCollection.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CHIP_ENABLE_OPENTHREAD

#include <stdio.h>

#include <lib/shell/Engine.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/ThreadStackManager.h>

#if CHIP_TARGET_STYLE_EMBEDDED
#include <openthread/cli.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/thread.h>
#if OPENTHREAD_API_VERSION >= 85
#if !CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
#ifndef SHELL_OTCLI_TX_BUFFER_SIZE
#define SHELL_OTCLI_TX_BUFFER_SIZE 1024
#endif
static char sTxBuffer[SHELL_OTCLI_TX_BUFFER_SIZE];
static constexpr uint16_t sTxLength = SHELL_OTCLI_TX_BUFFER_SIZE;
#endif // !CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI)
#endif
static constexpr uint16_t kMaxLineLength = 384;
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

static chip::Shell::Engine sShellOtcliSubcommands;

CHIP_ERROR cmd_otcli_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return CHIP_NO_ERROR;
}

CHIP_ERROR cmd_otcli_help(int argc, char ** argv)
{
    sShellOtcliSubcommands.ForEachCommand(cmd_otcli_help_iterator, nullptr);
    return CHIP_NO_ERROR;
}

#if CHIP_TARGET_STYLE_EMBEDDED

CHIP_ERROR cmd_otcli_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

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
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
#if OPENTHREAD_API_VERSION >= 85
    otCliInputLine(buff);
#else
    otCliConsoleInputLine(buff, buff_ptr - buff);
#endif
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
exit:
    return error;
}

#elif CHIP_TARGET_STYLE_UNIX

CHIP_ERROR cmd_otcli_dispatch(int argc, char ** argv)
{
    int pid;
    uid_t euid         = geteuid();
    char ctl_command[] = "/usr/local/sbin/ot-ctl";

    // Must run as sudo.
    if (euid != 0)
    {
        streamer_printf(streamer_get(), "Error otcli: requires running chip-shell as sudo\n\r");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    VerifyOrReturnError(argc > 0, CHIP_ERROR_INVALID_ARGUMENT);

    // Fork and execute the command.
    pid = fork();
    VerifyOrReturnError(pid != -1, CHIP_ERROR_INCORRECT_STATE);

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
        return (status) ? CHIP_ERROR_INCORRECT_STATE : CHIP_NO_ERROR;
    }
}

#endif // CHIP_TARGET_STYLE_UNIX

static const shell_command_t cmds_otcli_root = { &cmd_otcli_dispatch, "otcli", "Dispatch OpenThread CLI command" };

#if CHIP_TARGET_STYLE_EMBEDDED
#if OPENTHREAD_API_VERSION >= 85
#if !CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
static int OnOtCliOutput(void * aContext, const char * aFormat, va_list aArguments)
{
    int rval = vsnprintf(sTxBuffer, sTxLength, aFormat, aArguments);
    VerifyOrExit(rval >= 0 && rval < sTxLength, rval = CHIP_ERROR_BUFFER_TOO_SMALL.AsInteger());
    return streamer_write(streamer_get(), (const char *) sTxBuffer, rval);
exit:
    return rval;
}
#endif // !CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
#else

static int OnOtCliOutput(const char * aBuf, uint16_t aBufLength, void * aContext)
{
    return streamer_write(streamer_get(), aBuf, aBufLength);
}
#endif
#endif

#endif // CHIP_ENABLE_OPENTHREAD

void cmd_otcli_init()
{
#if CHIP_ENABLE_OPENTHREAD
#if CHIP_TARGET_STYLE_EMBEDDED
#if !CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
#if OPENTHREAD_API_VERSION >= 85
    otCliInit(otInstanceInitSingle(), &OnOtCliOutput, NULL);
#else
    otCliConsoleInit(otInstanceInitSingle(), &OnOtCliOutput, NULL);
#endif // OPENTHREAD_API_VERSION >= 85
#endif // !CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
#endif // CHIP_TARGET_STYLE_EMBEDDED

    // Register the root otcli command with the top-level shell.
    Engine::Root().RegisterCommands(&cmds_otcli_root, 1);
#endif // CHIP_ENABLE_OPENTHREAD
}
