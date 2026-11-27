/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include "LowPowerCLI.h"
#include "LowPower.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>

namespace chip::NXP::App {

namespace {

using namespace chip::Shell;

constexpr uint32_t kDefaultDurationMs = std::numeric_limits<uint32_t>::max();

void PrintLowPowerUsage()
{
    streamer_printf(streamer_get(), "Usage:\r\n"
                                    "  lowpower help\r\n"
                                    "  lowpower enable <mode> [duration_ms]\r\n"
                                    "Modes depend on the platform backend. RW61x supports 1..3:\r\n"
                                    "  1: Sleep, 2: DeepSleep, 3: PowerDown\r\n"
                                    "Duration is in milliseconds (default: 4294967295 ms).\r\n"
                                    "Minimum duration: one OS tick.\r\n"
                                    "Example: lowpower enable 2 5000\r\n");
}

CHIP_ERROR LowPowerCommand(int argc, char * argv[])
{
    if (argc == 1 && std::strcmp(argv[0], "help") == 0)
    {
        PrintLowPowerUsage();
        return CHIP_NO_ERROR;
    }

    if ((argc != 2 && argc != 3) || std::strcmp(argv[0], "enable") != 0)
    {
        PrintLowPowerUsage();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint32_t mode       = 0;
    uint32_t durationMs = kDefaultDurationMs;
    if (argv[1] == nullptr || argv[1][0] < '0' || argv[1][0] > '9' ||
        !ArgParser::ParseInt(argv[1], mode, 10) || mode > std::numeric_limits<uint8_t>::max() ||
        (argc == 3 &&
         (argv[2] == nullptr || argv[2][0] < '0' || argv[2][0] > '9' ||
          !ArgParser::ParseInt(argv[2], durationMs, 10))))
    {
        PrintLowPowerUsage();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err = LowPower::StartTimedMode(static_cast<uint8_t>(mode), durationMs);

    if (err == CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Constraint is PM%u. Ready to enter low power loop for %u ms\r\n",
                        static_cast<unsigned int>(mode), static_cast<unsigned int>(durationMs));
        return CHIP_NO_ERROR;
    }

    if (err == CHIP_ERROR_INCORRECT_STATE)
    {
        streamer_printf(streamer_get(), "Matter PM command: low power loop already running, wait for \"Exit low power loop.\"\r\n");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (err == CHIP_ERROR_INVALID_ARGUMENT)
    {
        PrintLowPowerUsage();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
}

} // namespace

void cmd_lowpower_init()
{
    static shell_command_t kCommands[] = {
        { &LowPowerCommand, "lowpower", "Low power control. Usage: lowpower <enable|help> [args...]" },
    };
    Shell::Engine::Root().RegisterCommands(kCommands, MATTER_ARRAY_SIZE(kCommands));
}

} // namespace chip::NXP::App
