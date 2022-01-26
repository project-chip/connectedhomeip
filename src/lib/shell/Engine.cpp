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

#include <lib/core/CHIPError.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

using namespace chip::Logging;

namespace chip {
namespace Shell {

Engine Engine::theEngineRoot;
shell_map * Engine::theShellMapListHead = NULL;

int Engine::Init()
{
    // Initialize the default streamer that was linked.
    int error = streamer_init(streamer_get());

    Engine::Root().RegisterDefaultCommands();

    return error;
}

void Engine::ForEachCommand(shell_command_iterator_t * on_command, void * arg)
{
    for (unsigned i = 0; i < this->_commandCount; i++)
    {
        if (on_command(this->_commands[i], arg) != CHIP_NO_ERROR)
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

    if (_commandCount + count > CHIP_SHELL_MAX_MODULES)
    {
        ChipLogError(Shell, "Max number of commands registered to this shell");
        assert(0);
    }

    for (unsigned i = 0; i < count; i++)
    {
        _commands[_commandCount + i] = &command_set[i];
    }
    _commandCount += count;

    Engine::InsertShellMap(prefix, this);
}

void Engine::InsertShellMap(char const * prefix, Engine * shell)
{
    shell_map_t * map = Engine::theShellMapListHead;
    while (map != NULL)
    {
        if (strcmp(prefix, map->prefix) == 0)
        {
            for (size_t i = 0; i < map->enginec; i++)
            {
                if (map->enginev[i] == shell)
                {
                    return;
                }
            }
            if (map->enginec == sizeof(map->enginev))
            {
                ChipLogError(Shell, "Max number of shells registered under this prefix");
                assert(0);
            }
            map->enginev[map->enginec] = shell;
            map->enginec++;
            return;
        }
        map = map->next;
    }
    shell_map_t * new_map       = new shell_map_t;
    new_map->prefix             = prefix;
    new_map->enginev[0]         = shell;
    new_map->enginec            = 1;
    new_map->next               = Engine::theShellMapListHead;
    Engine::theShellMapListHead = new_map;
}

// CHIP_ERROR process_completion(shell_command_t * cmd, void * arg)
// {
//     cmd_completion_context * ctx = (cmd_completion_context *) ((void **) arg)[0];
//     char * _incomplete_cmd       = (char *) ((void **) arg)[1];
//     // For end nodes like "ble adv", need to avoid duplicate returns.
//     // Return for prefix="ble adv" cmd=""; reject for prefix="ble" cmd="adv"
//     if ((strcmp(cmd->cmd_name, _incomplete_cmd) != 0 && strncmp(cmd->cmd_name, _incomplete_cmd, strlen(_incomplete_cmd)) == 0) ||
//         strcmp(_incomplete_cmd, "") == 0)
//     {
//         ctx->cmdc++;
//         ctx->cmdv[ctx->cmdc - 1] = cmd;
//     }
//     return CHIP_NO_ERROR;
// };

CHIP_ERROR Engine::GetCommandCompletions(cmd_completion_context * context)
{

    if (Engine::theShellMapListHead == NULL)
    {
        ChipLogDetail(Shell, "There isn't any command registered yet.");
        return CHIP_NO_ERROR;
    }
    const char * buf = context->line_buf;
    if (buf == nullptr)
    {
        buf = "";
    }

    int last_space_idx = -1;
    int buf_len        = strlen(buf);

    // find space in buf
    for (int i = buf_len - 1; i > -1; i--)
    {
        if (buf[i] == ' ')
        {
            last_space_idx = i;
            break;
        }
    }

    // check whether the buf perfectly matches a prefix
    bool perfect_match = false;
    shell_map_t * map  = Engine::theShellMapListHead;

    while (map != NULL)
    {

        if (strcmp(buf, map->prefix) == 0)
        {
            perfect_match = true;
            break;
        }
        map = map->next;
    }

    char * prefix;
    char * incomplete_cmd;

    if (perfect_match)
    {
        // If it's a perfect match
        // - use the whole buf as the prefix
        // - there is no "incomplete command" so set it to empty
        prefix         = new char[buf_len + 1];
        incomplete_cmd = new char[1];
        strlcpy(prefix, buf, buf_len + 1);
        strlcpy(incomplete_cmd, "", 1);
    }
    else
    {
        // If it's not a perfect match:
        // - prefix is up to the last space
        // - incomplete_cmd is what's after the last space
        bool no_space  = (last_space_idx == -1) ? true : false;
        prefix         = new char[last_space_idx + 1 + no_space];
        incomplete_cmd = new char[buf_len - last_space_idx];
        strlcpy(prefix, buf, last_space_idx + 1 + no_space);
        strlcpy(incomplete_cmd, &buf[last_space_idx + 1], buf_len - last_space_idx);
    }

    // Array to pass arguments into the ForEachCommand call
    void * lambda_args[] = { context, incomplete_cmd };
    map                  = Engine::theShellMapListHead;

    while (map != NULL && context->cmdc < CHIP_SHELL_MAX_CMD_COMPLETIONS)
    {
        if (strcmp(prefix, map->prefix) == 0)
        {
            context->ret_prefix = map->prefix;
            for (unsigned i = 0; i < map->enginec; i++)
            {
                map->enginev[i]->ForEachCommand(
                    [](shell_command_t * cmd, void * arg) -> CHIP_ERROR {
                        cmd_completion_context * ctx = (cmd_completion_context *) ((void **) arg)[0];
                        char * _incomplete_cmd       = (char *) ((void **) arg)[1];
                        // For end nodes like "ble adv", need to avoid duplicate returns.
                        // Return for prefix="ble adv" cmd=""; reject for prefix="ble" cmd="adv"
                        if ((strcmp(cmd->cmd_name, _incomplete_cmd) != 0 &&
                             strncmp(cmd->cmd_name, _incomplete_cmd, strlen(_incomplete_cmd)) == 0) ||
                            strcmp(_incomplete_cmd, "") == 0)
                        {
                            ctx->cmdc++;
                            ctx->cmdv[ctx->cmdc - 1] = cmd;
                        }
                        return CHIP_NO_ERROR;
                    },
                    lambda_args);
            }
            break;
        }

        map = map->next;
    }

    delete[] prefix;
    delete[] incomplete_cmd;

    return CHIP_NO_ERROR;
}

CHIP_ERROR Engine::ExecCommand(int argc, char * argv[])
{
    CHIP_ERROR retval = CHIP_ERROR_INVALID_ARGUMENT;
    VerifyOrReturnError(argc > 0, retval);
    // Find the command
    for (unsigned i = 0; i < _commandCount; i++)
    {
        if (strcmp(argv[0], _commands[i]->cmd_name) == 0)
        {
            // Execute the command!
            retval = _commands[i]->cmd_func(argc - 1, argv + 1);
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
