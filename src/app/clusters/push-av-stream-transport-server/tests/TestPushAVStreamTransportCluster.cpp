/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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
#include <vector>

#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportCluster.h>
#include <app/clusters/tls-client-management-server/TLSClientManagementCluster.h>
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/Optional.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>

namespace chip {
namespace app {

static uint8_t gDebugEventBuffer[120];
static uint8_t gInfoEventBuffer[120];
static uint8_t gCritEventBuffer[120];
static CircularEventBuffer gCircularEventBuffer[3];

class MockEventLogging : public Testing::AppContext
{
public:
    void SetUp() override
    {
        const LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), PriorityLevel::Critical },
        };

        AppContext::SetUp();

        ASSERT_EQ(mEventCounter.Init(0), CHIP_NO_ERROR);

        EventManagement::CreateEventManagement(&GetExchangeManager(), std::size(logStorageResources), gCircularEventBuffer,
                                               logStorageResources, &mEventCounter);
    }

    void TearDown() override
    {
        EventManagement::DestroyEventManagement();
        AppContext::TearDown();
    }

private:
    MonotonicallyIncreasingCounter<EventNumber> mEventCounter;
};

static void CheckLogState(EventManagement & aLogMgmt, size_t expectedNumEvents, PriorityLevel aPriority)
{
    TLV::TLVReader reader;
    size_t elementCount;
    CircularEventBufferWrapper bufWrapper;
    EXPECT_EQ(aLogMgmt.GetEventReader(reader, aPriority, &bufWrapper), CHIP_NO_ERROR);

    EXPECT_EQ(TLV::Utilities::Count(reader, elementCount, false), CHIP_NO_ERROR);

    EXPECT_EQ(elementCount, expectedNumEvents);
}
} // namespace app
} // namespace chip

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

using TransportZoneOptionsDecodableStruct              = Structs::TransportZoneOptionsStruct::DecodableType;
using TransportTriggerOptionsDecodableStruct           = Structs::TransportTriggerOptionsStruct::DecodableType;
using TransportMotionTriggerTimeControlDecodableStruct = Structs::TransportMotionTriggerTimeControlStruct::DecodableType;
using TransportOptionsDecodableStruct                  = Structs::TransportOptionsStruct::DecodableType;

using namespace Protocols::InteractionModel;

struct PushAvStream
{
    uint16_t id;
    TransportOptionsStruct transportOptions;
    TransportStatusEnum transportStatus;
    PushAvStreamTransportStatusEnum connectionStatus;
};
class TestPushAVStreamTransportDelegateImpl : public PushAvStreamTransportDelegate
{
public:
    Protocols::InteractionModel::Status AllocatePushTransport(const TransportOptionsStruct & transportOptions,
                                                              const uint16_t connectionID,
                                                              FabricIndex accessingFabricIndex) override
    {
        PushAvStream stream{ connectionID, transportOptions, TransportStatusEnum::kInactive,
                             PushAvStreamTransportStatusEnum::kIdle };

        /*Store the allocated stream persistently*/
        pushavStreams.push_back(stream);

        return Status::Success;
    }

    Protocols::InteractionModel::Status DeallocatePushTransport(const uint16_t connectionID) override
    {
        pushavStreams.erase(std::remove_if(pushavStreams.begin(), pushavStreams.end(),
                                           [connectionID](const PushAvStream & stream) { return stream.id == connectionID; }),
                            pushavStreams.end());
        ChipLogProgress(Zcl, "Deallocated Push AV Stream with ID: %d", connectionID);
        return Status::Success;
    }

    Protocols::InteractionModel::Status ModifyPushTransport(const uint16_t connectionID,
                                                            const TransportOptionsStorage transportOptions) override
    {
        for (PushAvStream & stream : pushavStreams)
        {
            if (stream.id == connectionID)
            {
                ChipLogProgress(Zcl, "Modified Push AV Stream with ID: %d", connectionID);
                return Status::Success;
            }
        }
        ChipLogError(Zcl, "Allocated Push AV Stream with ID: %d not found", connectionID);
        return Status::NotFound;
    }

    Protocols::InteractionModel::Status SetTransportStatus(const std::vector<uint16_t> connectionIDList,
                                                           TransportStatusEnum transportStatus) override
    {
        for (PushAvStream & stream : pushavStreams)
        {
            for (uint16_t connectionID : connectionIDList)
            {
                if (stream.id == connectionID)
                {
                    stream.transportStatus = transportStatus;
                    ChipLogProgress(Zcl, "Set Transport Status for Push AV Stream with ID: %d", connectionID);
                }
            }
        }
        return Status::Success;
    }

    Protocols::InteractionModel::Status
    ManuallyTriggerTransport(const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
                             const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl) override
    {
        // TODO: Validates the requested stream usage against the camera's resource management and stream priority policies.
        for (PushAvStream & stream : pushavStreams)
        {
            if (stream.id == connectionID)
            {
                stream.connectionStatus = PushAvStreamTransportStatusEnum::kBusy;
                ChipLogProgress(Zcl, "Transport triggered for Push AV Stream with ID: %d", connectionID);
            }
        }
        return Status::Success;
    }

    bool ValidateStreamUsage(StreamUsageEnum streamUsage) override { return true; }

    bool ValidateSegmentDuration(uint16_t segmentDuration, const Optional<DataModel::Nullable<uint16_t>> & videoStreamId) override
    {
        return true;
    }

    bool ValidateMaxPreRollLength(uint16_t maxPreRollLength, const DataModel::Nullable<uint16_t> & videoStreamId) override
    {
        return true;
    }

    Protocols::InteractionModel::Status
    ValidateBandwidthLimit(StreamUsageEnum streamUsage, const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                           const Optional<DataModel::Nullable<uint16_t>> & audioStreamId) override
    {
        // TODO: Validates the requested stream usage against the camera's resource management.
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    Protocols::InteractionModel::Status SelectVideoStream(StreamUsageEnum streamUsage, uint16_t & videoStreamId) override
    {
        // TODO: Select and Assign videoStreamID from the allocated videoStreams
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    Protocols::InteractionModel::Status SelectAudioStream(StreamUsageEnum streamUsage, uint16_t & audioStreamId) override
    {
        // TODO: Select and Assign audioStreamID from the allocated audioStreams
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    Protocols::InteractionModel::Status SetVideoStream(uint16_t videoStreamId) override
    {
        // TODO: Validate videoStreamID from the allocated videoStreams
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    Protocols::InteractionModel::Status SetAudioStream(uint16_t audioStreamId) override
    {
        // TODO: Validate audioStreamID from the allocated audioStreams
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    Protocols::InteractionModel::Status ValidateZoneId(uint16_t zoneId) override
    {
        // TODO: Validate zoneId from the allocated zones
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    bool ValidateMotionZoneListSize(size_t zoneListSize) override
    {
        // TODO: Validate motion zone size
        // Returning true to pass through checks in the Server Implementation.
        return true;
    }

    PushAvStreamTransportStatusEnum GetTransportBusyStatus(const uint16_t connectionID) override
    {
        for (PushAvStream & stream : pushavStreams)
        {
            if (stream.id == connectionID)
            {
                return stream.connectionStatus;
            }
        }
        return PushAvStreamTransportStatusEnum::kUnknown;
    }

    CHIP_ERROR IsHardPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR IsSoftRecordingPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR IsSoftLivestreamPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    void OnAttributeChanged(AttributeId attributeId) override
    {
        ChipLogProgress(Zcl, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
    }

    void Init() { ChipLogProgress(Zcl, "Push AV Stream Transport Initialized"); }

    CHIP_ERROR LoadCurrentConnections(std::vector<TransportConfigurationStorage> & currentConnections) override
    {
        ChipLogProgress(Zcl, "Push AV Current Connections loaded");
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR PersistentAttributesLoadedCallback() override
    {
        ChipLogProgress(Zcl, "Persistent attributes loaded");
        return CHIP_NO_ERROR;
    }

    void SetTLSCerts(Tls::CertificateTable::BufferedClientCert & clientCertEntry,
                     Tls::CertificateTable::BufferedRootCert & rootCertEntry) override
    {
        // No-op implementation for tests
    }

    void SetPushAvStreamTransportServer(PushAvStreamTransportServer * server) override
    {
        // No-op implementation for tests
    }
    bool GetCMAFSessionNumber(const uint16_t connectionID, uint64_t & sessionNumber) override
    {
        // Mock implementation for tests - return a simple session number
        sessionNumber = static_cast<uint64_t>(connectionID) + 2000;
        return true;
    }

private:
    std::vector<Clusters::PushAvStreamTransport::PushAvStream> pushavStreams;
};

class TestTLSClientManagementDelegate : public TLSClientManagementDelegate
{

public:
    CHIP_ERROR Init(PersistentStorageDelegate & storage) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ForEachEndpoint(EndpointId matterEndpoint, FabricIndex fabric, LoadedEndpointCallback callback) override
    {
        return CHIP_NO_ERROR;
    }

    Protocols::InteractionModel::ClusterStatusCode
    ProvisionEndpoint(EndpointId matterEndpoint, FabricIndex fabric,
                      const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType & provisionReq,
                      uint16_t & endpointID) override
    {
        return ClusterStatusCode(Status::Success);
    }

    CHIP_ERROR FindProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric, uint16_t endpointID,
                                           LoadedEndpointCallback callback) override
    {
        return CHIP_NO_ERROR;
    }

    Protocols::InteractionModel::Status RemoveProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                      uint16_t endpointID) override
    {
        return Status::Success;
    }

    CHIP_ERROR RootCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ClientCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCCDID id) override
    {
        return CHIP_NO_ERROR;
    }

    void RemoveFabric(FabricIndex fabric) override {}

    CHIP_ERROR MutateEndpointReferenceCount(EndpointId matterEndpoint, FabricIndex fabric, uint16_t endpointID,
                                            int8_t delta) override
    {
        return CHIP_NO_ERROR;
    }
};

class TestPushAVStreamTransportServerLogic : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestPushAVStreamTransportServerLogic, TestTransportOptionsConstraints)
{
    std::string cencKey   = "1234567890ABCDEF";
    std::string cencKeyID = "1234567890ABCDEF";

    CMAFContainerOptionsStruct cmafContainerOptions;
    ContainerOptionsStruct containerOptions;
    TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;
    TransportTriggerOptionsDecodableStruct triggerOptions;

    std::string url = "https://192.168.1.100:554/stream/";
    TransportOptionsDecodableStruct transportOptions;

    uint8_t tlvBuffer[512];
    Structs::TransportZoneOptionsStruct::Type zone1;
    Structs::TransportZoneOptionsStruct::Type zone2;
    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> decodedList;

    PushAvStreamTransportServerLogic logic(1, BitFlags<Feature>(1));

    // Create CMAFContainerOptionsStruct object
    cmafContainerOptions.chunkDuration = 1000;
    cmafContainerOptions.metadataEnabled.ClearValue();

    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    // Create ContainerOptionsStruct object
    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    // Create a TransportMotionTriggerTimeControlStruct object
    motionTimeControl.initialDuration      = 5000;
    motionTimeControl.augmentationDuration = 2000;
    motionTimeControl.maxDuration          = 30000;
    motionTimeControl.blindDuration        = 1000;

    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    triggerOptions.motionZones.ClearValue();

    triggerOptions.motionSensitivity.ClearValue();
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    // Create TransportOptionsStruct object
    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(1);
    transportOptions.audioStreamID.SetValue(2);
    transportOptions.TLSEndpointID    = 1;
    transportOptions.url              = Span(url.data(), url.size());
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.ClearValue();

    // Invalid command because the motion zones are missing
    EXPECT_EQ(logic.ValidateIncomingTransportOptions(transportOptions), Status::InvalidCommand);

    // Create transport zone options structs
    zone1.zone.SetNonNull(1);
    zone1.sensitivity.SetValue(5);

    zone2.zone.SetNonNull(2);
    zone2.sensitivity.SetValue(10);

    // Encode them into a TLV buffer
    TLV::TLVWriter writer;
    writer.Init(tlvBuffer, sizeof(tlvBuffer));

    TLV::TLVWriter containerWriter;
    CHIP_ERROR err;

    err = writer.OpenContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, containerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), zone1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), zone2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.CloseContainer(containerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    size_t encodedLen = writer.GetLengthWritten();

    // Decode the TLV into a DecodableList
    TLV::TLVReader motionZonesReader;
    motionZonesReader.Init(tlvBuffer, static_cast<uint32_t>(encodedLen));
    err = motionZonesReader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = decodedList.Decode(motionZonesReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(decodedList));

    // Upadate the trigger options in the transport options
    transportOptions.triggerOptions = triggerOptions;
    EXPECT_EQ(logic.ValidateIncomingTransportOptions(transportOptions),
              Status::ConstraintError); // ConstraintError because segmentDuration is not set

    cmafContainerOptions.segmentDuration = 1000;
    cmafContainerOptions.chunkDuration   = 500;
    std::string trackName                = "video";
    cmafContainerOptions.trackName       = Span(trackName.data(), trackName.size());
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);
    transportOptions.containerOptions = containerOptions;
    EXPECT_EQ(logic.ValidateIncomingTransportOptions(transportOptions), Status::Success);
}

void PrintBufHex(const uint8_t * buf, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        printf("%02X ", buf[i]);

        if ((i + 1) % 16 == 0)
            printf("\n");
    }

    if (size % 16 != 0)
        printf("\n");
}

TEST_F(TestPushAVStreamTransportServerLogic, Test_AllocateTransport_AllocateTransportResponse_ReadAttribute_DeallocateTransport)
{
    /*
     * Test AllocatePushTransport
     */
    std::string cencKey   = "1234567890ABCDEF";
    std::string cencKeyID = "1234567890ABCDEF";

    CMAFContainerOptionsStruct cmafContainerOptions;
    ContainerOptionsStruct containerOptions;
    TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;
    TransportTriggerOptionsDecodableStruct triggerOptions;

    std::string url = "https://192.168.1.100:554/stream/";
    TransportOptionsDecodableStruct transportOptions;

    uint8_t tlvBuffer[512];
    Structs::TransportZoneOptionsStruct::Type zone1;
    Structs::TransportZoneOptionsStruct::Type zone2;
    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> decodedList;

    // Create CMAFContainerOptionsStruct object
    cmafContainerOptions.segmentDuration = 1000;
    cmafContainerOptions.chunkDuration   = 500;
    std::string trackName                = "video";
    cmafContainerOptions.trackName       = Span(trackName.data(), trackName.size());
    cmafContainerOptions.metadataEnabled.ClearValue();

    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    // Create ContainerOptionsStruct object
    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    // Create a TransportMotionTriggerTimeControlStruct object
    motionTimeControl.initialDuration      = 5000;
    motionTimeControl.augmentationDuration = 2000;
    motionTimeControl.maxDuration          = 30000;
    motionTimeControl.blindDuration        = 1000;

    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    // Create transport zone options structs
    zone1.zone.SetNonNull(1);
    zone1.sensitivity.SetValue(5);

    zone2.zone.SetNonNull(2);
    zone2.sensitivity.SetValue(10);

    // Encode them into a TLV buffer
    TLV::TLVWriter writer;
    writer.Init(tlvBuffer, sizeof(tlvBuffer));

    TLV::TLVWriter containerWriter;
    CHIP_ERROR err;

    err = writer.OpenContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, containerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), zone1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), zone2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.CloseContainer(containerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    size_t encodedLen = writer.GetLengthWritten();

    // Decode the TLV into a DecodableList
    TLV::TLVReader motionZonesReader;
    motionZonesReader.Init(tlvBuffer, static_cast<uint32_t>(encodedLen));
    err = motionZonesReader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = decodedList.Decode(motionZonesReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(decodedList));

    triggerOptions.motionSensitivity.ClearValue();
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    // Create TransportOptionsStruct object
    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(1);
    transportOptions.audioStreamID.SetValue(2);
    transportOptions.TLSEndpointID    = 1;
    transportOptions.url              = Span(url.data(), url.size());
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.ClearValue();

    PushAvStreamTransportServer server(1, BitFlags<Feature>(1));
    TestPushAVStreamTransportDelegateImpl mockDelegate;
    TestTLSClientManagementDelegate tlsClientManagementDelegate;

    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::AllocatePushTransport::Id };
    Commands::AllocatePushTransport::DecodableType commandData;
    commandData.transportOptions = transportOptions;

    // Without a delegate, command is unsupported.
    EXPECT_EQ(server.GetLogic().HandleAllocatePushTransport(commandHandler, kCommandPath, commandData), std::nullopt);

    // Set the delegate to the server logic
    server.GetLogic().SetDelegate(&mockDelegate);
    server.GetLogic().SetTLSClientManagementDelegate(&tlsClientManagementDelegate);
    EXPECT_EQ(server.GetLogic().HandleAllocatePushTransport(commandHandler, kCommandPath, commandData), std::nullopt);

    EXPECT_EQ(server.GetLogic().mCurrentConnections.size(), (size_t) 1);
    uint16_t allocatedConnectionID = server.GetLogic().mCurrentConnections[0].connectionID;

    /*
     * Test AllocatePushTransportResponse
     */

    // Check the response
    EXPECT_TRUE(commandHandler.HasResponse());
    EXPECT_EQ(commandHandler.GetResponseCount(), (size_t) 1);

    // Decode response using MockCommandHandler helper
    Commands::AllocatePushTransportResponse::DecodableType decodedResponse;
    err = commandHandler.DecodeResponse(decodedResponse);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Validate decoded fields
    EXPECT_EQ(decodedResponse.transportConfiguration.connectionID, allocatedConnectionID);
    EXPECT_EQ(decodedResponse.transportConfiguration.GetFabricIndex(), 1);
    EXPECT_EQ(decodedResponse.transportConfiguration.transportStatus, TransportStatusEnum::kInactive);

    EXPECT_TRUE(decodedResponse.transportConfiguration.transportOptions.HasValue());

    Structs::TransportOptionsStruct::DecodableType respTransportOptions =
        decodedResponse.transportConfiguration.transportOptions.Value();

    EXPECT_EQ(respTransportOptions.streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(respTransportOptions.videoStreamID, 1);
    EXPECT_EQ(respTransportOptions.audioStreamID, 2);
    EXPECT_EQ(respTransportOptions.TLSEndpointID, 1);
    std::string respUrlStr(respTransportOptions.url.data(), respTransportOptions.url.size());
    EXPECT_EQ(respUrlStr, "https://192.168.1.100:554/stream/");

    Structs::TransportTriggerOptionsStruct::DecodableType respTriggerOptions = respTransportOptions.triggerOptions;
    EXPECT_EQ(respTriggerOptions.triggerType, TransportTriggerTypeEnum::kMotion);
    EXPECT_TRUE(respTriggerOptions.motionZones.HasValue());
    EXPECT_FALSE(respTriggerOptions.motionZones.Value().IsNull());

    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> & respMotionZonesList =
        respTriggerOptions.motionZones.Value().Value();

    auto respMotionZonesIter = respMotionZonesList.begin();

    EXPECT_TRUE(respMotionZonesIter.Next());
    const auto & respDecodedZone1 = respMotionZonesIter.GetValue();
    EXPECT_TRUE(!respDecodedZone1.zone.IsNull());
    EXPECT_EQ(respDecodedZone1.zone.Value(), 1);
    EXPECT_TRUE(respDecodedZone1.sensitivity.HasValue());
    EXPECT_EQ(respDecodedZone1.sensitivity.Value(), 5);

    EXPECT_TRUE(respMotionZonesIter.Next());
    const auto & respDecodedZone2 = respMotionZonesIter.GetValue();
    EXPECT_TRUE(!respDecodedZone2.zone.IsNull());
    EXPECT_EQ(respDecodedZone2.zone.Value(), 2);
    EXPECT_TRUE(respDecodedZone2.sensitivity.HasValue());
    EXPECT_EQ(respDecodedZone2.sensitivity.Value(), 10);

    // Should be no more entries
    EXPECT_FALSE(respMotionZonesIter.Next());

    EXPECT_FALSE(respTriggerOptions.motionSensitivity.HasValue());
    EXPECT_TRUE(respTriggerOptions.maxPreRollLen.HasValue());
    EXPECT_EQ(respTriggerOptions.maxPreRollLen.Value(), 1000);

    EXPECT_TRUE(respTriggerOptions.motionTimeControl.HasValue());
    Structs::TransportMotionTriggerTimeControlStruct::DecodableType respMotionTimeControl =
        respTriggerOptions.motionTimeControl.Value();
    EXPECT_EQ(respMotionTimeControl.initialDuration, 5000);
    EXPECT_EQ(respMotionTimeControl.augmentationDuration, 2000);
    EXPECT_EQ(respMotionTimeControl.maxDuration, (uint32_t) 30000);
    EXPECT_EQ(respMotionTimeControl.blindDuration, 1000);

    EXPECT_EQ(respTransportOptions.ingestMethod, IngestMethodsEnum::kCMAFIngest);

    Structs::ContainerOptionsStruct::DecodableType respContainerOptions = respTransportOptions.containerOptions;
    EXPECT_EQ(respContainerOptions.containerType, ContainerFormatEnum::kCmaf);
    EXPECT_TRUE(respContainerOptions.CMAFContainerOptions.HasValue());
    Structs::CMAFContainerOptionsStruct::Type respCMAFContainerOptions = respContainerOptions.CMAFContainerOptions.Value();
    EXPECT_EQ(respCMAFContainerOptions.segmentDuration, 1000);
    EXPECT_EQ(respCMAFContainerOptions.chunkDuration, 500);
    std::string respTrackName(respCMAFContainerOptions.trackName.data(), respCMAFContainerOptions.trackName.size());
    EXPECT_EQ(respTrackName, "video");
    EXPECT_FALSE(respCMAFContainerOptions.metadataEnabled.HasValue());

    std::string respCENCKeyStr(respCMAFContainerOptions.CENCKey.Value().data(),
                               respCMAFContainerOptions.CENCKey.Value().data() + respCMAFContainerOptions.CENCKey.Value().size());

    EXPECT_EQ(respCENCKeyStr, "1234567890ABCDEF");

    std::string respCENCKeyIDStr(respCMAFContainerOptions.CENCKeyID.Value().data(),
                                 respCMAFContainerOptions.CENCKeyID.Value().data() +
                                     respCMAFContainerOptions.CENCKeyID.Value().size());

    EXPECT_EQ(respCENCKeyIDStr, "1234567890ABCDEF");

    /*
     * Test ReadAttribute
     */
    // Test reading current connections through attribute reader
    uint8_t buf[1024];

    TLV::TLVWriter tlvWriter;
    tlvWriter.Init(buf);

    AttributeReportIBs::Builder builder;
    EXPECT_SUCCESS(builder.Init(&tlvWriter));

    ConcreteAttributePath path(1, Clusters::PushAvStreamTransport::Id,
                               Clusters::PushAvStreamTransport::Attributes::CurrentConnections::Id);

    DataModel::ReadAttributeRequest request;
    request.path = path;
    request.readFlags.Set(DataModel::ReadFlags::kFabricFiltered);
    DataVersion dataVersion(0);
    Access::SubjectDescriptor subjectDescriptor;
    FabricIndex peerFabricIndex   = 1;
    subjectDescriptor.fabricIndex = peerFabricIndex;
    AttributeValueEncoder encoder(builder, subjectDescriptor, path, dataVersion, true);

    // Read the CurrentConnections attribute using the cluster's Read function
    DataModel::ActionReturnStatus status = server.ReadAttribute(request, encoder);
    EXPECT_TRUE(status.IsSuccess());

    TLV::TLVReader reader;
    reader.Init(buf);

    PrintBufHex(buf, tlvWriter.GetLengthWritten());

    TLV::TLVReader attrReportsReader;
    TLV::TLVReader attrReportReader;
    TLV::TLVReader attrDataReader;

    EXPECT_SUCCESS(reader.Next());
    EXPECT_SUCCESS(reader.OpenContainer(attrReportsReader));

    EXPECT_SUCCESS(attrReportsReader.Next());
    EXPECT_SUCCESS(attrReportsReader.OpenContainer(attrReportReader));

    EXPECT_SUCCESS(attrReportReader.Next());
    EXPECT_SUCCESS(attrReportReader.OpenContainer(attrDataReader));

    // We're now in the attribute data IB, skip to the desired tag, we want TagNum = 2
    EXPECT_SUCCESS(attrDataReader.Next());
    for (int i = 0; i < 3 && !(IsContextTag(attrDataReader.GetTag()) && TagNumFromTag(attrDataReader.GetTag()) == 2); ++i)
    {
        EXPECT_SUCCESS(attrDataReader.Next());
    }
    EXPECT_TRUE(IsContextTag(attrDataReader.GetTag()));
    EXPECT_EQ(TagNumFromTag(attrDataReader.GetTag()), 2u);

    Clusters::PushAvStreamTransport::Attributes::CurrentConnections::TypeInfo::DecodableType currentConnections;
    err = currentConnections.Decode(attrDataReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    auto iter = currentConnections.begin();
    EXPECT_TRUE(iter.Next());
    Structs::TransportConfigurationStruct::DecodableType readTransportConfiguration = iter.GetValue();
    EXPECT_EQ(readTransportConfiguration.connectionID, allocatedConnectionID);
    EXPECT_EQ(readTransportConfiguration.transportStatus, TransportStatusEnum::kInactive);
    EXPECT_TRUE(readTransportConfiguration.transportOptions.HasValue());
    Structs::TransportOptionsStruct::DecodableType readTransportOptions = readTransportConfiguration.transportOptions.Value();
    EXPECT_EQ(readTransportOptions.streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(readTransportOptions.videoStreamID, 1);
    EXPECT_EQ(readTransportOptions.audioStreamID, 2);
    EXPECT_EQ(readTransportOptions.TLSEndpointID, 1);

    std::string urlStr(readTransportOptions.url.data(), readTransportOptions.url.size());
    EXPECT_EQ(urlStr, "https://192.168.1.100:554/stream/");

    Structs::TransportTriggerOptionsStruct::DecodableType readTriggerOptions = readTransportOptions.triggerOptions;
    EXPECT_EQ(readTriggerOptions.triggerType, TransportTriggerTypeEnum::kMotion);
    EXPECT_TRUE(readTriggerOptions.motionZones.HasValue());
    EXPECT_FALSE(readTriggerOptions.motionZones.Value().IsNull());

    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> readMotionZonesList =
        readTriggerOptions.motionZones.Value().Value();

    auto readMotionZonesIter = readMotionZonesList.begin();

    EXPECT_TRUE(readMotionZonesIter.Next());
    const auto & decodedZone1 = readMotionZonesIter.GetValue();
    EXPECT_TRUE(!decodedZone1.zone.IsNull());
    EXPECT_EQ(decodedZone1.zone.Value(), 1);
    EXPECT_TRUE(decodedZone1.sensitivity.HasValue());
    EXPECT_EQ(decodedZone1.sensitivity.Value(), 5);

    EXPECT_TRUE(readMotionZonesIter.Next());
    const auto & decodedZone2 = readMotionZonesIter.GetValue();
    EXPECT_TRUE(!decodedZone2.zone.IsNull());
    EXPECT_EQ(decodedZone2.zone.Value(), 2);
    EXPECT_TRUE(decodedZone2.sensitivity.HasValue());
    EXPECT_EQ(decodedZone2.sensitivity.Value(), 10);

    // Should be no more entries
    EXPECT_FALSE(readMotionZonesIter.Next());

    EXPECT_FALSE(readTriggerOptions.motionSensitivity.HasValue());
    EXPECT_TRUE(readTriggerOptions.maxPreRollLen.HasValue());
    EXPECT_EQ(readTriggerOptions.maxPreRollLen.Value(), 1000);

    EXPECT_TRUE(readTriggerOptions.motionTimeControl.HasValue());
    Structs::TransportMotionTriggerTimeControlStruct::DecodableType readMotionTimeControl =
        readTriggerOptions.motionTimeControl.Value();
    EXPECT_EQ(readMotionTimeControl.initialDuration, 5000);
    EXPECT_EQ(readMotionTimeControl.augmentationDuration, 2000);
    EXPECT_EQ(readMotionTimeControl.maxDuration, (uint32_t) 30000);
    EXPECT_EQ(readMotionTimeControl.blindDuration, 1000);

    EXPECT_EQ(readTransportOptions.ingestMethod, IngestMethodsEnum::kCMAFIngest);

    Structs::ContainerOptionsStruct::DecodableType readContainerOptions = readTransportOptions.containerOptions;
    EXPECT_EQ(readContainerOptions.containerType, ContainerFormatEnum::kCmaf);
    EXPECT_TRUE(readContainerOptions.CMAFContainerOptions.HasValue());
    Structs::CMAFContainerOptionsStruct::Type readCMAFContainerOptions = readContainerOptions.CMAFContainerOptions.Value();
    EXPECT_EQ(readCMAFContainerOptions.segmentDuration, 1000);
    EXPECT_EQ(readCMAFContainerOptions.chunkDuration, 500);
    std::string readTrackName(readCMAFContainerOptions.trackName.data(), readCMAFContainerOptions.trackName.size());
    EXPECT_EQ(readTrackName, "video");
    EXPECT_FALSE(readCMAFContainerOptions.metadataEnabled.HasValue());

    std::string cencKeyStr(readCMAFContainerOptions.CENCKey.Value().data(),
                           readCMAFContainerOptions.CENCKey.Value().data() + readCMAFContainerOptions.CENCKey.Value().size());

    EXPECT_EQ(cencKeyStr, "1234567890ABCDEF");

    std::string cencKeyIDStr(readCMAFContainerOptions.CENCKeyID.Value().data(),
                             readCMAFContainerOptions.CENCKeyID.Value().data() + readCMAFContainerOptions.CENCKeyID.Value().size());

    EXPECT_EQ(cencKeyIDStr, "1234567890ABCDEF");

    /*
     * Test DeallocatePushTransport
     */
    Testing::MockCommandHandler deallocateCommandHandler;
    deallocateCommandHandler.SetFabricIndex(1);
    ConcreteCommandPath kDeallocateCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::DeallocatePushTransport::Id };
    Commands::DeallocatePushTransport::DecodableType deallocateCommandData;
    deallocateCommandData.connectionID = allocatedConnectionID;

    EXPECT_EQ(
        server.GetLogic().HandleDeallocatePushTransport(deallocateCommandHandler, kDeallocateCommandPath, deallocateCommandData),
        std::nullopt);

    EXPECT_EQ(server.GetLogic().mCurrentConnections.size(), (size_t) 0);
}

TEST_F(MockEventLogging, Test_AllocateTransport_ModifyTransport_FindTransport_FindTransportResponse)
{
    /*
     * Test AllocatePushTransport
     */
    std::string cencKey   = "1234567890ABCDEF";
    std::string cencKeyID = "1234567890ABCDEF";

    CMAFContainerOptionsStruct cmafContainerOptions;
    ContainerOptionsStruct containerOptions;
    TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;
    TransportTriggerOptionsDecodableStruct triggerOptions;

    std::string url = "https://192.168.1.100:554/stream/";
    TransportOptionsDecodableStruct transportOptions;

    uint8_t tlvBuffer[512];
    Structs::TransportZoneOptionsStruct::Type zone1;
    Structs::TransportZoneOptionsStruct::Type zone2;
    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> decodedList;

    // Create CMAFContainerOptionsStruct object
    cmafContainerOptions.segmentDuration = 1000;
    cmafContainerOptions.chunkDuration   = 500;
    std::string trackName                = "video";
    cmafContainerOptions.trackName       = Span(trackName.data(), trackName.size());
    cmafContainerOptions.metadataEnabled.ClearValue();

    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    // Create ContainerOptionsStruct object
    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    // Create a TransportMotionTriggerTimeControlStruct object
    motionTimeControl.initialDuration      = 5000;
    motionTimeControl.augmentationDuration = 2000;
    motionTimeControl.maxDuration          = 30000;
    motionTimeControl.blindDuration        = 1000;

    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    // Create transport zone options structs
    zone1.zone.SetNonNull(1);
    zone1.sensitivity.SetValue(5);

    zone2.zone.SetNonNull(2);
    zone2.sensitivity.SetValue(10);

    // Encode them into a TLV buffer
    TLV::TLVWriter writer;
    writer.Init(tlvBuffer, sizeof(tlvBuffer));

    TLV::TLVWriter containerWriter;
    CHIP_ERROR err;

    err = writer.OpenContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, containerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), zone1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), zone2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.CloseContainer(containerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    size_t encodedLen = writer.GetLengthWritten();

    // Decode the TLV into a DecodableList
    TLV::TLVReader motionZonesReader;
    motionZonesReader.Init(tlvBuffer, static_cast<uint32_t>(encodedLen));
    err = motionZonesReader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = decodedList.Decode(motionZonesReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(decodedList));

    triggerOptions.motionSensitivity.ClearValue();
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    // Create TransportOptionsStruct object
    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(1);
    transportOptions.audioStreamID.SetValue(2);
    transportOptions.TLSEndpointID    = 1;
    transportOptions.url              = Span(url.data(), url.size());
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.ClearValue();

    PushAvStreamTransportServer server(1, BitFlags<Feature>(1));
    TestPushAVStreamTransportDelegateImpl mockDelegate;
    TestTLSClientManagementDelegate tlsClientManagementDelegate;

    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::AllocatePushTransport::Id };
    Commands::AllocatePushTransport::DecodableType commandData;
    commandData.transportOptions = transportOptions;

    // Set the delegate to the server logic
    server.GetLogic().SetDelegate(&mockDelegate);
    server.GetLogic().SetTLSClientManagementDelegate(&tlsClientManagementDelegate);
    EXPECT_EQ(server.GetLogic().HandleAllocatePushTransport(commandHandler, kCommandPath, commandData), std::nullopt);

    EXPECT_EQ(server.GetLogic().mCurrentConnections.size(), (size_t) 1);
    uint16_t allocatedConnectionID = server.GetLogic().mCurrentConnections[0].connectionID;

    /*
     * Test ModifyPushTransport
     */

    // Create CMAFContainerOptionsStruct object
    cmafContainerOptions.segmentDuration = 30000;
    cmafContainerOptions.chunkDuration   = 1000;
    cmafContainerOptions.metadataEnabled.ClearValue();

    cencKey   = "ABCDEF1234567890";
    cencKeyID = "ABCDEF1234567890";

    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    // Create ContainerOptionsStruct object
    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    // Create a TransportMotionTriggerTimeControlStruct object
    motionTimeControl.initialDuration      = 1000;
    motionTimeControl.augmentationDuration = 1000;
    motionTimeControl.maxDuration          = 10000;
    motionTimeControl.blindDuration        = 1000;

    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    // Create transport zone options structs
    zone1.zone.SetNonNull(7);
    zone1.sensitivity.SetValue(8);

    zone2.zone.SetNonNull(9);
    zone2.sensitivity.SetValue(6);

    // Encode them into a TLV buffer
    TLV::TLVWriter modifyTLVWriter;
    modifyTLVWriter.Init(tlvBuffer, sizeof(tlvBuffer));

    TLV::TLVWriter modifyContainerWriter;

    err = modifyTLVWriter.OpenContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, modifyContainerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(modifyContainerWriter, TLV::AnonymousTag(), zone1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(modifyContainerWriter, TLV::AnonymousTag(), zone2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = modifyTLVWriter.CloseContainer(modifyContainerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    size_t modifyEncodedLen = modifyTLVWriter.GetLengthWritten();

    // Decode the TLV into a DecodableList
    TLV::TLVReader modifyMotionZonesReader;
    modifyMotionZonesReader.Init(tlvBuffer, static_cast<uint32_t>(modifyEncodedLen));
    err = modifyMotionZonesReader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> modifyDecodedList;

    err = modifyDecodedList.Decode(modifyMotionZonesReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(modifyDecodedList));

    triggerOptions.motionSensitivity.ClearValue();
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    // Create TransportOptionsStruct object
    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(11);
    transportOptions.audioStreamID.SetValue(22);
    transportOptions.TLSEndpointID    = 1;
    url                               = "https://192.168.1.100:554/modify-stream/";
    transportOptions.url              = Span(url.data(), url.size());
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.ClearValue();

    Testing::MockCommandHandler modifyCommandHandler;
    modifyCommandHandler.SetFabricIndex(1);

    ConcreteCommandPath kModifyCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::ModifyPushTransport::Id };
    Commands::ModifyPushTransport::DecodableType modifyCommandData;
    modifyCommandData.connectionID     = allocatedConnectionID;
    modifyCommandData.transportOptions = transportOptions;

    server.GetLogic().HandleModifyPushTransport(modifyCommandHandler, kModifyCommandPath, modifyCommandData);

    EXPECT_EQ(server.GetLogic().mCurrentConnections.size(), (size_t) 1);

    /*
     * Test FindPushTransport
     */

    Testing::MockCommandHandler findCommandHandler;
    findCommandHandler.SetFabricIndex(1);

    ConcreteCommandPath kFindCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::FindTransport::Id };
    Commands::FindTransport::DecodableType findCommandData;
    // As connectionID is static, the new allocated connectionID will be 2.
    findCommandData.connectionID.SetNonNull(allocatedConnectionID);

    server.GetLogic().HandleFindTransport(findCommandHandler, kFindCommandPath, findCommandData);

    // Check the response
    EXPECT_TRUE(findCommandHandler.HasResponse());
    EXPECT_EQ(findCommandHandler.GetResponseCount(), (size_t) 1);

    // Decode response using MockCommandHandler helper
    Commands::FindTransportResponse::DecodableType decodedResponse;
    err = findCommandHandler.DecodeResponse(decodedResponse);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    auto iter = decodedResponse.transportConfigurations.begin();

    EXPECT_TRUE(iter.Next());

    Structs::TransportConfigurationStruct::DecodableType readTransportConfiguration = iter.GetValue();
    EXPECT_EQ(readTransportConfiguration.connectionID, allocatedConnectionID);
    EXPECT_EQ(readTransportConfiguration.transportStatus, TransportStatusEnum::kInactive);

    EXPECT_TRUE(readTransportConfiguration.transportOptions.HasValue());
    Structs::TransportOptionsStruct::DecodableType findTransportOptions = readTransportConfiguration.transportOptions.Value();
    EXPECT_EQ(findTransportOptions.streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(findTransportOptions.videoStreamID, 11);
    EXPECT_EQ(findTransportOptions.audioStreamID, 22);
    EXPECT_EQ(findTransportOptions.TLSEndpointID, 1);

    std::string findUrlStr(findTransportOptions.url.data(), findTransportOptions.url.size());
    EXPECT_EQ(findUrlStr, "https://192.168.1.100:554/modify-stream/");

    Structs::TransportTriggerOptionsStruct::DecodableType findTriggerOptions = findTransportOptions.triggerOptions;
    EXPECT_EQ(findTriggerOptions.triggerType, TransportTriggerTypeEnum::kMotion);
    EXPECT_TRUE(findTriggerOptions.motionZones.HasValue());
    EXPECT_FALSE(findTriggerOptions.motionZones.Value().IsNull());

    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> findMotionZonesList =
        findTriggerOptions.motionZones.Value().Value();

    auto findMotionZonesIter = findMotionZonesList.begin();

    EXPECT_TRUE(findMotionZonesIter.Next());
    const auto & decodedZone1 = findMotionZonesIter.GetValue();
    EXPECT_TRUE(!decodedZone1.zone.IsNull());
    EXPECT_EQ(decodedZone1.zone.Value(), 7);
    EXPECT_TRUE(decodedZone1.sensitivity.HasValue());
    EXPECT_EQ(decodedZone1.sensitivity.Value(), 8);

    EXPECT_TRUE(findMotionZonesIter.Next());
    const auto & decodedZone2 = findMotionZonesIter.GetValue();
    EXPECT_TRUE(!decodedZone2.zone.IsNull());
    EXPECT_EQ(decodedZone2.zone.Value(), 9);
    EXPECT_TRUE(decodedZone2.sensitivity.HasValue());
    EXPECT_EQ(decodedZone2.sensitivity.Value(), 6);

    // Should be no more entries
    EXPECT_FALSE(findMotionZonesIter.Next());

    EXPECT_FALSE(findTriggerOptions.motionSensitivity.HasValue());
    EXPECT_TRUE(findTriggerOptions.maxPreRollLen.HasValue());
    EXPECT_EQ(findTriggerOptions.maxPreRollLen.Value(), 1000);

    EXPECT_TRUE(findTriggerOptions.motionTimeControl.HasValue());
    Structs::TransportMotionTriggerTimeControlStruct::DecodableType findMotionTimeControl =
        findTriggerOptions.motionTimeControl.Value();
    EXPECT_EQ(findMotionTimeControl.initialDuration, 1000);
    EXPECT_EQ(findMotionTimeControl.augmentationDuration, 1000);
    EXPECT_EQ(findMotionTimeControl.maxDuration, (uint32_t) 10000);
    EXPECT_EQ(findMotionTimeControl.blindDuration, 1000);

    EXPECT_EQ(findTransportOptions.ingestMethod, IngestMethodsEnum::kCMAFIngest);

    Structs::ContainerOptionsStruct::DecodableType findContainerOptions = findTransportOptions.containerOptions;
    EXPECT_EQ(findContainerOptions.containerType, ContainerFormatEnum::kCmaf);
    EXPECT_TRUE(findContainerOptions.CMAFContainerOptions.HasValue());
    Structs::CMAFContainerOptionsStruct::Type findCMAFContainerOptions = findContainerOptions.CMAFContainerOptions.Value();
    EXPECT_EQ(findCMAFContainerOptions.segmentDuration, 30000);
    EXPECT_EQ(findCMAFContainerOptions.chunkDuration, 1000);
    EXPECT_FALSE(findCMAFContainerOptions.metadataEnabled.HasValue());

    std::string cencKeyStr(findCMAFContainerOptions.CENCKey.Value().data(),
                           findCMAFContainerOptions.CENCKey.Value().data() + findCMAFContainerOptions.CENCKey.Value().size());

    EXPECT_EQ(cencKeyStr, "ABCDEF1234567890");

    std::string cencKeyIDStr(findCMAFContainerOptions.CENCKeyID.Value().data(),
                             findCMAFContainerOptions.CENCKeyID.Value().data() + findCMAFContainerOptions.CENCKeyID.Value().size());

    EXPECT_EQ(cencKeyIDStr, "ABCDEF1234567890");

    EXPECT_FALSE(iter.Next());
}

TEST_F(MockEventLogging, Test_AllocateTransport_SetTransportStatus_ManuallyTriggerTransport)
{
    std::string cencKey   = "1234567890ABCDEF";
    std::string cencKeyID = "1234567890ABCDEF";

    CMAFContainerOptionsStruct cmafContainerOptions;
    ContainerOptionsStruct containerOptions;
    TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;
    TransportTriggerOptionsDecodableStruct triggerOptions;

    std::string url = "https://192.168.1.100:554/stream/";
    TransportOptionsDecodableStruct transportOptions;

    uint8_t tlvBuffer[512];
    Structs::TransportZoneOptionsStruct::Type zone1;
    Structs::TransportZoneOptionsStruct::Type zone2;
    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> decodedList;

    // Create CMAFContainerOptionsStruct object
    cmafContainerOptions.segmentDuration = 1000;
    cmafContainerOptions.chunkDuration   = 500;
    std::string trackName                = "video";
    cmafContainerOptions.trackName       = Span(trackName.data(), trackName.size());
    cmafContainerOptions.metadataEnabled.ClearValue();

    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    // Create ContainerOptionsStruct object
    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    // Create a TransportMotionTriggerTimeControlStruct object
    motionTimeControl.initialDuration      = 5000;
    motionTimeControl.augmentationDuration = 2000;
    motionTimeControl.maxDuration          = 30000;
    motionTimeControl.blindDuration        = 1000;

    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    // Create transport zone options structs
    zone1.zone.SetNonNull(1);
    zone1.sensitivity.SetValue(5);

    zone2.zone.SetNonNull(2);
    zone2.sensitivity.SetValue(10);

    // Encode them into a TLV buffer
    TLV::TLVWriter writer;
    writer.Init(tlvBuffer, sizeof(tlvBuffer));

    TLV::TLVWriter containerWriter;
    CHIP_ERROR err;

    err = writer.OpenContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, containerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), zone1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), zone2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.CloseContainer(containerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    size_t encodedLen = writer.GetLengthWritten();

    // Decode the TLV into a DecodableList
    TLV::TLVReader motionZonesReader;
    motionZonesReader.Init(tlvBuffer, static_cast<uint32_t>(encodedLen));
    err = motionZonesReader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = decodedList.Decode(motionZonesReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(decodedList));

    triggerOptions.motionSensitivity.ClearValue();
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    // Create TransportOptionsStruct object
    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(1);
    transportOptions.audioStreamID.SetValue(2);
    transportOptions.TLSEndpointID    = 1;
    transportOptions.url              = Span(url.data(), url.size());
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.ClearValue();

    PushAvStreamTransportServer server(1, BitFlags<Feature>(1));
    TestPushAVStreamTransportDelegateImpl mockDelegate;
    TestTLSClientManagementDelegate tlsClientManagementDelegate;

    Testing::MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::AllocatePushTransport::Id };
    Commands::AllocatePushTransport::DecodableType commandData;
    commandData.transportOptions = transportOptions;

    server.GetLogic().SetDelegate(&mockDelegate);
    server.GetLogic().SetTLSClientManagementDelegate(&tlsClientManagementDelegate);
    EXPECT_EQ(server.GetLogic().HandleAllocatePushTransport(commandHandler, kCommandPath, commandData), std::nullopt);
    EXPECT_EQ(server.GetLogic().mCurrentConnections.size(), (size_t) 1);

    uint16_t allocatedConnectionID = server.GetLogic().mCurrentConnections[0].connectionID;

    /*
     * Test SetTransportStatus
     */

    Testing::MockCommandHandler setCommandHandler;
    setCommandHandler.SetFabricIndex(1);

    ConcreteCommandPath kSetCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::SetTransportStatus::Id };
    Commands::SetTransportStatus::DecodableType setCommandData;

    setCommandData.connectionID.SetNonNull(allocatedConnectionID);
    setCommandData.transportStatus = TransportStatusEnum::kActive;
    server.GetLogic().HandleSetTransportStatus(setCommandHandler, kSetCommandPath, setCommandData);

    EXPECT_EQ(server.GetLogic().mCurrentConnections[0].transportStatus, TransportStatusEnum::kActive);

    Testing::MockCommandHandler triggerCommandHandler;
    triggerCommandHandler.SetFabricIndex(1);

    ConcreteCommandPath kTriggerCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::ManuallyTriggerTransport::Id };
    Commands::ManuallyTriggerTransport::DecodableType triggerCommandData;
    triggerCommandData.connectionID     = allocatedConnectionID;
    triggerCommandData.activationReason = TriggerActivationReasonEnum::kUserInitiated;

    server.GetLogic().HandleManuallyTriggerTransport(triggerCommandHandler, kTriggerCommandPath, triggerCommandData);

    EventManagement & logMgmt = EventManagement::GetInstance();

    CheckLogState(logMgmt, 1, PriorityLevel::Info);
}

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
