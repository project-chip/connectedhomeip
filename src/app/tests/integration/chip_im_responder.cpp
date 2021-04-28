/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      Currently it provides simple command and read handler with sample cluster
 *
 */

#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/integration/common.h>
#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

namespace chip {
namespace app {
void DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                  chip::TLV::TLVReader & aReader, Command * apCommandObj)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    static bool statusCodeFlipper = false;

    if (aClusterId != kTestClusterId || aCommandId != kTestCommandId || aEndPointId != kTestEndPointId)
    {
        return;
    }

    if (aReader.GetLength() != 0)
    {
        chip::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
    }

    chip::app::CommandPathParams commandPathParams = { kTestEndPointId, // Endpoint
                                                       kTestGroupId,    // GroupId
                                                       kTestClusterId,  // ClusterId
                                                       kTestCommandId,  // CommandId
                                                       (chip::app::CommandPathFlags::kEndpointIdValid) };

    // Add command data here

    uint8_t effectIdentifier = 1; // Dying light
    uint8_t effectVariant    = 1;

    if (statusCodeFlipper)
    {
        printf("responder constructing status code in command");
        apCommandObj->AddStatusCode(&commandPathParams, Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                    Protocols::SecureChannel::Id, Protocols::SecureChannel::kProtocolCodeSuccess);
    }
    else
    {
        printf("responder constructing command data in command");

        chip::TLV::TLVWriter * writer;

        err = apCommandObj->PrepareCommand(&commandPathParams);
        SuccessOrExit(err);

        writer = apCommandObj->GetCommandDataElementTLVWriter();
        err    = writer->Put(chip::TLV::ContextTag(1), effectIdentifier);
        SuccessOrExit(err);

        err = writer->Put(chip::TLV::ContextTag(2), effectVariant);
        SuccessOrExit(err);

        err = apCommandObj->FinishCommand();
        SuccessOrExit(err);
    }
    statusCodeFlipper = !statusCodeFlipper;

exit:
    return;
}

} // namespace app
} // namespace chip

namespace {
chip::TransportMgr<chip::Transport::UDP> gTransportManager;
chip::SecureSessionMgr gSessionManager;
chip::SecurePairingUsingTestSecret gTestPairing;
chip::secure_channel::MessageCounterManager gMessageCounterManager;

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::InteractionModelDelegate mockDelegate;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    const chip::Transport::AdminId gAdminId = 0;
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(gAdminId, chip::kTestDeviceNodeId);

    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

    InitializeChip();

    err = gTransportManager.Init(
        chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv4));
    SuccessOrExit(err);

    err = gSessionManager.Init(chip::kTestDeviceNodeId, &chip::DeviceLayer::SystemLayer, &gTransportManager, &admins,
                               &gMessageCounterManager);
    SuccessOrExit(err);

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeManager, &mockDelegate);
    SuccessOrExit(err);

    err = gSessionManager.NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing,
                                     chip::SecureSessionMgr::PairingDirection::kResponder, gAdminId);
    SuccessOrExit(err);

    printf("Listening for IM requests...\n");

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:

    if (err != CHIP_NO_ERROR)
    {
        printf("IM responder failed, err:%s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    chip::app::InteractionModelEngine::GetInstance()->Shutdown();

    ShutdownChip();

    return EXIT_SUCCESS;
}
