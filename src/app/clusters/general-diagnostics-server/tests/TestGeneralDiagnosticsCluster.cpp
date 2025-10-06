/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/MessageDef/CommandDataIB.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-cluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/GeneralDiagnostics/Enums.h>
#include <clusters/GeneralDiagnostics/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <messaging/ExchangeContext.h>
#include <platform/DiagnosticDataProvider.h>

#include <cmath>
#include <vector>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralDiagnostics::Attributes;
using namespace chip::app::DataModel;

template <class T>
class ScopedDiagnosticsProvider
{
public:
    ScopedDiagnosticsProvider()
    {
        mOldProvider = &DeviceLayer::GetDiagnosticDataProvider();
        DeviceLayer::SetDiagnosticDataProvider(&mProvider);
    }
    ~ScopedDiagnosticsProvider() { DeviceLayer::SetDiagnosticDataProvider(mOldProvider); }

    ScopedDiagnosticsProvider(const ScopedDiagnosticsProvider &)             = delete;
    ScopedDiagnosticsProvider & operator=(const ScopedDiagnosticsProvider &) = delete;
    ScopedDiagnosticsProvider(ScopedDiagnosticsProvider &&)                  = delete;
    ScopedDiagnosticsProvider & operator=(ScopedDiagnosticsProvider &&)      = delete;

private:
    DeviceLayer::DiagnosticDataProvider * mOldProvider;
    T mProvider;
};

// Mock DiagnosticDataProvider for testing
class NullProvider : public DeviceLayer::DiagnosticDataProvider
{
};

struct TestGeneralDiagnosticsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    // Small buffer size sufficient for encoding an empty TLV structure
    static constexpr size_t kEmptyCommandTLVBufferSize = 32;

    // Helper method to setup a TLV reader with an empty structure for TimeSnapshot command
    static void SetupEmptyCommandTLV(uint8_t (&buffer)[kEmptyCommandTLVBufferSize], TLV::TLVReader & reader)
    {
        // Create a TLV writer and initialize it with the buffer
        TLV::TLVWriter writer;
        writer.Init(buffer);
        TLV::TLVType outerContainer;

        ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainer), CHIP_NO_ERROR);
        ASSERT_EQ(writer.EndContainer(outerContainer), CHIP_NO_ERROR);
        ASSERT_EQ(writer.Finalize(), CHIP_NO_ERROR);

        // Initialize the reader with the buffer and the length of the written data
        reader.Init(buffer, writer.GetLengthWritten());
        ASSERT_EQ(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()), CHIP_NO_ERROR);
    }
};

// Mock CommandHandler for testing command invocations
class MockCommandHandler : public CommandHandler
{
public:
    ~MockCommandHandler() override {}

    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath & aRequestCommandPath,
                                 const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                                 const char * context = nullptr) override
    {
        return CHIP_NO_ERROR;
    }

    void AddStatus(const ConcreteCommandPath & aRequestCommandPath, const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                   const char * context = nullptr) override
    {
        CHIP_ERROR err = FallibleAddStatus(aRequestCommandPath, aStatus, context);
        VerifyOrDie(err == CHIP_NO_ERROR);
    }

    FabricIndex GetAccessingFabricIndex() const override { return 1; }

    CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                               const DataModel::EncodableToTLV & aEncodable) override
    {
        chip::System::PacketBufferHandle handle = chip::MessagePacketBuffer::New(1024);
        VerifyOrReturnError(!handle.IsNull(), CHIP_ERROR_NO_MEMORY);
        TLV::TLVWriter baseWriter;
        baseWriter.Init(handle->Start(), handle->MaxDataLength());
        DataModel::FabricAwareTLVWriter writer(baseWriter, /*fabricIndex*/ 1);
        TLV::TLVType ct;
        ReturnErrorOnFailure(
            static_cast<TLV::TLVWriter &>(writer).StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, ct));
        ReturnErrorOnFailure(aEncodable.EncodeTo(writer, TLV::ContextTag(app::CommandDataIB::Tag::kFields)));
        ReturnErrorOnFailure(static_cast<TLV::TLVWriter &>(writer).EndContainer(ct));
        handle->SetDataLength(static_cast<TLV::TLVWriter &>(writer).GetLengthWritten());

        mResponseCommandId = aResponseCommandId;
        mEncodedData       = std::move(handle);
        return CHIP_NO_ERROR;
    }

    void AddResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                     const DataModel::EncodableToTLV & aEncodable) override
    {
        (void) AddResponseData(aRequestCommandPath, aResponseCommandId, aEncodable);
    }

    bool IsTimedInvoke() const override { return false; }
    void FlushAcksRightAwayOnSlowCommand() override {}
    Access::SubjectDescriptor GetSubjectDescriptor() const override { return Access::SubjectDescriptor{}; }
    Messaging::ExchangeContext * GetExchangeContext() const override { return nullptr; }

    // Helper methods to extract response data
    bool HasResponse() const { return !mEncodedData.IsNull(); }

    CHIP_ERROR DecodeTimeSnapshotResponse(GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType & response)
    {
        VerifyOrReturnError(!mEncodedData.IsNull(), CHIP_ERROR_INCORRECT_STATE);

        TLV::TLVReader reader;
        reader.Init(mEncodedData->Start(), mEncodedData->DataLength());
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType outerContainer;
        ReturnErrorOnFailure(reader.EnterContainer(outerContainer));
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(app::CommandDataIB::Tag::kFields)));

        return response.Decode(reader);
    }

    CommandId GetResponseCommandId() const { return mResponseCommandId; }

private:
    CommandId mResponseCommandId = 0;
    chip::System::PacketBufferHandle mEncodedData;
};

TEST_F(TestGeneralDiagnosticsCluster, CompileTest)
{
    const GeneralDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet;

    GeneralDiagnosticsCluster cluster(optionalAttributeSet);
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, GeneralDiagnostics::Id }), BitFlags<ClusterQualityFlags>());

    const GeneralDiagnosticsFunctionsConfig functionsConfig{
        .enablePosixTime      = true,
        .enablePayloadSnaphot = true,
    };

    GeneralDiagnosticsClusterFullConfigurable clusterWithTimeAndPayload(optionalAttributeSet, functionsConfig);
    ASSERT_EQ(clusterWithTimeAndPayload.GetClusterFlags({ kRootEndpointId, GeneralDiagnostics::Id }),
              BitFlags<ClusterQualityFlags>());
}

TEST_F(TestGeneralDiagnosticsCluster, AttributesTest)
{
    {
        // everything returns empty here ..
        const GeneralDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet;
        ScopedDiagnosticsProvider<NullProvider> nullProvider;
        GeneralDiagnosticsCluster cluster(optionalAttributeSet);

        // Check required accepted commands are present
        ConcreteClusterPath generalDiagnosticsPath = ConcreteClusterPath(kRootEndpointId, GeneralDiagnostics::Id);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommandsBuilder;
        ASSERT_EQ(cluster.AcceptedCommands(generalDiagnosticsPath, acceptedCommandsBuilder), CHIP_NO_ERROR);
        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> acceptedCommands = acceptedCommandsBuilder.TakeBuffer();
        ASSERT_EQ(acceptedCommands.size(), 2u);

        ASSERT_EQ(acceptedCommands[0].commandId, GeneralDiagnostics::Commands::TestEventTrigger::Id);
        ASSERT_EQ(acceptedCommands[0].GetInvokePrivilege(),
                  GeneralDiagnostics::Commands::TestEventTrigger::kMetadataEntry.GetInvokePrivilege());

        ASSERT_EQ(acceptedCommands[1].commandId, GeneralDiagnostics::Commands::TimeSnapshot::Id);
        ASSERT_EQ(acceptedCommands[1].GetInvokePrivilege(),
                  GeneralDiagnostics::Commands::TimeSnapshot::kMetadataEntry.GetInvokePrivilege());

        // Check required generated commands are present
        ReadOnlyBufferBuilder<chip::CommandId> generatedCommandsBuilder;
        ASSERT_EQ(cluster.GeneratedCommands(generalDiagnosticsPath, generatedCommandsBuilder), CHIP_NO_ERROR);
        ReadOnlyBuffer<chip::CommandId> generatedCommands = generatedCommandsBuilder.TakeBuffer();

        ASSERT_EQ(generatedCommands.size(), 1u);
        ASSERT_EQ(generatedCommands[0], GeneralDiagnostics::Commands::TimeSnapshotResponse::Id);

        // Everything is unimplemented, so attributes are just the global and mandatory ones.
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(cluster.Attributes(generalDiagnosticsPath, attributesBuilder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({
                      GeneralDiagnostics::Attributes::NetworkInterfaces::kMetadataEntry,
                      GeneralDiagnostics::Attributes::RebootCount::kMetadataEntry,
                      GeneralDiagnostics::Attributes::UpTime::kMetadataEntry,
                      GeneralDiagnostics::Attributes::TestEventTriggersEnabled::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    {
        class AllProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override
            {
                rebootCount = 123;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override
            {
                totalOperationalHours = 456;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetBootReason(chip::app::Clusters::GeneralDiagnostics::BootReasonEnum & bootReason) override
            {
                bootReason = GeneralDiagnostics::BootReasonEnum::kSoftwareReset;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR
            GetActiveHardwareFaults(chip::DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & hardwareFaults) override
            {
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetActiveRadioFaults(chip::DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & radioFaults) override
            {
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR
            GetActiveNetworkFaults(chip::DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & networkFaults) override
            {
                return CHIP_NO_ERROR;
            }
        };

        // Enable all the optional attributes
        const GeneralDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet =
            GeneralDiagnosticsCluster::OptionalAttributeSet()
                .Set<TotalOperationalHours::Id>()
                .Set<BootReason::Id>()
                .Set<ActiveHardwareFaults::Id>()
                .Set<ActiveRadioFaults::Id>()
                .Set<ActiveNetworkFaults::Id>();

        ScopedDiagnosticsProvider<AllProvider> nullProvider;
        GeneralDiagnosticsCluster cluster(optionalAttributeSet);

        // Check mandatory commands are present
        ConcreteClusterPath generalDiagnosticsPath = ConcreteClusterPath(kRootEndpointId, GeneralDiagnostics::Id);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(cluster.AcceptedCommands(generalDiagnosticsPath, commandsBuilder), CHIP_NO_ERROR);
        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> commands = commandsBuilder.TakeBuffer();
        ASSERT_EQ(commands.size(), 2u);

        ASSERT_EQ(commands[0].commandId, GeneralDiagnostics::Commands::TestEventTrigger::Id);
        ASSERT_EQ(commands[0].GetInvokePrivilege(),
                  GeneralDiagnostics::Commands::TestEventTrigger::kMetadataEntry.GetInvokePrivilege());

        ASSERT_EQ(commands[1].commandId, GeneralDiagnostics::Commands::TimeSnapshot::Id);
        ASSERT_EQ(commands[1].GetInvokePrivilege(),
                  GeneralDiagnostics::Commands::TimeSnapshot::kMetadataEntry.GetInvokePrivilege());

        // Check required generated commands are present
        ReadOnlyBufferBuilder<chip::CommandId> generatedCommandsBuilder;
        ASSERT_EQ(cluster.GeneratedCommands(generalDiagnosticsPath, generatedCommandsBuilder), CHIP_NO_ERROR);
        ReadOnlyBuffer<chip::CommandId> generatedCommands = generatedCommandsBuilder.TakeBuffer();

        ASSERT_EQ(generatedCommands.size(), 1u);
        ASSERT_EQ(generatedCommands[0], GeneralDiagnostics::Commands::TimeSnapshotResponse::Id);

        // Everything is implemented, so attributes are the global ones and ALL optional ones as well.
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(cluster.Attributes(generalDiagnosticsPath, attributesBuilder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({
                      GeneralDiagnostics::Attributes::NetworkInterfaces::kMetadataEntry,
                      GeneralDiagnostics::Attributes::RebootCount::kMetadataEntry,
                      GeneralDiagnostics::Attributes::UpTime::kMetadataEntry,
                      GeneralDiagnostics::Attributes::TestEventTriggersEnabled::kMetadataEntry,
                      GeneralDiagnostics::Attributes::TotalOperationalHours::kMetadataEntry,
                      GeneralDiagnostics::Attributes::BootReason::kMetadataEntry,
                      GeneralDiagnostics::Attributes::ActiveHardwareFaults::kMetadataEntry,
                      GeneralDiagnostics::Attributes::ActiveRadioFaults::kMetadataEntry,
                      GeneralDiagnostics::Attributes::ActiveNetworkFaults::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));

        // Check proper read/write of values and returns
        uint16_t rebootCount                               = 0;
        uint32_t operationalHours                          = 0;
        GeneralDiagnostics::BootReasonEnum bootReasonValue = GeneralDiagnostics::BootReasonEnum::kUnspecified;
        DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> hardwareFaults;
        DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> radioFaults;
        DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> networkFaults;

        EXPECT_EQ(cluster.GetRebootCount(rebootCount), CHIP_NO_ERROR);
        EXPECT_EQ(rebootCount, 123u);

        EXPECT_EQ(cluster.GetTotalOperationalHours(operationalHours), CHIP_NO_ERROR);
        EXPECT_EQ(operationalHours, 456u);

        EXPECT_EQ(cluster.GetBootReason(bootReasonValue), CHIP_NO_ERROR);
        EXPECT_EQ(bootReasonValue, GeneralDiagnostics::BootReasonEnum::kSoftwareReset);

        EXPECT_EQ(cluster.GetActiveHardwareFaults(hardwareFaults), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetActiveRadioFaults(radioFaults), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetActiveNetworkFaults(networkFaults), CHIP_NO_ERROR);
    }
}

TEST_F(TestGeneralDiagnosticsCluster, TimeSnapshotCommandTest)
{
    // Create a cluster with no optional attributes enabled
    const GeneralDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet;
    ScopedDiagnosticsProvider<NullProvider> nullProvider;
    GeneralDiagnosticsCluster cluster(optionalAttributeSet);

    // Prepare command invocation infrastructure
    MockCommandHandler handler;
    ConcreteCommandPath commandPath(kRootEndpointId, GeneralDiagnostics::Id, GeneralDiagnostics::Commands::TimeSnapshot::Id);
    DataModel::InvokeRequest invokeRequest;
    invokeRequest.path = commandPath;

    uint8_t tlvBuffer[kEmptyCommandTLVBufferSize];
    TLV::TLVReader reader;
    SetupEmptyCommandTLV(tlvBuffer, reader);

    // Invoke TimeSnapshot command
    auto result = cluster.InvokeCommand(invokeRequest, reader, &handler);

    // Verify that the command was handled successfully (returns nullopt on success)
    EXPECT_FALSE(result.has_value());

    // Verify that a response was generated
    EXPECT_TRUE(handler.HasResponse());
    EXPECT_EQ(handler.GetResponseCommandId(), GeneralDiagnostics::Commands::TimeSnapshotResponse::Id);

    // Decode and verify the response
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType response;
    ASSERT_EQ(handler.DecodeTimeSnapshotResponse(response), CHIP_NO_ERROR);

    // Verify that systemTimeMs is present
    EXPECT_GE(response.systemTimeMs, 0u);

    // Basic configuration excludes POSIX time
    EXPECT_TRUE(response.posixTimeMs.IsNull());
}

TEST_F(TestGeneralDiagnosticsCluster, TimeSnapshotCommandWithPosixTimeTest)
{
    // Configure cluster with POSIX time support enabled and no optional attributes enabled
    const GeneralDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet;
    ScopedDiagnosticsProvider<NullProvider> nullProvider;
    const GeneralDiagnosticsFunctionsConfig functionsConfig{
        .enablePosixTime      = true,
        .enablePayloadSnaphot = false,
    };
    GeneralDiagnosticsClusterFullConfigurable cluster(optionalAttributeSet, functionsConfig);

    // Prepare command invocation infrastructure
    MockCommandHandler handler;
    ConcreteCommandPath commandPath(kRootEndpointId, GeneralDiagnostics::Id, GeneralDiagnostics::Commands::TimeSnapshot::Id);
    DataModel::InvokeRequest invokeRequest;
    invokeRequest.path = commandPath;

    uint8_t tlvBuffer[kEmptyCommandTLVBufferSize];
    TLV::TLVReader reader;
    SetupEmptyCommandTLV(tlvBuffer, reader);

    // Invoke TimeSnapshot command
    auto result = cluster.InvokeCommand(invokeRequest, reader, &handler);

    // Verify that the command was handled successfully
    EXPECT_FALSE(result.has_value());

    // Verify that a response was generated
    EXPECT_TRUE(handler.HasResponse());
    EXPECT_EQ(handler.GetResponseCommandId(), GeneralDiagnostics::Commands::TimeSnapshotResponse::Id);

    // Decode and verify the response
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType response;
    ASSERT_EQ(handler.DecodeTimeSnapshotResponse(response), CHIP_NO_ERROR);

    // Verify that systemTimeMs is present
    EXPECT_GE(response.systemTimeMs, 0u);

    // POSIX time is included when available (system dependent)
    if (!response.posixTimeMs.IsNull())
    {
        EXPECT_GT(response.posixTimeMs.Value(), 0u);
    }
}

TEST_F(TestGeneralDiagnosticsCluster, TimeSnapshotResponseValues)
{
    // Create a cluster with no optional attributes enabled
    const GeneralDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet;
    ScopedDiagnosticsProvider<NullProvider> nullProvider;
    GeneralDiagnosticsCluster cluster(optionalAttributeSet);

    // Prepare command invocation infrastructure
    ConcreteCommandPath commandPath(kRootEndpointId, GeneralDiagnostics::Id, GeneralDiagnostics::Commands::TimeSnapshot::Id);

    DataModel::InvokeRequest invokeRequest;
    invokeRequest.path = commandPath;

    // First invocation. Capture initial timestamp
    MockCommandHandler firstHandler;
    uint8_t firstTlvBuffer[kEmptyCommandTLVBufferSize];
    TLV::TLVReader firstReader;
    SetupEmptyCommandTLV(firstTlvBuffer, firstReader);
    auto firstResult = cluster.InvokeCommand(invokeRequest, firstReader, &firstHandler);
    EXPECT_FALSE(firstResult.has_value());
    EXPECT_TRUE(firstHandler.HasResponse());

    // Decode first response
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType firstResponse;
    ASSERT_EQ(firstHandler.DecodeTimeSnapshotResponse(firstResponse), CHIP_NO_ERROR);

    // Verify first response is valid
    EXPECT_GE(firstResponse.systemTimeMs, 0u);

    // Second invocation. Capture subsequent timestamp
    MockCommandHandler secondHandler;
    uint8_t secondTlvBuffer[kEmptyCommandTLVBufferSize];
    TLV::TLVReader secondReader;
    SetupEmptyCommandTLV(secondTlvBuffer, secondReader);
    auto secondResult = cluster.InvokeCommand(invokeRequest, secondReader, &secondHandler);
    EXPECT_FALSE(secondResult.has_value());
    EXPECT_TRUE(secondHandler.HasResponse());

    // Decode second response
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType secondResponse;
    ASSERT_EQ(secondHandler.DecodeTimeSnapshotResponse(secondResponse), CHIP_NO_ERROR);

    // Verify second response is also valid and greater than first
    EXPECT_GE(secondResponse.systemTimeMs, firstResponse.systemTimeMs);
}

} // namespace
