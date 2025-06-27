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
#include <app/MessageDef/CommandDataIB.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-cluster.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/Optional.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <messaging/ExchangeContext.h>

namespace chip {
namespace app {

class MockCommandHandler : public CommandHandler
{
public:
    ~MockCommandHandler() override {}

    struct ResponseRecord
    {
        ConcreteCommandPath path;
        CommandId commandId;
        chip::System::PacketBufferHandle encodedData;
    };

    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath & aRequestCommandPath,
                                 const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                                 const char * context = nullptr) override
    {
        mStatuses.push_back({ aRequestCommandPath, aStatus });
        return CHIP_NO_ERROR;
    }

    void AddStatus(const ConcreteCommandPath & aRequestCommandPath, const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                   const char * context = nullptr) override
    {
        CHIP_ERROR err = FallibleAddStatus(aRequestCommandPath, aStatus, context);
        VerifyOrDie(err == CHIP_NO_ERROR);
    }

    CHIP_ERROR AddClusterSpecificSuccess(const ConcreteCommandPath & aRequestCommandPath, ClusterStatus aClusterStatus) override
    {
        return FallibleAddStatus(aRequestCommandPath,
                                 Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificSuccess(aClusterStatus));
    }

    CHIP_ERROR AddClusterSpecificFailure(const ConcreteCommandPath & aRequestCommandPath, ClusterStatus aClusterStatus) override
    {
        return FallibleAddStatus(aRequestCommandPath,
                                 Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(aClusterStatus));
    }

    FabricIndex GetAccessingFabricIndex() const override { return mFabricIndex; }

    const std::vector<ResponseRecord> & GetResponses() const { return mResponses; }

    CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                               const DataModel::EncodableToTLV & aEncodable) override
    {
        chip::System::PacketBufferHandle handle = chip::MessagePacketBuffer::New(1024);
        VerifyOrReturnError(!handle.IsNull(), CHIP_ERROR_NO_MEMORY);

        TLV::TLVWriter baseWriter;
        baseWriter.Init(handle->Start(), handle->MaxDataLength());

        FabricIndex fabricIndex = 1;

        DataModel::FabricAwareTLVWriter writer(baseWriter, fabricIndex);

        TLV::TLVType containerType;
        ReturnErrorOnFailure(
            static_cast<TLV::TLVWriter &>(writer).StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
        ReturnErrorOnFailure(aEncodable.EncodeTo(writer, TLV::ContextTag(chip::app::CommandDataIB::Tag::kFields)));
        ReturnErrorOnFailure(static_cast<TLV::TLVWriter &>(writer).EndContainer(containerType));

        handle->SetDataLength(static_cast<TLV::TLVWriter &>(writer).GetLengthWritten());

        mResponses.push_back({ aRequestCommandPath, aResponseCommandId, std::move(handle) });
        return CHIP_NO_ERROR;
    }

    void AddResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                     const DataModel::EncodableToTLV & aEncodable) override
    {
        AddResponseData(aRequestCommandPath, aResponseCommandId, aEncodable);
    }

    bool IsTimedInvoke() const override { return mIsTimedInvoke; }

    void FlushAcksRightAwayOnSlowCommand() override { mAcksFlushed = true; }

    Access::SubjectDescriptor GetSubjectDescriptor() const override { return mSubjectDescriptor; }

    Messaging::ExchangeContext * GetExchangeContext() const override { return mExchangeContext; }

    // Optional for test configuration
    void SetFabricIndex(FabricIndex index) { mFabricIndex = index; }
    void SetTimedInvoke(bool isTimed) { mIsTimedInvoke = isTimed; }
    void SetExchangeContext(Messaging::ExchangeContext * context) { mExchangeContext = context; }

private:
    struct StatusRecord
    {
        ConcreteCommandPath path;
        Protocols::InteractionModel::ClusterStatusCode status;
    };

    std::vector<ResponseRecord> mResponses;
    std::vector<StatusRecord> mStatuses;

    FabricIndex mFabricIndex                      = 0;
    bool mIsTimedInvoke                           = false;
    bool mAcksFlushed                             = false;
    Messaging::ExchangeContext * mExchangeContext = nullptr;
    Access::SubjectDescriptor mSubjectDescriptor;
};

static uint8_t gDebugEventBuffer[120];
static uint8_t gInfoEventBuffer[120];
static uint8_t gCritEventBuffer[120];
static chip::app::CircularEventBuffer gCircularEventBuffer[3];

class MockEventLogging : public chip::Test::AppContext
{
public:
    void SetUp() override
    {
        const chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        AppContext::SetUp();

        ASSERT_EQ(mEventCounter.Init(0), CHIP_NO_ERROR);

        chip::app::EventManagement::CreateEventManagement(&GetExchangeManager(), std::size(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, &mEventCounter);
    }

    void TearDown() override
    {
        chip::app::EventManagement::DestroyEventManagement();
        AppContext::TearDown();
    }

private:
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
};

static void CheckLogState(chip::app::EventManagement & aLogMgmt, size_t expectedNumEvents, chip::app::PriorityLevel aPriority)
{
    chip::TLV::TLVReader reader;
    size_t elementCount;
    chip::app::CircularEventBufferWrapper bufWrapper;
    EXPECT_EQ(aLogMgmt.GetEventReader(reader, aPriority, &bufWrapper), CHIP_NO_ERROR);

    EXPECT_EQ(chip::TLV::Utilities::Count(reader, elementCount, false), CHIP_NO_ERROR);

    EXPECT_EQ(elementCount, expectedNumEvents);
}

} // namespace app
} // namespace chip

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

struct PushAvStream
{
    uint16_t id;
    TransportOptionsStruct transportOptions;
    TransportStatusEnum transportStatus;
    PushAvStreamTransportStatusEnum connectionStatus;
};

} // namespace PushAvStreamTransport
} // namespace Clusters
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

using namespace chip::Protocols::InteractionModel;

class TestPushAVStreamTransportDelegateImpl : public PushAvStreamTransportDelegate
{
public:
    Protocols::InteractionModel::Status AllocatePushTransport(const TransportOptionsStruct & transportOptions,
                                                              const uint16_t connectionID)
    {
        PushAvStream stream{ connectionID, transportOptions, TransportStatusEnum::kInactive,
                             PushAvStreamTransportStatusEnum::kIdle };

        /*Store the allocated stream persistently*/
        pushavStreams.push_back(stream);

        return Status::Success;
    }

    Protocols::InteractionModel::Status DeallocatePushTransport(const uint16_t connectionID)
    {
        pushavStreams.erase(std::remove_if(pushavStreams.begin(), pushavStreams.end(),
                                           [connectionID](const PushAvStream & stream) { return stream.id == connectionID; }),
                            pushavStreams.end());
        return Status::Success;
    }

    Protocols::InteractionModel::Status ModifyPushTransport(const uint16_t connectionID,
                                                            const TransportOptionsStorage transportOptions)
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
                                                           TransportStatusEnum transportStatus)
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
                             const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl)
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

    Protocols::InteractionModel::Status ValidateBandwidthLimit(StreamUsageEnum streamUsage,
                                                               const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                                               const Optional<DataModel::Nullable<uint16_t>> & audioStreamId)
    {
        // TODO: Validates the requested stream usage against the camera's resource management.
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    bool ValidateUrl(std::string url) { return true; }

    Protocols::InteractionModel::Status SelectVideoStream(StreamUsageEnum streamUsage, uint16_t & videoStreamId)
    {
        // TODO: Select and Assign videoStreamID from the allocated videoStreams
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    Protocols::InteractionModel::Status SelectAudioStream(StreamUsageEnum streamUsage, uint16_t & audioStreamId)
    {
        // TODO: Select and Assign audioStreamID from the allocated audioStreams
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    Protocols::InteractionModel::Status ValidateVideoStream(uint16_t videoStreamId)
    {
        // TODO: Validate videoStreamID from the allocated videoStreams
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    Protocols::InteractionModel::Status ValidateAudioStream(uint16_t audioStreamId)
    {
        // TODO: Validate audioStreamID from the allocated audioStreams
        // Returning Status::Success to pass through checks in the Server Implementation.
        return Status::Success;
    }

    PushAvStreamTransportStatusEnum GetTransportBusyStatus(const uint16_t connectionID)
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

    void OnAttributeChanged(AttributeId attributeId)
    {
        ChipLogProgress(Zcl, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
    }

    void Init() { ChipLogProgress(Zcl, "Push AV Stream Transport Initialized"); }
    CHIP_ERROR
    LoadCurrentConnections(std::vector<TransportConfigurationStorage> & currentConnections)
    {
        ChipLogProgress(Zcl, "Push AV Current Connections loaded");

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR
    PersistentAttributesLoadedCallback()
    {
        ChipLogProgress(Zcl, "Persistent attributes loaded");

        return CHIP_NO_ERROR;
    }

private:
    std::vector<Clusters::PushAvStreamTransport::PushAvStream> pushavStreams;
};

class TestPushAVStreamTransportServerLogic : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestPushAVStreamTransportServerLogic, TestTransportOptionsConstraints)
{
    PushAvStreamTransportServerLogic logic(1, BitFlags<Feature>(0));

    TestPushAVStreamTransportDelegateImpl mockDelegate;
    logic.SetDelegate(1, &mockDelegate);

    // Create CMAFContainerOptionsStruct object
    CMAFContainerOptionsStruct cmafContainerOptions;
    cmafContainerOptions.chunkDuration = 1000;
    cmafContainerOptions.metadataEnabled.SetValue(true);
    std::string cencKey   = "1234567890";
    std::string cencKeyID = "1234567890";
    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    // Create ContainerOptionsStruct object
    ContainerOptionsStruct containerOptions;
    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    // Create a TransportMotionTriggerTimeControlStruct object
    TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;

    motionTimeControl.initialDuration      = 5000;
    motionTimeControl.augmentationDuration = 2000;
    motionTimeControl.maxDuration          = 30000;
    motionTimeControl.blindDuration        = 1000;

    std::vector<TransportZoneOptionsDecodableStruct> mTransportZoneOptions;

    TransportTriggerOptionsDecodableStruct triggerOptions;
    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    triggerOptions.motionZones.SetValue(
        DataModel::Nullable<DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType>>());
    triggerOptions.motionZones.Value().SetNull();
    triggerOptions.motionSensitivity.SetValue(8);
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    // Create TransportOptionsStruct object
    TransportOptionsDecodableStruct transportOptions;
    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(1);
    transportOptions.audioStreamID.SetValue(2);
    transportOptions.endpointID       = 1;
    transportOptions.url              = "rtsp://192.168.1.100:554/stream";
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.SetValue(1000);

    EXPECT_EQ(logic.ValidateIncomingTransportOptions(transportOptions), Status::ConstraintError);
}

void PrintBufHex(const uint8_t * buf, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        printf("%02X ", buf[i]); // Print each byte as 2-digit hex

        // Optional: print a newline every 16 bytes
        if ((i + 1) % 16 == 0)
            printf("\n");
    }

    // Add final newline if buffer didn’t end on a 16-byte boundary
    if (size % 16 != 0)
        printf("\n");
}

TEST_F(TestPushAVStreamTransportServerLogic, TestCurrentConnectionsAttributeAccess)
{
    PushAvStreamTransportServer server(1, BitFlags<Feature>(1));

    // Create CMAFContainerOptionsStruct object
    CMAFContainerOptionsStruct cmafContainerOptions;
    cmafContainerOptions.chunkDuration = 1000;
    cmafContainerOptions.metadataEnabled.ClearValue();
    std::string cencKey   = "deadbeefdeadbeef";
    std::string cencKeyID = "dadabeefdadabeef";
    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    // Create ContainerOptionsStruct object
    ContainerOptionsStruct containerOptions;
    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    // Create a TransportMotionTriggerTimeControlStruct object
    TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;

    motionTimeControl.initialDuration      = 5000;
    motionTimeControl.augmentationDuration = 2000;
    motionTimeControl.maxDuration          = 30000;
    motionTimeControl.blindDuration        = 1000;

    TransportTriggerOptionsDecodableStruct triggerOptions;
    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    // Create transport zone options structs
    Structs::TransportZoneOptionsStruct::Type zone1;
    zone1.zone.SetNonNull(1);
    zone1.sensitivity.SetValue(5);

    Structs::TransportZoneOptionsStruct::Type zone2;
    zone2.zone.SetNonNull(2);
    zone2.sensitivity.SetValue(10);

    // Encode them into a TLV buffer
    uint8_t tlvBuffer[512];
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

    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> decodedList;
    err = decodedList.Decode(motionZonesReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(decodedList));
    triggerOptions.motionSensitivity.ClearValue();
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    // Create TransportOptionsStruct object
    TransportOptionsDecodableStruct transportOptions;
    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(1);
    transportOptions.audioStreamID.SetValue(2);
    transportOptions.endpointID       = 1;
    std::string url                   = "rtsp://192.168.1.100:554/stream";
    transportOptions.url              = Span(url.data(), url.size());
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.ingestMethod     = IngestMethodsEnum::kCMAFIngest;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.SetValue(1000);

    EXPECT_EQ(server.GetLogic().ValidateIncomingTransportOptions(transportOptions), Status::Success);

    std::shared_ptr<TransportOptionsStorage> transportOptionsPtr{ new (std::nothrow) TransportOptionsStorage(transportOptions) };

    EXPECT_NE(transportOptionsPtr, nullptr);

    uint16_t connectionID = 1;

    TransportConfigurationStorage transportConfiguration(connectionID, transportOptionsPtr);

    FabricIndex peerFabricIndex = 1;

    transportConfiguration.SetFabricIndex(peerFabricIndex);

    transportConfiguration.transportStatus = TransportStatusEnum::kInactive;

    server.GetLogic().mCurrentConnections.push_back(transportConfiguration);

    // Test reading current connections through attribute reader

    uint8_t buf[1024];

    TLV::TLVWriter tlvWriter;
    tlvWriter.Init(buf);

    AttributeReportIBs::Builder builder;
    builder.Init(&tlvWriter);

    ConcreteAttributePath path(1, Clusters::PushAvStreamTransport::Id,
                               Clusters::PushAvStreamTransport::Attributes::CurrentConnections::Id);

    DataModel::ReadAttributeRequest request;
    request.path = path;
    request.readFlags.Set(DataModel::ReadFlags::kFabricFiltered);
    chip::DataVersion dataVersion(0);
    Access::SubjectDescriptor subjectDescriptor;
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

    reader.Next();
    reader.OpenContainer(attrReportsReader);

    attrReportsReader.Next();
    attrReportsReader.OpenContainer(attrReportReader);

    attrReportReader.Next();
    attrReportReader.OpenContainer(attrDataReader);

    // We're now in the attribute data IB, skip to the desired tag, we want TagNum = 2
    attrDataReader.Next();
    for (int i = 0; i < 3 && !(IsContextTag(attrDataReader.GetTag()) && TagNumFromTag(attrDataReader.GetTag()) == 2); ++i)
    {
        attrDataReader.Next();
    }
    EXPECT_TRUE(IsContextTag(attrDataReader.GetTag()));
    EXPECT_EQ(TagNumFromTag(attrDataReader.GetTag()), 2u);

    Clusters::PushAvStreamTransport::Attributes::CurrentConnections::TypeInfo::DecodableType currentConnections;
    err = currentConnections.Decode(attrDataReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    auto iter = currentConnections.begin();
    EXPECT_TRUE(iter.Next());
    Structs::TransportConfigurationStruct::DecodableType readTransportConfiguration = iter.GetValue();
    EXPECT_EQ(readTransportConfiguration.connectionID, 1);
    EXPECT_EQ(readTransportConfiguration.transportStatus, TransportStatusEnum::kInactive);
    EXPECT_TRUE(readTransportConfiguration.transportOptions.HasValue());
    Structs::TransportOptionsStruct::DecodableType readTransportOptions = readTransportConfiguration.transportOptions.Value();
    EXPECT_EQ(readTransportOptions.streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(readTransportOptions.videoStreamID, 1);
    EXPECT_EQ(readTransportOptions.audioStreamID, 2);
    EXPECT_EQ(readTransportOptions.endpointID, 1);

    std::string urlStr(readTransportOptions.url.data(), readTransportOptions.url.size());
    EXPECT_EQ(urlStr, "rtsp://192.168.1.100:554/stream");

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
    EXPECT_EQ(readCMAFContainerOptions.chunkDuration, 1000);
    EXPECT_FALSE(readCMAFContainerOptions.metadataEnabled.HasValue());

    std::string cencKeyStr(readCMAFContainerOptions.CENCKey.Value().data(),
                           readCMAFContainerOptions.CENCKey.Value().data() + readCMAFContainerOptions.CENCKey.Value().size());

    EXPECT_EQ(cencKeyStr, "deadbeefdeadbeef");

    std::string cencKeyIDStr(readCMAFContainerOptions.CENCKeyID.Value().data(),
                             readCMAFContainerOptions.CENCKeyID.Value().data() + readCMAFContainerOptions.CENCKeyID.Value().size());

    EXPECT_EQ(cencKeyIDStr, "dadabeefdadabeef");
}

TEST_F(TestPushAVStreamTransportServerLogic, AllocatePushTransport)
{
    PushAvStreamTransportServer server(1, BitFlags<Feature>(1));
    TestPushAVStreamTransportDelegateImpl mockDelegate;
    server.GetLogic().SetDelegate(1, &mockDelegate);

    // Create CMAFContainerOptionsStruct object
    CMAFContainerOptionsStruct cmafContainerOptions;
    cmafContainerOptions.chunkDuration = 1000;
    cmafContainerOptions.metadataEnabled.ClearValue();
    std::string cencKey   = "deadbeefdeadbeef";
    std::string cencKeyID = "dadabeefdadabeef";
    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    // Create ContainerOptionsStruct object
    ContainerOptionsStruct containerOptions;
    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    // Create a TransportMotionTriggerTimeControlStruct object
    TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;

    motionTimeControl.initialDuration      = 5000;
    motionTimeControl.augmentationDuration = 2000;
    motionTimeControl.maxDuration          = 30000;
    motionTimeControl.blindDuration        = 1000;

    TransportTriggerOptionsDecodableStruct triggerOptions;
    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    // Create transport zone options structs
    Structs::TransportZoneOptionsStruct::Type zone1;
    zone1.zone.SetNonNull(1);
    zone1.sensitivity.SetValue(5);

    Structs::TransportZoneOptionsStruct::Type zone2;
    zone2.zone.SetNonNull(2);
    zone2.sensitivity.SetValue(10);

    // Encode them into a TLV buffer
    uint8_t tlvBuffer[512];
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
    TLV::TLVReader reader;
    reader.Init(tlvBuffer, static_cast<uint32_t>(encodedLen));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> decodedList;
    err = decodedList.Decode(reader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(decodedList));
    triggerOptions.motionSensitivity.ClearValue();
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    // Create TransportOptionsStruct object
    TransportOptionsDecodableStruct transportOptions;
    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(1);
    transportOptions.audioStreamID.SetValue(2);
    transportOptions.endpointID       = 1;
    std::string url                   = "rtsp://192.168.1.100:554/stream";
    transportOptions.url              = Span(url.data(), url.size());
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.ingestMethod     = IngestMethodsEnum::kCMAFIngest;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.ClearValue();

    MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::AllocatePushTransport::Id };
    Commands::AllocatePushTransport::DecodableType commandData;
    commandData.transportOptions = transportOptions;

    // Without a delegate, command is unsupported.
    EXPECT_EQ(server.GetLogic().HandleAllocatePushTransport(commandHandler, kCommandPath, commandData), std::nullopt);

    EXPECT_EQ(server.GetLogic().mCurrentConnections.size(), (size_t) 1);

    TransportConfigurationStorage transportConfiguration = server.GetLogic().mCurrentConnections[0];
    EXPECT_EQ(transportConfiguration.connectionID, 1);
    EXPECT_EQ(transportConfiguration.GetFabricIndex(), 1);

    Structs::TransportOptionsStruct::Type readTransportOptions = transportConfiguration.transportOptions.Value();
    EXPECT_EQ(readTransportOptions.streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(readTransportOptions.videoStreamID, 1);
    EXPECT_EQ(readTransportOptions.audioStreamID, 2);
    EXPECT_EQ(readTransportOptions.endpointID, 1);
    std::string urlStr(readTransportOptions.url.data(), readTransportOptions.url.size());
    EXPECT_EQ(urlStr, "rtsp://192.168.1.100:554/stream");

    Structs::TransportTriggerOptionsStruct::Type readTriggerOptions = readTransportOptions.triggerOptions;
    EXPECT_EQ(readTriggerOptions.triggerType, TransportTriggerTypeEnum::kMotion);
    EXPECT_TRUE(readTriggerOptions.motionZones.HasValue());
    EXPECT_FALSE(readTriggerOptions.motionZones.Value().IsNull());

    DataModel::List<const Structs::TransportZoneOptionsStruct::Type> & motionZonesList =
        readTriggerOptions.motionZones.Value().Value();

    EXPECT_EQ(motionZonesList.size(), (size_t) 2);

    Structs::TransportZoneOptionsStruct::Type motionZone1 = motionZonesList[0];
    Structs::TransportZoneOptionsStruct::Type motionZone2 = motionZonesList[1];

    EXPECT_FALSE(motionZone1.zone.IsNull());
    EXPECT_EQ(motionZone1.zone.Value(), 1);
    EXPECT_TRUE(motionZone1.sensitivity.HasValue());
    EXPECT_EQ(motionZone1.sensitivity.Value(), 5);

    EXPECT_FALSE(motionZone2.zone.IsNull());
    EXPECT_EQ(motionZone2.zone.Value(), 2);
    EXPECT_TRUE(motionZone2.sensitivity.HasValue());
    EXPECT_EQ(motionZone2.sensitivity.Value(), 10);

    EXPECT_FALSE(readTriggerOptions.motionSensitivity.HasValue());
    EXPECT_TRUE(readTriggerOptions.maxPreRollLen.HasValue());
    EXPECT_EQ(readTriggerOptions.maxPreRollLen.Value(), (uint32_t) 1000);
}

TEST_F(TestPushAVStreamTransportServerLogic, AllocatePushTransportResponse)
{
    PushAvStreamTransportServer server(1, BitFlags<Feature>(1));
    TestPushAVStreamTransportDelegateImpl mockDelegate;
    server.GetLogic().SetDelegate(1, &mockDelegate);

    // Create CMAFContainerOptionsStruct object
    CMAFContainerOptionsStruct cmafContainerOptions;
    cmafContainerOptions.chunkDuration = 1000;
    cmafContainerOptions.metadataEnabled.ClearValue();
    std::string cencKey   = "deadbeefdeadbeef";
    std::string cencKeyID = "dadabeefdadabeef";
    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    // Create ContainerOptionsStruct object
    ContainerOptionsStruct containerOptions;
    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    // Create a TransportMotionTriggerTimeControlStruct object
    TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;

    motionTimeControl.initialDuration      = 5000;
    motionTimeControl.augmentationDuration = 2000;
    motionTimeControl.maxDuration          = 30000;
    motionTimeControl.blindDuration        = 1000;

    TransportTriggerOptionsDecodableStruct triggerOptions;
    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    // Create transport zone options structs
    Structs::TransportZoneOptionsStruct::Type zone1;
    zone1.zone.SetNonNull(1);
    zone1.sensitivity.SetValue(5);

    Structs::TransportZoneOptionsStruct::Type zone2;
    zone2.zone.SetNonNull(2);
    zone2.sensitivity.SetValue(10);

    // Encode them into a TLV buffer
    uint8_t tlvBuffer[512];
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
    TLV::TLVReader reader;
    reader.Init(tlvBuffer, static_cast<uint32_t>(encodedLen));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> decodedList;
    err = decodedList.Decode(reader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(decodedList));
    triggerOptions.motionSensitivity.ClearValue();
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    // Create TransportOptionsStruct object
    TransportOptionsDecodableStruct transportOptions;
    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(1);
    transportOptions.audioStreamID.SetValue(2);
    transportOptions.endpointID       = 1;
    std::string url                   = "rtsp://192.168.1.100:554/stream";
    transportOptions.url              = Span(url.data(), url.size());
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.ingestMethod     = IngestMethodsEnum::kCMAFIngest;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.ClearValue();

    MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::AllocatePushTransport::Id };
    Commands::AllocatePushTransport::DecodableType commandData;
    commandData.transportOptions = transportOptions;

    // Call the command handler
    server.GetLogic().HandleAllocatePushTransport(commandHandler, kCommandPath, commandData);

    // Check the response
    const auto & responses = commandHandler.GetResponses();
    EXPECT_EQ(responses.size(), (size_t) 1);

    // Get the encoded buffer
    const auto & encodedBuffer = responses[0].encodedData;

    PrintBufHex(encodedBuffer->Start(), encodedBuffer->DataLength());

    EXPECT_FALSE(encodedBuffer.IsNull());

    // Set up TLV reader
    TLV::TLVReader responseReader;
    responseReader.Init(encodedBuffer->Start(), static_cast<uint32_t>(encodedBuffer->DataLength()));

    // Enter the top-level anonymous structure (CommandDataIB wrapper)
    err = responseReader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    TLV::TLVReader outerContainer;
    err = responseReader.OpenContainer(outerContainer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Read the next element inside the container: should be kFields
    err = outerContainer.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_TRUE(IsContextTag(outerContainer.GetTag()));
    EXPECT_EQ(TagNumFromTag(outerContainer.GetTag()), chip::to_underlying(CommandDataIB::Tag::kFields));

    // Decode into response object
    Commands::AllocatePushTransportResponse::DecodableType decodedResponse;
    err = decodedResponse.Decode(outerContainer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Validate decoded fields
    // ConnectionID=2 as connection is being allocated second time.
    EXPECT_EQ(decodedResponse.transportConfiguration.connectionID, 2);
    EXPECT_EQ(decodedResponse.transportConfiguration.GetFabricIndex(), 1);
    EXPECT_EQ(decodedResponse.transportConfiguration.transportStatus, TransportStatusEnum::kInactive);

    EXPECT_TRUE(decodedResponse.transportConfiguration.transportOptions.HasValue());

    Structs::TransportOptionsStruct::DecodableType readTransportOptions =
        decodedResponse.transportConfiguration.transportOptions.Value();

    EXPECT_EQ(readTransportOptions.streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(readTransportOptions.videoStreamID, 1);
    EXPECT_EQ(readTransportOptions.audioStreamID, 2);
    EXPECT_EQ(readTransportOptions.endpointID, 1);
    std::string urlStr(readTransportOptions.url.data(), readTransportOptions.url.size());
    EXPECT_EQ(urlStr, "rtsp://192.168.1.100:554/stream");

    Structs::TransportTriggerOptionsStruct::DecodableType readTriggerOptions = readTransportOptions.triggerOptions;
    EXPECT_EQ(readTriggerOptions.triggerType, TransportTriggerTypeEnum::kMotion);
    EXPECT_TRUE(readTriggerOptions.motionZones.HasValue());
    EXPECT_FALSE(readTriggerOptions.motionZones.Value().IsNull());

    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> & readMotionZonesList =
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
    EXPECT_EQ(readCMAFContainerOptions.chunkDuration, 1000);
    EXPECT_FALSE(readCMAFContainerOptions.metadataEnabled.HasValue());

    std::string cencKeyStr(readCMAFContainerOptions.CENCKey.Value().data(),
                           readCMAFContainerOptions.CENCKey.Value().data() + readCMAFContainerOptions.CENCKey.Value().size());

    EXPECT_EQ(cencKeyStr, "deadbeefdeadbeef");

    std::string cencKeyIDStr(readCMAFContainerOptions.CENCKeyID.Value().data(),
                             readCMAFContainerOptions.CENCKeyID.Value().data() + readCMAFContainerOptions.CENCKeyID.Value().size());

    EXPECT_EQ(cencKeyIDStr, "dadabeefdadabeef");
}

TEST_F(MockEventLogging, ManuallyTriggerTransport)
{
    PushAvStreamTransportServer server(1, BitFlags<Feature>(1));
    TestPushAVStreamTransportDelegateImpl mockDelegate;
    server.GetLogic().SetDelegate(1, &mockDelegate);

    // Create CMAFContainerOptionsStruct object
    CMAFContainerOptionsStruct cmafContainerOptions;
    cmafContainerOptions.chunkDuration = 1000;
    cmafContainerOptions.metadataEnabled.ClearValue();
    std::string cencKey   = "deadbeefdeadbeef";
    std::string cencKeyID = "dadabeefdadabeef";
    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    // Create ContainerOptionsStruct object
    ContainerOptionsStruct containerOptions;
    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    // Create a TransportMotionTriggerTimeControlStruct object
    TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;

    motionTimeControl.initialDuration      = 5000;
    motionTimeControl.augmentationDuration = 2000;
    motionTimeControl.maxDuration          = 30000;
    motionTimeControl.blindDuration        = 1000;

    TransportTriggerOptionsDecodableStruct triggerOptions;
    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    // Create transport zone options structs
    Structs::TransportZoneOptionsStruct::Type zone1;
    zone1.zone.SetNonNull(1);
    zone1.sensitivity.SetValue(5);

    Structs::TransportZoneOptionsStruct::Type zone2;
    zone2.zone.SetNonNull(2);
    zone2.sensitivity.SetValue(10);

    // Encode them into a TLV buffer
    uint8_t tlvBuffer[512];
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
    TLV::TLVReader reader;
    reader.Init(tlvBuffer, static_cast<uint32_t>(encodedLen));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> decodedList;
    err = decodedList.Decode(reader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(decodedList));
    triggerOptions.motionSensitivity.ClearValue();
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    // Create TransportOptionsStruct object
    TransportOptionsDecodableStruct transportOptions;
    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(1);
    transportOptions.audioStreamID.SetValue(2);
    transportOptions.endpointID       = 1;
    std::string url                   = "rtsp://192.168.1.100:554/stream";
    transportOptions.url              = Span(url.data(), url.size());
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.ingestMethod     = IngestMethodsEnum::kCMAFIngest;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.ClearValue();

    MockCommandHandler commandHandler;
    commandHandler.SetFabricIndex(1);
    ConcreteCommandPath kCommandPath1{ 1, Clusters::PushAvStreamTransport::Id, Commands::AllocatePushTransport::Id };
    Commands::AllocatePushTransport::DecodableType commandData1;
    commandData1.transportOptions = transportOptions;

    server.GetLogic().HandleAllocatePushTransport(commandHandler, kCommandPath1, commandData1);

    ConcreteCommandPath kCommandPath3{ 1, Clusters::PushAvStreamTransport::Id, Commands::SetTransportStatus::Id };
    Commands::SetTransportStatus::DecodableType commandData3;
    commandData3.connectionID.SetNonNull(3);
    commandData3.transportStatus = TransportStatusEnum::kActive;
    server.GetLogic().HandleSetTransportStatus(commandHandler, kCommandPath3, commandData3);

    // Status status = server.GetLogic().GeneratePushTransportBeginEvent(1, TransportTriggerTypeEnum::kMotion,
    //                                                                   MakeOptional(TriggerActivationReasonEnum::kUserInitiated));
    // EXPECT_EQ(status, Status::Success);

    ConcreteCommandPath kCommandPath2{ 1, Clusters::PushAvStreamTransport::Id, Commands::ManuallyTriggerTransport::Id };
    Commands::ManuallyTriggerTransport::DecodableType commandData2;
    commandData2.connectionID     = 3;
    commandData2.activationReason = TriggerActivationReasonEnum::kUserInitiated;

    server.GetLogic().HandleManuallyTriggerTransport(commandHandler, kCommandPath2, commandData2);

    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();

    CheckLogState(logMgmt, 1, chip::app::PriorityLevel::Info);
}

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
