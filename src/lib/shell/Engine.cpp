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

#include <algorithm>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <map>
#include <stdbool.h>
#include <stdint.h>
#include <vector>

using namespace chip::Logging;

namespace chip {
namespace Shell {

Engine Engine::theEngineRoot;
shell_map_t Engine::theShellMap;

int Engine::Init()
{
    // Initialize the default streamer that was linked.
    int error = streamer_init(streamer_get());

    Engine::Root().RegisterDefaultCommands();

    return error;
}

void Engine::ForEachCommand(shell_command_iterator_t * on_command, void * arg)
{
    for (auto * _command : _commandSet)
    {
        if (on_command(_command, arg) != CHIP_NO_ERROR)
        {
            return;
        }
    }
}

void Engine::RegisterCommands(shell_command_t * command_set, unsigned count, const char * prefix)
{
    if (this == &Engine::Root() || prefix == nullptr)
    {
        prefix = "";
    }

    if (_commandSet.size() + count > CHIP_SHELL_MAX_MODULES)
    {
        ChipLogError(Shell, "Max number of commands reached\n");
        assert(0);
    }

    for (unsigned i = 0; i < count; i++)
    {
        _commandSet.push_back(command_set + i);
    }

    Engine::AddToShellMap(prefix, this);
}

void Engine::AddToShellMap(char const * prefix, Engine * shell)
{
    shell_map_t::iterator it = Engine::ShellMap().find(prefix);
    if (it == Engine::ShellMap().end())
    {
        Engine::ShellMap().insert(shell_map_t::value_type(prefix, { shell }));
    }
    else if (std::find(it->second.begin(), it->second.end(), shell) == it->second.end())
    {
        it->second.push_back(shell);
    }
}

std::vector<shell_command_t *> Engine::GetCommandSuggestions(const char * prefix)
{
    if (prefix == nullptr)
    {
        prefix = "";
    }

    shell_map_t::iterator engine_map = Engine::ShellMap().find(prefix);
    std::vector<shell_command_t *> next_commands;

    if (engine_map == Engine::ShellMap().end())
    {
        return next_commands;
    }

    std::vector<Engine *> engines = engine_map->second;

    for (auto engine : engines)
    {
        std::vector<shell_command_t *> engine_commands = engine->GetCommandSet();

        for (auto engine_command : engine_commands)
        {
            next_commands.push_back(engine_command);
        }
    }
    return next_commands;
}

CHIP_ERROR Engine::ExecCommand(int argc, char * argv[])
{
    CHIP_ERROR retval = CHIP_ERROR_INVALID_ARGUMENT;
    VerifyOrReturnError(argc > 0, retval);
    // Find the command
    for (unsigned i = 0; i < _commandSet.size(); i++)
    {
        if (strcmp(argv[0], _commandSet[i]->cmd_name) == 0)
        {
            // Execute the command!
            retval = _commandSet[i]->cmd_func(argc - 1, argv + 1);
            break;
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
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    RegisterDnsCommands();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    RegisterOtaCommands();
#endif
}

} // namespace Shell
} // namespace chip
