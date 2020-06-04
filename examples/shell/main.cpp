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
using namespace chip::Logging;
using namespace chip::ArgParser;

// extern "C" void __cxa_pure_virtual() {    volatile int a = 1; }

static void OutputBytes(const uint8_t * aBytes, uint8_t aLength)
{
    for (int i = 0; i < aLength; i++)
    {
        streamer_printf(streamer_get(), "%02x", aBytes[i]);
    }
}

int cmd_rand(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "%d\n\r", GetRandU8());
    return 0;
}

int cmd_log(int argc, char ** argv)
{
    int i = 1;
    while (i < argc)
    {
        ChipLogProgress(chipTool, "%s", argv[i++]);
    }
    return 0;
}

int cmd_base64(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    char base64[256];
    uint8_t binary[256];
    uint32_t binarySize;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (strcmp(argv[0], "encode") == 0)
    {
        VerifyOrExit(argc > 1, error = CHIP_ERROR_INVALID_ARGUMENT);
        ParseHexString(argv[1], strlen(argv[1]), binary, sizeof(binary), binarySize);
        Base64Encode(binary, binarySize, base64);
        streamer_printf(streamer_get(), "%s\n\r", base64);
    }
    else if (strcmp(argv[0], "decode") == 0)
    {
        VerifyOrExit(argc > 1, error = CHIP_ERROR_INVALID_ARGUMENT);
        binarySize = Base64Decode(argv[1], sizeof(argv[1]), binary);
        OutputBytes(binary, binarySize);
        streamer_printf(streamer_get(), "\n\r");
    }
    else
    {
        error = CHIP_ERROR_INVALID_ARGUMENT;
    }

exit:
    return error;
}

static const struct chip_shell_cmd cmds_support[] = {
    { &cmd_rand, "rand", "Random number utilities" },
    { &cmd_log, "log", "Logging utilities" },
    { &cmd_base64, "base64", "Base64 encode / decode utilities" },
};

int main(void)
{
    chip_shell_register(cmds_support, ARRAY_SIZE(cmds_support));
    chip_shell_task(NULL);
}
