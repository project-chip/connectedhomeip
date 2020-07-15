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

#if CHIP_ENABLE_OPENTHREAD

#include <platform/CHIPDeviceLayer.h>
#include <shell/shell.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>

#include <openthread/cli.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/thread.h>

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

int cmd_otcli_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

// From OT CLI internal lib, kMaxLineLength = 128
#define kMaxLineLength 128
    char buff[kMaxLineLength] = { 0 };
    char * buff_ptr           = buff;
    int i                     = 0;

    for (i = 0; i < argc; i++)
    {
        size_t arg_len = strlen(argv[i]);

        /* Make sure that the next argument won't overflow the buffer */
        if (buff_ptr + arg_len < buff + kMaxLineLength)
        {
            strncpy(buff_ptr, argv[i], arg_len);
            buff_ptr += arg_len;

            /* Make sure that there is enough buffer for a space char */
            if (buff_ptr + sizeof(char) < buff + kMaxLineLength)
            {
                strncpy(buff_ptr, " ", sizeof(char));
                buff_ptr++;
            }
        }
        else
        {
            /* Ignore the rest of the parameters since they won't fit and give it a shot */
            ChipLogProgress(chipTool, "otcli truncates some parameters.\n");
            break;
        }
    }

    otCliConsoleInputLine(buff, buff_ptr - buff);
exit:
    return error;
}

static const shell_command_t cmds_otcli_root = { &cmd_otcli_dispatch, "otcli", "Dispatch OpenThread CLI command" };

#endif // CHIP_ENABLE_OPENTHREAD

/*
static int OnOtCliInitialized(const char *aBuf, uint16_t aBufLength, void *aContext)
{
    ChipLogProgress(chipTool, "%s", aBuf);
    return 0;
}
*/

void cmd_otcli_init(void)
{
#if CHIP_ENABLE_OPENTHREAD
    /*
        // Either init here, or delay to cmd_otcli_dispatch() behind isInited static

        otCliConsoleInit(GetOtInstance(), &OnOtCliInitialized, NULL);
    */
    // Register the root `base64` command with the top-level shell.
    shell_register(&cmds_otcli_root, 1);
#endif // CHIP_ENABLE_OPENTHREAD
}
