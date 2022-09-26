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
#include <lib/support/CHIPCounter.h>
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

Protocols::InteractionModel::Status ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    // The Mock cluster catalog -- only have one command on one cluster on one endpoint.
    using Protocols::InteractionModel::Status;

    if (aCommandPath.mEndpointId != kTestEndpointId)
    {
        return Status::UnsupportedEndpoint;
    }

    if (aCommandPath.mClusterId != kTestClusterId)
    {
        return Status::UnsupportedCluster;
    }

    if (aCommandPath.mCommandId != kTestCommandId)
    {
        return Status::UnsupportedCommand;
    }

    return Status::Success;
}

void DispatchSingleClusterCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj)
{
    static bool statusCodeFlipper = false;

    if (ServerClusterCommandExists(aCommandPath) != Protocols::InteractionModel::Status::Success)
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

CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                 const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeValueEncoder::AttributeEncodeState * apEncoderState)
{
    ReturnErrorOnFailure(AttributeValueEncoder(aAttributeReports, 0, aPath, 0).Encode(kTestFieldValue1));
    return CHIP_NO_ERROR;
}

bool ConcreteAttributePathExists(const ConcreteAttributePath & aPath)
{
    return true;
}

const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aConcreteClusterPath)
{
    // Note: This test does not make use of the real attribute metadata.
    static EmberAfAttributeMetadata stub = { .defaultValue = EmberAfDefaultOrMinMaxAttributeValue(uint32_t(0)) };
    return &stub;
}

CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteDataAttributePath & aPath,
                                  TLV::TLVReader & aReader, WriteHandler * apWriteHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ConcreteDataAttributePath attributePath(2, 3, 4);
    err = apWriteHandler->AddStatus(attributePath, Protocols::InteractionModel::Status::Success);
    return err;
}

bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion)
{
    return true;
}

bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint)
{
    return false;
}

} // namespace app
} // namespace chip

namespace {
chip::TransportMgr<chip::Transport::UDP> gTransportManager;
LivenessEventGenerator gLivenessGenerator;

uint8_t gDebugEventBuffer[2048];
uint8_t gInfoEventBuffer[2048];
uint8_t gCritEventBuffer[2048];
chip::app::CircularEventBuffer gCircularEventBuffer[3];

chip::MonotonicallyIncreasingCounter<chip::EventNumber> gEventCounter;

CHIP_ERROR InitializeEventLogging(chip::Messaging::ExchangeManager * apMgr)
{
    ReturnErrorOnFailure(gEventCounter.Init(0));

    chip::app::LogStorageResources logStorageResources[] = {
        { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
        { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
        { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
    };

    chip::app::EventManagement::CreateEventManagement(apMgr, sizeof(logStorageResources) / sizeof(logStorageResources[0]),
                                                      gCircularEventBuffer, logStorageResources, &gEventCounter);
    return CHIP_NO_ERROR;
}

} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Transport::PeerAddress peer(chip::Transport::Type::kUndefined);
    const chip::FabricIndex gFabricIndex = 0;

    InitializeChip();

    err = gTransportManager.Init(chip::Transport::UdpListenParameters(chip::DeviceLayer::UDPEndPointManager())
                                     .SetAddressType(chip::Inet::IPAddressType::kIPv6));
    SuccessOrExit(err);

    err = gSessionManager.Init(&chip::DeviceLayer::SystemLayer(), &gTransportManager, &gMessageCounterManager, &gStorage,
                               &gFabricTable);
    SuccessOrExit(err);

    err = gExchangeManager.Init(&gSessionManager);
    SuccessOrExit(err);

    err = gMessageCounterManager.Init(&gExchangeManager);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeManager, &gFabricTable);
    SuccessOrExit(err);

    err = InitializeEventLogging(&gExchangeManager);
    SuccessOrExit(err);

    err = gSessionManager.InjectPaseSessionWithTestKey(gSession, 1, chip::kTestControllerNodeId, 1, gFabricIndex, peer,
                                                       chip::CryptoContext::SessionRole::kResponder);
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
