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

#include <shell/shell.h>

#include <core/CHIPCore.h>
#include <support/Base64.h>
#include <support/CHIPArgParser.hpp>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::Logging;
using namespace chip::ArgParser;

int cmd_rand(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "%d\n\r", GetRandU8());
    return 0;
}

int cmd_log(int argc, char ** argv)
{
    int i = 0;
    while (i < argc)
    {
        ChipLogProgress(chipTool, "%s", argv[i++]);
    }
    return 0;
}

void cmd_base64_help()
{
    streamer_printf(streamer_get(),
                    "Usage: base64 <command> <data>\n\r\n\r"
                    "Supported commands:\n\r"
                    "    encode    Converts hex string to base64 string\n\r"
                    "    decode    Converts base64 string to hex string\n\r");
}

int cmd_base64(int argc, char ** argv)
{
    CHIP_ERROR error             = CHIP_NO_ERROR;
    const struct streamer * sout = streamer_get();
    char base64[256];
    uint8_t binary[256];
    uint32_t binarySize, base64Size;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (strcmp(argv[0], "encode") == 0)
    {
        VerifyOrExit(argc > 1, error = CHIP_ERROR_INVALID_ARGUMENT);
        ParseHexString(argv[1], strlen(argv[1]), binary, sizeof(binary), binarySize);
        base64Size = Base64Encode(binary, binarySize, base64);
        streamer_printf(sout, "%.*s\n\r", base64Size, base64);
    }
    else if (strcmp(argv[0], "decode") == 0)
    {
        VerifyOrExit(argc > 1, error = CHIP_ERROR_INVALID_ARGUMENT);
        binarySize = Base64Decode(argv[1], sizeof(argv[1]), binary);
        streamer_print_hex(sout, binary, binarySize);
        streamer_printf(sout, "\n\r");
    }
    else
    {
        error = CHIP_ERROR_INVALID_ARGUMENT;
    }

exit:
    if (error)
    {
        cmd_base64_help();
    }
    return error;
}

static const struct shell_cmd cmds_support[] = {
    { &cmd_rand, "rand", "Random number utilities" },
    { &cmd_log, "log", "Logging utilities" },
    { &cmd_base64, "base64", "Base64 encode / decode utilities" },
};

int main(void)
{
    shell_register(cmds_support, ARRAY_SIZE(cmds_support));
    shell_task(NULL);
}
