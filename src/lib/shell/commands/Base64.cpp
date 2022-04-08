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

#include <cstdint>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>

chip::Shell::Engine sShellBase64Commands;

namespace chip {
namespace Shell {

static CHIP_ERROR Base64HelpHandler(int argc, char ** argv)
{
    sShellBase64Commands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR Base64DecodeHandler(int argc, char ** argv)
{
    streamer_t * sout = streamer_get();
    uint32_t binarySize;
    uint8_t binary[256];

    VerifyOrReturnError(argc > 0, CHIP_ERROR_INVALID_ARGUMENT);
    binarySize = Base64Decode(argv[0], strlen(argv[0]), binary);
    VerifyOrReturnError(binarySize != UINT16_MAX, CHIP_ERROR_INVALID_ARGUMENT);
    streamer_print_hex(sout, binary, binarySize);
    streamer_printf(sout, "\r\n");
    return CHIP_NO_ERROR;
}

static CHIP_ERROR Base64EncodeHandler(int argc, char ** argv)
{
    streamer_t * sout = streamer_get();
    char base64[256];
    uint8_t binary[256];
    uint32_t binarySize, base64Size;

    VerifyOrReturnError(argc > 0, CHIP_ERROR_INVALID_ARGUMENT);
    ArgParser::ParseHexString(argv[0], strlen(argv[0]), binary, sizeof(binary), binarySize);
    base64Size = Base64Encode(binary, binarySize, base64);
    streamer_printf(sout, "%.*s\r\n", base64Size, base64);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR Base64Dispatch(int argc, char ** argv)
{
    if (argc == 0)
    {
        return Base64HelpHandler(argc, argv);
    }
    return sShellBase64Commands.ExecCommand(argc, argv);
}

void RegisterBase64Commands()
{
    /// Subcommands for root command: `base64 <subcommand>`
    static const shell_command_t sBase64SubCommands[] = {
        { &Base64HelpHandler, "help", "Usage: base64 <subcommand>" },
        { &Base64EncodeHandler, "encode", "Encode a hex sting as base64. Usage: base64 encode <hex_string>" },
        { &Base64DecodeHandler, "decode", "Decode a base64 sting as hex. Usage: base64 decode <base64_string>" },
    };

    static const shell_command_t sBase64Command = { &Base64Dispatch, "base64", "Base64 encode / decode utilities" };

    // Register `base64` subcommands with the local shell dispatcher.
    sShellBase64Commands.RegisterCommands(sBase64SubCommands, ArraySize(sBase64SubCommands));

    // Register the root `base64` command with the top-level shell.
    Engine::Root().RegisterCommands(&sBase64Command, 1);
}

} // namespace Shell
} // namespace chip
