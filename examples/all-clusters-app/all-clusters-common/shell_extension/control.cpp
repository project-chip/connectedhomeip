/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "control.hpp"

#include "app/common/gen/enums.h"
#include "app/util/DataModelHandler.h"
#include "app/util/basic-types.h"
#include "core/CHIPError.h"
#include "core/NodeId.h"
#include "messaging/ExchangeContext.h"
#include "messaging/ExchangeDelegate.h"
#include "protocols/secure_channel/SessionEstablishmentDelegate.h"
#include "support/logging/CHIPLogging.h"
#include "transport/FabricTable.h"
#include "transport/SessionHandle.h"
#include "transport/raw/PeerAddress.h"
#include <app/server/Server.h>
#include <bits/stdint-uintn.h>
#include <controller/CHIPDevice.h>
#include <controller/InteractionPeer.h>
#include <credentials/CHIPCert.h>
#include <cstdio>
#include <lib/shell/commands/Help.h>
#include <lib/shell/streamer.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <memory>
#include <protocols/secure_channel/CASESession.h>
#include <zap-generated/CHIPClusters.h>

using chip::CASESession;
using chip::FabricIndex;
using chip::NodeId;
using chip::Optional;
using chip::SecureSessionMgr;
using chip::Server;
using chip::SessionEstablishmentDelegate;
using chip::SessionHandle;
using chip::Controller::InteractionPeer;
using chip::Controller::InteractionPeerDelegate;
using chip::Messaging::ExchangeContext;
using chip::Messaging::ExchangeDelegate;
using chip::Shell::Engine;
using chip::Shell::PrintCommandHelp;
using chip::Shell::shell_command_t;
using chip::Shell::streamer_get;
using chip::Shell::streamer_printf;
using chip::Transport::FabricInfo;
using chip::Transport::PeerAddress;

namespace {

class Device : public InteractionPeerDelegate, public SessionEstablishmentDelegate, public ExchangeDelegate
{
public:
    Device(NodeId nodeId, PeerAddress address, FabricIndex fabricIndex) :
        mPeerId(nodeId), mPeerAddress(address), mFabricIndex(fabricIndex), mInteractionPeer(this)
    {}

    CHIP_ERROR Connect()
    {
        uint16_t keyID = 0;
        ReturnErrorOnFailure(Server::GetServer().GetSessionIDAllocator().Allocate(keyID));
        ExchangeContext * exchange = Server::GetServer().GetExchangManager().NewContext(SessionHandle(), &mCASESession);
        VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_INTERNAL);
        ReturnErrorOnFailure(
            mCASESession.MessageDispatch().Init(Server::GetServer().GetSecureSessionManager().GetTransportManager()));
        mCASESession.MessageDispatch().SetPeerAddress(mPeerAddress);
        FabricInfo * fabric = Server::GetServer().GetFabricTable().FindFabricWithIndex(mFabricIndex);
        ReturnErrorCodeIf(fabric == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(mCASESession.EstablishSession(mPeerAddress, fabric, mPeerId, keyID, exchange, this));
        return CHIP_NO_ERROR;
    };

    CHIP_ERROR ToggleOnOff(void)
    {
        mOnOffCluster.Associate(&mInteractionPeer, 1);
        return mOnOffCluster.Toggle(nullptr, nullptr);
    }

private:
    CHIP_ERROR PreparePeer() override { return CHIP_NO_ERROR; };

    NodeId GetDeviceId() const override { return mPeerId; }

    FabricIndex GetFabricIndex() const override { return mFabricIndex; }

    SessionHandle * GetSessionHandle() override { return &mSessionHandle; }

    CHIP_ERROR SendMessage(chip::Protocols::Id protocolId, uint8_t msgType, chip::Messaging::SendFlags sendFlags,
                           chip::System::PacketBufferHandle && message) override
    {
        ExchangeContext * exchange = Server::GetServer().GetExchangManager().NewContext(mSessionHandle, nullptr);
        VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_NO_MEMORY);
        sendFlags.Set(chip::Messaging::SendMessageFlags::kFromInitiator);
        exchange->SetDelegate(this);
        return exchange->SendMessage(protocolId, msgType, std::move(message), sendFlags);
    }

    void OnSessionEstablishmentError(CHIP_ERROR error) override
    {
        ChipLogError(Controller, "OnSessionEstablishmentError: %s", ErrorStr(error));
        Server::GetServer().GetSessionIDAllocator().Free(mCASESession.GetLocalKeyId());
    }

    void OnSessionEstablished() override
    {
        ChipLogProgress(Controller, "OnSessionEstablished");
        mCASESession.SetPeerNodeId(mPeerId);
        mSessionHandle = { mPeerId, mCASESession.GetLocalKeyId(), mCASESession.GetPeerKeyId(), mFabricIndex };
        CHIP_ERROR err = Server::GetServer().GetSecureSessionManager().NewPairing(
            Optional<PeerAddress>::Value(mPeerAddress), mPeerId, &mCASESession, chip::SecureSession::SessionRole::kInitiator,
            mFabricIndex);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed in setting up CASE secure channel: err %s", ErrorStr(err));
            OnSessionEstablishmentError(err);
        }
        else
        {
            ChipLogProgress(Controller, "Session establish done");
        }
    }

    CHIP_ERROR OnMessageReceived(ExchangeContext * exchange, const chip::PacketHeader & header,
                                 const chip::PayloadHeader & payloadHeader, chip::System::PacketBufferHandle && msgBuf) override
    {
        HandleDataModelMessage(exchange, std::move(msgBuf));
        exchange->Close();
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * exchange) override
    {
        exchange->Close();
        ChipLogError(Controller, "Control message timeout");
    }

    NodeId mPeerId;
    PeerAddress mPeerAddress;
    FabricIndex mFabricIndex;

    SessionHandle mSessionHandle;
    CASESession mCASESession;

    InteractionPeer mInteractionPeer;
    chip::Controller::OnOffCluster mOnOffCluster;
};

std::unique_ptr<Device> sDevice;

Engine sControlSubCommands;

CHIP_ERROR ControlHelpHandler(int argc, char ** argv)
{
    sControlSubCommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

uint8_t HexCharToInt(char value)
{
    if ('0' <= value && value <= '9')
    {
        return static_cast<uint8_t>(value - '0');
    }
    if ('a' <= value && value <= 'f')
    {
        return static_cast<uint8_t>(0xa + value - 'a');
    }
    if ('A' <= value && value <= 'F')
    {
        return static_cast<uint8_t>(0xa + value - 'A');
    }
    assert(false);
    return 0;
}

chip::NodeId HexStringToNodeId(const char * hex)
{
    chip::NodeId nodeId = 0;

    for (const char * c = hex; *c; c++)
    {
        nodeId <<= 4;
        nodeId += HexCharToInt(*c);
    }
    return nodeId;
}

CHIP_ERROR ControlConnectHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    chip::Inet::IPAddress addr;
    chip::Inet::IPAddress::FromString(argv[0], addr);
    chip::Transport::PeerAddress peer_addr = chip::Transport::PeerAddress::UDP(addr, CHIP_PORT);
    chip::NodeId nodeId                    = HexStringToNodeId(argv[1]);

    sDevice = std::make_unique<Device>(nodeId, peer_addr, /*fabricIndex=*/1);
    sDevice->Connect();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ControlToggleHandler(int argc, char ** argv)
{
    if (!sDevice)
    {
        streamer_printf(streamer_get(), "Not connected");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return sDevice->ToggleOnOff();
}

CHIP_ERROR ControlDispatch(int argc, char ** argv)
{
    if (argc == 0)
    {
        ControlHelpHandler(argc, argv);
        return CHIP_NO_ERROR;
    }

    return sControlSubCommands.ExecCommand(argc, argv);
}

} // namespace

namespace chip {

void RegisterControlCommands()
{
    static const shell_command_t sHeapSubCommands[] = {
        { &ControlHelpHandler, "help", "Usage: control <subcommand>" },
        { &ControlConnectHandler, "connect", "Connect to device with CASE" },
        { &ControlToggleHandler, "toggle", "Toggle OnOff" },
    };
    sControlSubCommands.RegisterCommands(sHeapSubCommands, ArraySize(sHeapSubCommands));

    static const shell_command_t sHeapCommand = { &ControlDispatch, "control", "Control other nodes" };
    Engine::Root().RegisterCommands(&sHeapCommand, 1);
}

} // namespace chip
