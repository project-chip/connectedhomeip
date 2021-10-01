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

bool ServerClusterCommandExists(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId)
{
    // The Mock cluster catalog -- only have one command on one cluster on one endpoint.
    return (aEndPointId == kTestEndpointId && aClusterId == kTestClusterId && aCommandId == kTestCommandId);
}

void DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                  chip::TLV::TLVReader & aReader, CommandHandler * apCommandObj)
{
    static bool statusCodeFlipper = false;

    if (aClusterId != kTestClusterId || aCommandId != kTestCommandId || aEndPointId != kTestEndpointId)
    {
        return;
    }

    if (aReader.GetLength() != 0)
    {
        chip::TLV::Debug::Dump(aReader, TLVPrettyPrinter);
    }

    chip::app::CommandPathParams commandPathParams = { kTestEndpointId, // Endpoint
                                                       kTestGroupId,    // GroupId
                                                       kTestClusterId,  // ClusterId
                                                       kTestCommandId,  // CommandId
                                                       (chip::app::CommandPathFlags::kEndpointIdValid) };

    // Add command data here
    if (statusCodeFlipper)
    {
        printf("responder constructing status code in command");
        apCommandObj->AddStatusCode(commandPathParams, Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                    Protocols::InteractionModel::Id, Protocols::InteractionModel::Status::Success);
    }
    else
    {
        printf("responder constructing command data in command");

        chip::TLV::TLVWriter * writer;

        ReturnOnFailure(apCommandObj->PrepareCommand(commandPathParams));

        writer = apCommandObj->GetCommandDataElementTLVWriter();
        ReturnOnFailure(writer->Put(chip::TLV::ContextTag(kTestFieldId1), kTestFieldValue1));

        ReturnOnFailure(writer->Put(chip::TLV::ContextTag(kTestFieldId2), kTestFieldValue2));

        ReturnOnFailure(apCommandObj->FinishCommand());
    }
    statusCodeFlipper = !statusCodeFlipper;
}

void DispatchSingleClusterResponseCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                          chip::TLV::TLVReader & aReader, CommandSender * apCommandObj)
{
    ChipLogDetail(Controller, "Received Cluster Command: Cluster=%" PRIx32 " Command=%" PRIx32 " Endpoint=%" PRIx16, aClusterId,
                  aCommandId, aEndPointId);
    // Nothing todo.
}

CHIP_ERROR ReadSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVWriter * apWriter, bool * apDataExists)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    uint64_t version = 0;
    VerifyOrExit(aClusterInfo.mClusterId == kTestClusterId && aClusterInfo.mEndpointId == kTestEndpointId,
                 err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(apWriter != nullptr, /* no op */);

    err = apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), kTestFieldValue1);
    SuccessOrExit(err);
    err = apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_DataVersion), version);

exit:
    return err;
}

CHIP_ERROR WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler * apWriteHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathParams attributePathParams;
    attributePathParams.mNodeId     = 1;
    attributePathParams.mEndpointId = 2;
    attributePathParams.mClusterId  = 3;
    attributePathParams.mFieldId    = 4;
    attributePathParams.mListIndex  = 5;
    attributePathParams.mFlags.Set(AttributePathParams::Flags::kFieldIdValid);

    err = apWriteHandler->AddAttributeStatusCode(attributePathParams, Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                                 Protocols::SecureChannel::Id, Protocols::InteractionModel::Status::Success);
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
    dirtyPath.mFlags.Set(chip::app::ClusterInfo::Flags::kFieldIdValid);
    printf("MutateClusterHandler is triggered...");
    // send dirty change
    if (!testSyncReport)
    {
        dirtyPath.mFieldId = 1;
        chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(dirtyPath);
        chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleRun();
        chip::DeviceLayer::SystemLayer().StartTimer(1000, MutateClusterHandler, NULL);
        testSyncReport = true;
    }
    else
    {
        dirtyPath.mFieldId = 10; // unknown field
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
        chip::DeviceLayer::SystemLayer().StartTimer(1000, MutateClusterHandler, NULL);
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

    err = gTransportManager.Init(chip::Transport::UdpListenParameters(&chip::DeviceLayer::InetLayer)
#if INET_CONFIG_ENABLE_IPV4
                                     .SetAddressType(chip::Inet::kIPAddressType_IPv4)
#else
                                     .SetAddressType(chip::Inet::kIPAddressType_IPv6)
#endif
    );
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

    ShutdownChip();

    return EXIT_SUCCESS;
}
