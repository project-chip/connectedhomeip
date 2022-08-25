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

#pragma once

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CASESessionManager.h>
#include <app/server/Server.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

namespace chip {
namespace Shell {

class OnOffCommands
{
public:
    // delete the copy constructor
    OnOffCommands(const OnOffCommands &) = delete;
    // delete the move constructor
    OnOffCommands(OnOffCommands &&) = delete;
    // delete the assignment operator
    OnOffCommands & operator=(const OnOffCommands &) = delete;

    static OnOffCommands & GetInstance()
    {
        static OnOffCommands instance;
        return instance;
    }

    // Register the OnOff commands
    void Register();

private:
    OnOffCommands() {}

    static CHIP_ERROR OnOffHandler(int argc, char ** argv)
    {
        if (argc == 0)
        {
            sSubShell.ForEachCommand(PrintCommandHelp, nullptr);
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR error = sSubShell.ExecCommand(argc, argv);

        if (error != CHIP_NO_ERROR)
        {
            streamer_printf(streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", error.Format());
        }

        return error;
    }

    static CHIP_ERROR OnLightHandler(int argc, char ** argv)
    {
        if (argc != 1)
            return CHIP_ERROR_INVALID_ARGUMENT;
        chip::app::Clusters::OnOff::Attributes::OnOff::Set(atoi(argv[0]), 1);
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR OffLightHandler(int argc, char ** argv)
    {
        if (argc != 1)
            return CHIP_ERROR_INVALID_ARGUMENT;
        chip::app::Clusters::OnOff::Attributes::OnOff::Set(atoi(argv[0]), 0);
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR ToggleLightHandler(int argc, char ** argv)
    {
        if (argc != 1)
            return CHIP_ERROR_INVALID_ARGUMENT;
        bool value;
        chip::app::Clusters::OnOff::Attributes::OnOff::Get(atoi(argv[0]), &value);
        chip::app::Clusters::OnOff::Attributes::OnOff::Set(atoi(argv[0]), !value);
        return CHIP_NO_ERROR;
    }

    static Shell::Engine sSubShell;
};

class CASECommands
{
public:
    // delete the copy constructor
    CASECommands(const CASECommands &) = delete;
    // delete the move constructor
    CASECommands(CASECommands &&) = delete;
    // delete the assignment operator
    CASECommands & operator=(const CASECommands &) = delete;

    static CASECommands & GetInstance()
    {
        static CASECommands instance;
        return instance;
    }

    // Register the CASESession commands
    void Register();

    void SetFabricInfo(const FabricInfo * fabricInfo) { mFabricInfo = fabricInfo; }
    void SetNodeId(NodeId nodeId) { mNodeId = nodeId; }
    void SetOnConnecting(bool onConnecting) { mOnConnecting = onConnecting; }
    const FabricInfo * GetFabricInfo(void) { return mFabricInfo; }
    NodeId GetNodeId(void) { return mNodeId; }
    bool GetOnConnecting(void) { return mOnConnecting; }

private:
    CASECommands() {}
    static void OnConnected(void * context, Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle)
    {
        streamer_printf(streamer_get(), "Establish CASESession Success!\r\n");
        GetInstance().SetOnConnecting(false);
    }

    static void OnConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
    {
        streamer_printf(streamer_get(), "Establish CASESession Failure!\r\n");
        GetInstance().SetOnConnecting(false);
    }

    static void ConnectToNode(intptr_t arg)
    {
        CASECommands * caseCommand              = reinterpret_cast<CASECommands *>(arg);
        Server * server                         = &(chip::Server::GetInstance());
        CASESessionManager * caseSessionManager = server->GetCASESessionManager();
        if (caseSessionManager == nullptr)
        {
            ChipLogError(SecureChannel, "Can't get the CASESessionManager");
            return;
        }
        caseSessionManager->FindOrEstablishSession(
            ScopedNodeId(caseCommand->GetNodeId(), caseCommand->GetFabricInfo()->GetFabricIndex()), &sOnConnectedCallback,
            &sOnConnectionFailureCallback);
    }

    static CHIP_ERROR ConnectToNodeHandler(int argc, char ** argv)
    {
        if (GetInstance().GetOnConnecting())
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        const FabricIndex fabricIndex = static_cast<FabricIndex>(strtoul(argv[0], nullptr, 10));
        const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);

        if (fabricInfo == nullptr)
        {
            streamer_printf(streamer_get(), "Can't get fabric information from FabricIndex\r\n");
            return CHIP_ERROR_NOT_FOUND;
        }
        GetInstance().SetFabricInfo(fabricInfo);
        GetInstance().SetNodeId(static_cast<NodeId>(strtoul(argv[1], nullptr, 10)));
        streamer_printf(streamer_get(), "Try to establish CaseSession to NodeId:0x" ChipLogFormatX64 " on fabric index %d\r\n",
                        ChipLogValueX64(GetInstance().GetNodeId()), fabricIndex);
        GetInstance().SetOnConnecting(true);
        chip::DeviceLayer::PlatformMgr().ScheduleWork(ConnectToNode, reinterpret_cast<intptr_t>(&GetInstance()));
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR CASEHandler(int argc, char ** argv)
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

    static Callback::Callback<OnDeviceConnected> sOnConnectedCallback;
    static Callback::Callback<OnDeviceConnectionFailure> sOnConnectionFailureCallback;
    static Shell::Engine sSubShell;
    const FabricInfo * mFabricInfo = nullptr;
    NodeId mNodeId                 = 0;
    bool mOnConnecting             = false;
};

} // namespace Shell
} // namespace chip
