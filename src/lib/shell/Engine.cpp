/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Google LLC
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

/**
 *    @file
 *      Source implementation for a generic shell API for CHIP examples.
 */

#include <lib/shell/Engine.h>

#include <lib/core/CHIPError.h>
#include <lib/shell/Commands.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

using namespace chip::Logging;

namespace chip {
namespace Shell {

Engine Engine::theEngineRoot;

int Engine::Init()
{
    // Initialize the default streamer that was linked.
    int error = streamer_init(streamer_get());

    Engine::Root().RegisterDefaultCommands();

    return error;
}

void Engine::ForEachCommand(shell_command_iterator_t * on_command, void * arg)
{
    for (unsigned i = 0; i < _commandSetCount; i++)
    {
        for (unsigned j = 0; j < _commandSetSize[i]; j++)
        {
            if (on_command(&_commandSet[i][j], arg) != CHIP_NO_ERROR)
            {
                return;
            }
        }
    }
}

void Engine::RegisterCommands(shell_command_t * command_set, unsigned count)
{
    if (_commandSetCount >= CHIP_SHELL_MAX_MODULES)
    {
        ChipLogError(Shell, "Max number of modules reached\n");
        assert(0);
    }

    _commandSet[_commandSetCount]     = command_set;
    _commandSetSize[_commandSetCount] = count;
    ++_commandSetCount;
}

CHIP_ERROR Engine::ExecCommand(int argc, char * argv[])
{
    CHIP_ERROR retval = CHIP_ERROR_INVALID_ARGUMENT;

    VerifyOrReturnError(argc > 0, retval);
    VerifyOrReturnError(nullptr != argv, retval);

    // Find the command
    for (unsigned i = 0; i < _commandSetCount; i++)
    {
        for (unsigned j = 0; j < _commandSetSize[i]; j++)
        {
            if (strcmp(argv[0], _commandSet[i][j].cmd_name) == 0)
            {
                // Execute the command!
                retval = _commandSet[i][j].cmd_func(argc - 1, argv + 1);
                break;
            }
        }
    }

    return retval;
}

void Engine::RegisterDefaultCommands()
{
    RegisterMetaCommands();
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    RegisterBLECommands();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    RegisterWiFiCommands();
#endif
#if CONFIG_DEVICE_LAYER
    RegisterConfigCommands();
    RegisterDeviceCommands();
    RegisterOnboardingCodesCommands();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_NFC_ONBOARDING_PAYLOAD
    RegisterNFCCommands();
#endif
    RegisterDnsCommands();
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    RegisterOtaCommands();
#endif
#if CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS
    RegisterStatCommands();
#endif
}

} // namespace Shell
} // namespace chip
