/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements a chip-im-responder, for the
 *      CHIP Interaction Data Model Protocol.
 *
 *      Currently it provides simple command handler with sample cluster and command
 *
 */

#include "common.h"

#include "app/InteractionModelEngine.h"
#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <core/CHIPCore.h>
#include <platform/CHIPDeviceLayer.h>

#include "InteractionModelEngine.h"
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecurePairingSession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

namespace {

// The CommandHandler object
chip::TransportMgr<chip::Transport::UDP> gTransportManager;
chip::SecureSessionMgr gSessionManager;
chip::SecurePairingUsingTestSecret gTestPairing;

// Callback handler when a CHIP EchoRequest is received.
void HandleCommandRequestReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aReader.GetLength() != 0)
    {
        chip::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
    }

    chip::app::Command::CommandParams commandParams = { 1,  // Endpoint
                                                        0,  // GroupId
                                                        6,  // ClusterId
                                                        40, // CommandId
                                                        (chip::app::Command::kCommandPathFlag_EndpointIdValid) };

    // Add command data here

    uint8_t effectIdentifier = 1; // Dying light
    uint8_t effectVariant    = 1;

    chip::TLV::TLVType dummyType = chip::TLV::kTLVType_NotSpecified;

    chip::TLV::TLVWriter writer = apCommandObj->CreateCommandDataElementTLVWriter();

    printf("responder constructing response");
    err = writer.StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, dummyType);
    SuccessOrExit(err);

    err = writer.Put(chip::TLV::ContextTag(1), effectIdentifier);
    SuccessOrExit(err);

    err = writer.Put(chip::TLV::ContextTag(2), effectVariant);
    SuccessOrExit(err);

    err = writer.EndContainer(dummyType);
    SuccessOrExit(err);

    err = writer.Finalize();
    SuccessOrExit(err);

    err = apCommandObj->AddCommand(commandParams);
    SuccessOrExit(err);

exit:
    return;
}

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);

    InitializeChip();

    err = gTransportManager.Init(
        chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv4));
    SuccessOrExit(err);

    err = gSessionManager.Init(chip::kTestDeviceNodeId, &chip::DeviceLayer::SystemLayer, &gTransportManager);
    SuccessOrExit(err);

    err = gExchangeManager.Init(chip::kTestDeviceNodeId, &gTransportManager, &gSessionManager);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeManager);
    SuccessOrExit(err);

    err = gSessionManager.NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing);
    SuccessOrExit(err);

    chip::app::InteractionModelEngine::GetInstance()->RegisterClusterCommandHandler(
        6, 40, chip::app::Command::CommandRoleId::kCommandHandlerId, HandleCommandRequestReceived);
    printf("Listening for IM requests...\n");

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:
    chip::app::InteractionModelEngine::GetInstance()->DeregisterClusterCommandHandler(
        6, 40, chip::app::Command::CommandRoleId::kCommandHandlerId);

    if (err != CHIP_NO_ERROR)
    {
        printf("CommandHandler failed, err:%s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    chip::app::InteractionModelEngine::GetInstance()->Shutdown();

    ShutdownChip();

    return EXIT_SUCCESS;
}
