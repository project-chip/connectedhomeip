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
#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

namespace chip {
namespace app {

bool ServerClusterCommandExists(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId)
{
    // The Mock cluster catalog -- only have one command on one cluster on one endpoint.
    return (aEndPointId == kTestEndpointId && aClusterId == kTestClusterId && aCommandId == kTestCommandId);
}

void DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                  chip::TLV::TLVReader & aReader, Command * apCommandObj)
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
                                    Protocols::InteractionModel::Id, Protocols::InteractionModel::ProtocolCode::Success);
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

CHIP_ERROR ReadSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVWriter * apWriter, bool * apDataExists)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(aClusterInfo.mClusterId == kTestClusterId && aClusterInfo.mEndpointId == kTestEndpointId,
                 err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(apWriter != nullptr, /* no op */);

    if (aClusterInfo.mFieldId == kRootFieldId || aClusterInfo.mFieldId == 1)
    {
        err = apWriter->Put(TLV::ContextTag(kTestFieldId1), kTestFieldValue1);
        SuccessOrExit(err);
    }
    if (aClusterInfo.mFieldId == kRootFieldId || aClusterInfo.mFieldId == 2)
    {
        err = apWriter->Put(TLV::ContextTag(kTestFieldId2), kTestFieldValue2);
        SuccessOrExit(err);
    }

exit:
    ChipLogFunctError(err);
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
                                                 Protocols::SecureChannel::Id, Protocols::InteractionModel::ProtocolCode::Success);
    ChipLogFunctError(err);
    return err;
}
} // namespace app
} // namespace chip

namespace {
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

    InitializeEventLogging(&gExchangeManager);

    err = gSessionManager.NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing, chip::SecureSession::SessionRole::kResponder,
                                     gAdminId);
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
