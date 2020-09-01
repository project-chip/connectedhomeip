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

#include <lib/shell/shell.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/RandUtils.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::Logging;
using namespace chip::ArgParser;

chip::Shell::Shell theShellBase64;

int cmd_base64_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_base64_help(int argc, char ** argv)
{
    theShellBase64.ForEachCommand(cmd_base64_help_iterator, NULL);
    return 0;
}

int cmd_base64_decode(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint32_t binarySize;
    uint8_t binary[256];

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    binarySize = Base64Decode(argv[0], strlen(argv[0]), binary);
    streamer_print_hex(sout, binary, binarySize);
    streamer_printf(sout, "\n\r");

exit:
    return error;
}

int cmd_base64_encode(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    char base64[256];
    uint8_t binary[256];
    uint32_t binarySize, base64Size;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    ParseHexString(argv[0], strlen(argv[0]), binary, sizeof(binary), binarySize);
    base64Size = Base64Encode(binary, binarySize, base64);
    streamer_printf(sout, "%.*s\n\r", base64Size, base64);

exit:
    return error;
}

int cmd_base64_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = theShellBase64.ExecCommand(argc, argv);

exit:
    return error;
}

static const shell_command_t cmds_base64_root = { &cmd_base64_dispatch, "base64", "Base64 encode / decode utilities" };

/// Subcommands for root command: `base64 <subcommand>`
static const shell_command_t cmds_base64[] = {
    { &cmd_base64_help, "help", "Usage: base64 <subcommand>" },
    { &cmd_base64_encode, "encode", "Encode a hex sting as base64. Usage: base64 encode <hex_string>" },
    { &cmd_base64_decode, "decode", "Decode a base64 sting as hex. Usage: base64 decode <base64_string>" },
};

void cmd_base64_init(void)
{
    // Register `base64` subcommands with the local shell dispatcher.
    theShellBase64.RegisterCommands(cmds_base64, ArraySize(cmds_base64));

    // Register the root `base64` command with the top-level shell.
    shell_register(&cmds_base64_root, 1);
}
