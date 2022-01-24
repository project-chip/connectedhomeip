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

#include <app/server/Server.h>
#include <app/CASESessionManager.h>
#include <platform/CHIPDeviceLayer.h>
#include <lib/support/CodeUtils.h>
#include <lib/shell/Engine.h>
#include <lib/shell/Commands.h>
#include <lib/shell/commands/Help.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip::DeviceLayer;

namespace chip {
namespace Shell {

Shell::Engine sSubShell;
static NodeId sNodeId;

static void OnConnected(void * context, OperationalDeviceProxy * deviceProxy)
{
    streamer_printf(streamer_get(), "Establish CASESession Success!\r\n");
}
Callback::Callback<OnDeviceConnected> sOnConnectedCallback(OnConnected, nullptr);
static void OnConnectionFailure(void * context, PeerId peerId, CHIP_ERROR error)
{
    streamer_printf(streamer_get(), "Establish CASESession Failure!\r\n");
}
Callback::Callback<OnDeviceConnectionFailure> sOnConnectionFailureCallback(OnConnectionFailure, nullptr);

void ConnectToNode(intptr_t arg)
{
    FabricInfo * fabricInfo = reinterpret_cast<FabricInfo *>(arg);
    Server * server = &(chip::Server::GetInstance());
    CASESessionManager * caseSessionManager = server->GetCASESessionManager();
    if (caseSessionManager == nullptr)
    {
        ChipLogError(SecureChannel, "Can't get the CASESessionManager");
        return;
    }
    caseSessionManager->FindOrEstablishSession(fabricInfo->GetPeerIdForNode(sNodeId),&sOnConnectedCallback,
                                               &sOnConnectionFailureCallback);
}

CHIP_ERROR ConnectToNodeHandler(int argc, char ** argv)
{
    const FabricIndex fabricIndex   = static_cast<FabricIndex>(strtoul(argv[0], nullptr, 10));
    FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);

    if (fabricInfo == nullptr)
    {
        streamer_printf(streamer_get(), "Can't get fabric information from FabricIndex\r\n");
        return CHIP_ERROR_NOT_FOUND;
    }
    sNodeId = static_cast<NodeId>(strtoul(argv[1], nullptr, 10));
    streamer_printf(streamer_get(), "Try to establish CaseSession to NodeId:0x" ChipLogFormatX64 " on fabric index %d\r\n",
                    ChipLogValueX64(sNodeId), fabricIndex);
    PlatformMgr().ScheduleWork(ConnectToNode, reinterpret_cast<intptr_t>(fabricInfo));
    return CHIP_NO_ERROR;
}

CHIP_ERROR caseHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        sSubShell.ForEachCommand(PrintCommandHelp, nullptr);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR err = sSubShell.ExecCommand(argc, argv);
    if (err != CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", err.Format());
    }
    return err;
}

void RegisterCaseCommands()
{
    static const shell_command_t subCommands[] = {
        { &ConnectToNodeHandler, "connect", "Establish CASESession to a node, Usage: case connect <fabric-index> <node-id>"},
    };
    sSubShell.RegisterCommands(subCommands, ArraySize(subCommands));

    static const shell_command_t caseCommand = { &caseHandler, "case", "Case Commands"};
    Engine::Root().RegisterCommands(&caseCommand, 1);
}

} // namespace Shell
} // namespace chip
