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

#include "app/InteractionModelEngine.h"
#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/tests/integration/common.h>
#include <core/CHIPCore.h>
#include <platform/CHIPDeviceLayer.h>

#include "InteractionModelEngine.h"
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/PASESession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

namespace chip {
namespace app {

void DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                  chip::TLV::TLVReader & aReader, Command * apCommandObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aClusterId != kTestClusterId || aCommandId != kTestCommandId || aEndPointId != kTestEndPointId)
    {
        return;
    }

    if (aReader.GetLength() != 0)
    {
        chip::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
    }

    chip::app::Command::CommandParams commandParams = { kTestEndPointId, // Endpoint
                                                        kTestGroupId,    // GroupId
                                                        kTestClusterId,  // ClusterId
                                                        kTestCommandId,  // CommandId
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
} // namespace app
} // namespace chip

namespace {

// The CommandHandler object
chip::TransportMgr<chip::Transport::UDP> gTransportManager;
chip::SecureSessionMgr gSessionManager;
chip::SecurePairingUsingTestSecret gTestPairing;

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    const chip::Transport::AdminId gAdminId = 0;
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(gAdminId, chip::kTestDeviceNodeId);

    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

    InitializeChip();

    err = gTransportManager.Init(
        chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv4));
    SuccessOrExit(err);

    err = gSessionManager.Init(chip::kTestDeviceNodeId, &chip::DeviceLayer::SystemLayer, &gTransportManager, &admins);
    SuccessOrExit(err);

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeManager);
    SuccessOrExit(err);

    err = gSessionManager.NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing,
                                     chip::SecureSessionMgr::PairingDirection::kResponder, gAdminId);
    SuccessOrExit(err);

    printf("Listening for IM requests...\n");

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:

    if (err != CHIP_NO_ERROR)
    {
        printf("CommandHandler failed, err:%s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    chip::app::InteractionModelEngine::GetInstance()->Shutdown();

    ShutdownChip();

    return EXIT_SUCCESS;
}
