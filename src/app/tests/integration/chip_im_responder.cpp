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

#include "MockEvents.h"
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/integration/common.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/ErrorStr.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>
#include <transport/raw/UDP.h>

namespace chip {
namespace app {

bool ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    // The Mock cluster catalog -- only have one command on one cluster on one endpoint.
    return (aCommandPath.mEndpointId == kTestEndpointId && aCommandPath.mClusterId == kTestClusterId &&
            aCommandPath.mCommandId == kTestCommandId);
}

void DispatchSingleClusterCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj)
{
    static bool statusCodeFlipper = false;

    if (!ServerClusterCommandExists(aCommandPath))
    {
        return;
    }

    if (aReader.GetLength() != 0)
    {
        chip::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
    }

    chip::app::ConcreteCommandPath path = {
        kTestEndpointId, // Endpoint
        kTestClusterId,  // ClusterId
        kTestCommandId,  // CommandId
    };

    // Add command data here
    if (statusCodeFlipper)
    {
        printf("responder constructing status code in command");
        apCommandObj->AddStatus(path, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        printf("responder constructing command data in command");

        chip::TLV::TLVWriter * writer;

        ReturnOnFailure(apCommandObj->PrepareCommand(path));

        writer = apCommandObj->GetCommandDataIBTLVWriter();
        ReturnOnFailure(writer->Put(chip::TLV::ContextTag(kTestFieldId1), kTestFieldValue1));

        ReturnOnFailure(writer->Put(chip::TLV::ContextTag(kTestFieldId2), kTestFieldValue2));

        ReturnOnFailure(apCommandObj->FinishCommand());
    }
    statusCodeFlipper = !statusCodeFlipper;
}

void DispatchSingleClusterResponseCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                          CommandSender * apCommandObj)
{
    // Nothing todo.
    (void) aCommandPath;
    (void) aReader;
    (void) apCommandObj;
}

CHIP_ERROR ReadSingleClusterData(FabricIndex aAccessingFabricIndex, const ConcreteReadAttributePath & aPath,
                                 AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeValueEncoder::AttributeEncodeState * apEncoderState)
{
    ReturnErrorOnFailure(AttributeValueEncoder(aAttributeReports, 0, aPath, 0).Encode(kTestFieldValue1));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler * apWriteHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathParams attributePathParams;
    attributePathParams.mEndpointId  = 2;
    attributePathParams.mClusterId   = 3;
    attributePathParams.mAttributeId = 4;
    err                              = apWriteHandler->AddStatus(attributePathParams, Protocols::InteractionModel::Status::Success);
    return err;
}
} // namespace app
} // namespace chip

namespace {
bool testSyncReport = false;
chip::TransportMgr<chip::Transport::UDP> gTransportManager;
chip::SecurePairingUsingTestSecret gTestPairing;
LivenessEventGenerator gLivenessGenerator;

uint8_t gDebugEventBuffer[2048];
uint8_t gInfoEventBuffer[2048];
uint8_t gCritEventBuffer[2048];
chip::app::CircularEventBuffer gCircularEventBuffer[3];

void InitializeEventLogging(chip::Messaging::ExchangeManager * apMgr)
{
    chip::app::LogStorageResources logStorageResources[] = {
        { &gCritEventBuffer[0], sizeof(gDebugEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Debug },
        { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Info },
        { &gDebugEventBuffer[0], sizeof(gCritEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Critical },
    };

    chip::app::EventManagement::CreateEventManagement(apMgr, sizeof(logStorageResources) / sizeof(logStorageResources[0]),
                                                      gCircularEventBuffer, logStorageResources);
}

void MutateClusterHandler(chip::System::Layer * systemLayer, void * appState)
{
    chip::app::ClusterInfo dirtyPath;
    dirtyPath.mClusterId  = kTestClusterId;
    dirtyPath.mEndpointId = kTestEndpointId;
    printf("MutateClusterHandler is triggered...");
    // send dirty change
    if (!testSyncReport)
    {
        dirtyPath.mAttributeId = 1;
        chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(dirtyPath);
        chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(1), MutateClusterHandler, NULL);
        testSyncReport = true;
    }
    else
    {
        dirtyPath.mAttributeId = 10; // unknown field
        chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(dirtyPath);
        // send sync message(empty report)
        chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
    }
}

class MockInteractionModelApp : public chip::app::InteractionModelDelegate
{
public:
    virtual CHIP_ERROR SubscriptionEstablished(const chip::app::ReadHandler * apReadHandler)
    {
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(1), MutateClusterHandler, NULL);
        return CHIP_NO_ERROR;
    }
};
} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MockInteractionModelApp mockDelegate;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    const chip::FabricIndex gFabricIndex = 0;

    InitializeChip();

    err = gTransportManager.Init(
        chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer()).SetAddressType(chip::Inet::IPAddressType::kIPv6));
    SuccessOrExit(err);

    err = gSessionManager.Init(&chip::DeviceLayer::SystemLayer(), &gTransportManager, &gMessageCounterManager);
    SuccessOrExit(err);

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeManager, &mockDelegate);
    SuccessOrExit(err);

    InitializeEventLogging(&gExchangeManager);

    err = gSessionManager.NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing, chip::CryptoContext::SessionRole::kResponder,
                                     gFabricIndex);
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
    gTransportManager.Close();
    ShutdownChip();

    return EXIT_SUCCESS;
}
