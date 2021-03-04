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
 *      Currently it provides simple command and read handler with sample cluster
 *
 */

#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/LoggingConfiguration.h>
#include <app/reporting/LoggingManagement.h>
#include <app/tests/integration/MockEvents.h>
#include <app/tests/integration/common.h>
#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
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
chip::TransportMgr<chip::Transport::UDP> gTransportManager;
chip::SecureSessionMgr gSessionManager;
chip::SecurePairingUsingTestSecret gTestPairing;
LivenessEventGenerator gLivenessGenerator;

uint8_t gDebugEventBuffer[2048];
uint8_t gInfoEventBuffer[2048];
uint8_t gCritEventBuffer[2048];

void InitializeEventLogging(chip::Messaging::ExchangeManager * apMgr)
{
    chip::app::reporting::LogStorageResources logStorageResources[] = {
        { &gCritEventBuffer[0], sizeof(gDebugEventBuffer), nullptr, 0, nullptr, chip::app::reporting::PriorityLevel::Critical },
        { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), nullptr, 0, nullptr, chip::app::reporting::PriorityLevel::Info },
        { &gDebugEventBuffer[0], sizeof(gCritEventBuffer), nullptr, 0, nullptr, chip::app::reporting::PriorityLevel::Debug },
    };

    chip::app::reporting::LoggingManagement::CreateLoggingManagement(
        apMgr, sizeof(logStorageResources) / sizeof(logStorageResources[0]), logStorageResources);
    chip::app::reporting::LoggingConfiguration::GetInstance().SetGlobalPriorityLevel(chip::app::reporting::PriorityLevel::Debug);
}

class MockInteractionModelApp : public chip::app::InteractionModelDelegate
{
public:
    void HandleIMCallBack(chip::app::InteractionModelDelegate::CallbackId aCallbackId,
                          const chip::app::InteractionModelDelegate::InParam & aInParam,
                          chip::app::InteractionModelDelegate::OutParam & aOutParam)
    {
        switch (aCallbackId)
        {
        default:
            chip::app::InteractionModelDelegate::DefaultCallbackIdHandler(aCallbackId, aInParam, aOutParam);
            break;
        }
    }
};

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MockInteractionModelApp mockDelegate;
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

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeManager, &mockDelegate);
    SuccessOrExit(err);

    InitializeEventLogging(&gExchangeManager);

    err = gSessionManager.NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing,
                                     chip::SecureSessionMgr::PairingDirection::kResponder, gAdminId);
    SuccessOrExit(err);

    printf("Listening for IM requests...\n");

    MockEventGenerator::GetInstance()->Init(&gExchangeManager, &gLivenessGenerator, 1000, true);

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:
    MockEventGenerator::GetInstance()->SetEventGeneratorStop();

    if (err != CHIP_NO_ERROR)
    {
        printf("IM responder failed, err:%s\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }

    chip::app::InteractionModelEngine::GetInstance()->Shutdown();

    ShutdownChip();

    return EXIT_SUCCESS;
}
