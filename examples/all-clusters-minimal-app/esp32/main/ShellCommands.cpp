/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    sSubShell.RegisterCommands(subCommands, ArraySize(subCommands));

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
    sSubShell.RegisterCommands(subCommands, ArraySize(subCommands));

    static const shell_command_t CASECommand = { &CASEHandler, "case", "Case Commands" };
    Engine::Root().RegisterCommands(&CASECommand, 1);
}
} // namespace Shell
} // namespace chip
