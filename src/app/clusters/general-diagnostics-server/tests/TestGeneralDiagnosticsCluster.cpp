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
#include <app/clusters/testing/ClusterTester.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/GeneralDiagnostics/Enums.h>
#include <clusters/GeneralDiagnostics/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <messaging/ExchangeContext.h>
#include <platform/DiagnosticDataProvider.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralDiagnostics::Attributes;
using namespace chip::app::DataModel;
using namespace chip::Test;

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

    // Helper method to invoke TimeSnapshot command and decode response
    template <typename ClusterType>
    static void InvokeTimeSnapshotAndGetResponse(ClusterType & cluster,
                                                 GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType & response)
    {
        ClusterTester tester(cluster);
        CHIP_ERROR startupError = cluster.Startup(tester.GetServerClusterContext());
        ASSERT_TRUE(startupError == CHIP_NO_ERROR || startupError == CHIP_ERROR_ALREADY_INITIALIZED);

        GeneralDiagnostics::Commands::TimeSnapshot::Type request{};
        auto result = tester.Invoke<GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType,
                                    GeneralDiagnostics::Commands::TimeSnapshot::Type>(
            GeneralDiagnostics::Commands::TimeSnapshot::Id, request);

        ASSERT_TRUE(result.status.has_value());
        ASSERT_TRUE(result.status->IsSuccess());
        ASSERT_TRUE(result.response.has_value());
        response = result.response.value();
    }
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

        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
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

        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));

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

    // Invoke TimeSnapshot command and get response
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType response;
    InvokeTimeSnapshotAndGetResponse(cluster, response);

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

    // Invoke TimeSnapshot command and get response
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType response;
    InvokeTimeSnapshotAndGetResponse(cluster, response);

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

    // First invocation. Capture initial timestamp
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType firstResponse;
    InvokeTimeSnapshotAndGetResponse(cluster, firstResponse);

    // Verify first response is valid
    EXPECT_GE(firstResponse.systemTimeMs, 0u);

    // Second invocation. Capture subsequent timestamp
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType secondResponse;
    InvokeTimeSnapshotAndGetResponse(cluster, secondResponse);

    // Verify second response is also valid and greater than first
    EXPECT_GE(secondResponse.systemTimeMs, firstResponse.systemTimeMs);
}

} // namespace
