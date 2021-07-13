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

#include <core/CHIPError.h>
#include <lib/shell/Commands.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

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

void Engine::ForEachCommand(shell_command_iterator_t * on_command, void * arg)
{
    for (unsigned i = 0; i < _commandSetCount; i++)
    {
        for (unsigned j = 0; j < _commandSetSize[i]; j++)
        {
            if (on_command(&_commandSet[i][j], arg))
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
    RegisterBase64Commands();
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
#if CHIP_DEVICE_CONFIG_ENABLE_NFC
    RegisterNFCCommands();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    RegisterDnsCommands();
#endif
}

} // namespace Shell
} // namespace chip
