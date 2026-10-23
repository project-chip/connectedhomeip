/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <ShellCommands.h>

namespace chip {
namespace Shell {

Shell::Engine OnOffCommands::sSubShell;
void OnOffCommands::Register()
{
    static const shell_command_t subCommands[] = { { &OnLightHandler, "on", "Usage: OnOff on endpoint-id" },
                                                   { &OffLightHandler, "off", "Usage: OnOff off endpoint-id" },
                                                   { &ToggleLightHandler, "toggle", "Usage: OnOff toggle endpoint-id" } };
    sSubShell.RegisterCommands(subCommands, MATTER_ARRAY_SIZE(subCommands));

    // Register the root `OnOff` command in the top-level shell.
    static const shell_command_t onOffCommand = { &OnOffHandler, "OnOff", "OnOff commands" };

    Engine::Root().RegisterCommands(&onOffCommand, 1);
}

Callback::Callback<OnDeviceConnected> CASECommands::sOnConnectedCallback(CASECommands::OnConnected, nullptr);
Callback::Callback<OnDeviceConnectionFailure> CASECommands::sOnConnectionFailureCallback(CASECommands::OnConnectionFailure,
                                                                                         nullptr);
Shell::Engine CASECommands::sSubShell;
void CASECommands::Register()
{
    static const shell_command_t subCommands[] = {
        { &ConnectToNodeHandler, "connect", "Establish CASESession to a node, Usage: case connect <fabric-index> <node-id>" },
    };
    sSubShell.RegisterCommands(subCommands, MATTER_ARRAY_SIZE(subCommands));

    static const shell_command_t CASECommand = { &CASEHandler, "case", "Case Commands" };
    Engine::Root().RegisterCommands(&CASECommand, 1);
}
} // namespace Shell
} // namespace chip
