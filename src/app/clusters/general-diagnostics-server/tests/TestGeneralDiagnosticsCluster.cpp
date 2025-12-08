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
#include <app/InteractionModelEngine.h>
#include <app/clusters/general-diagnostics-server/GeneralDiagnosticsCluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
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
using namespace chip::Testing;

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

// Helper method to invoke TimeSnapshot command and decode response
CHIP_ERROR InvokeTimeSnapshotAndGetResponse(ClusterTester & tester,
                                            GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType & response)
{
    GeneralDiagnostics::Commands::TimeSnapshot::Type request{};
    auto result = tester.Invoke(GeneralDiagnostics::Commands::TimeSnapshot::Id, request);

    if (!result.status.has_value())
    {
        return CHIP_ERROR_INTERNAL;
    }

    ReturnErrorOnFailure(result.status->GetUnderlyingError());

    if (!result.response.has_value())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    response = result.response.value();
    return CHIP_NO_ERROR;
}

struct TestGeneralDiagnosticsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestGeneralDiagnosticsCluster, CompileTest)
{
    const GeneralDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet;

    GeneralDiagnosticsCluster cluster(optionalAttributeSet, BitFlags<GeneralDiagnostics::Feature>(0),
                                      InteractionModelEngine::GetInstance());
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, GeneralDiagnostics::Id }), BitFlags<ClusterQualityFlags>());

    const GeneralDiagnosticsFunctionsConfig functionsConfig{
        .enablePosixTime       = true,
        .enablePayloadSnapshot = true,
    };

    GeneralDiagnosticsClusterFullConfigurable clusterWithTimeAndPayload(
        optionalAttributeSet, BitFlags<GeneralDiagnostics::Feature>(0), InteractionModelEngine::GetInstance(), functionsConfig);
    ASSERT_EQ(clusterWithTimeAndPayload.GetClusterFlags({ kRootEndpointId, GeneralDiagnostics::Id }),
              BitFlags<ClusterQualityFlags>());
}

TEST_F(TestGeneralDiagnosticsCluster, AttributesTest)
{
    {
        // everything returns empty here ..
        const GeneralDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet;
        ScopedDiagnosticsProvider<NullProvider> nullProvider;

        // Create cluster without enabling any feature flags
        GeneralDiagnosticsCluster cluster(optionalAttributeSet, BitFlags<GeneralDiagnostics::Feature>(0),
                                          InteractionModelEngine::GetInstance());

        // Check required accepted commands are present
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      GeneralDiagnostics::Commands::TestEventTrigger::kMetadataEntry,
                                                      GeneralDiagnostics::Commands::TimeSnapshot::kMetadataEntry,
                                                  }));

        // Check required generated commands are present
        ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                                   {
                                                       GeneralDiagnostics::Commands::TimeSnapshotResponse::Id,
                                                   }));

        // Everything is unimplemented, so attributes are just the global and mandatory ones.
        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                GeneralDiagnostics::Attributes::NetworkInterfaces::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::RebootCount::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::UpTime::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::TestEventTriggersEnabled::kMetadataEntry,
                                            }));
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
            CHIP_ERROR GetBootReason(app::Clusters::GeneralDiagnostics::BootReasonEnum & bootReason) override
            {
                bootReason = GeneralDiagnostics::BootReasonEnum::kSoftwareReset;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR
            GetActiveHardwareFaults(DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & hardwareFaults) override
            {
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetActiveRadioFaults(DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & radioFaults) override
            {
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR
            GetActiveNetworkFaults(DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & networkFaults) override
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

        // Create cluster with LOAD feature flag enabled
        BitFlags<GeneralDiagnostics::Feature> features{ GeneralDiagnostics::Feature::kDeviceLoad };
        GeneralDiagnosticsCluster cluster(optionalAttributeSet, features, InteractionModelEngine::GetInstance());

        // Check mandatory commands are present
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      GeneralDiagnostics::Commands::TestEventTrigger::kMetadataEntry,
                                                      GeneralDiagnostics::Commands::TimeSnapshot::kMetadataEntry,
                                                  }));

        // Check required generated commands are present
        ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                                   {
                                                       GeneralDiagnostics::Commands::TimeSnapshotResponse::Id,
                                                   }));

        // Everything is implemented, so attributes are the global ones and ALL optional ones as well.
        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                GeneralDiagnostics::Attributes::NetworkInterfaces::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::RebootCount::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::UpTime::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::TestEventTriggersEnabled::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::TotalOperationalHours::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::BootReason::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::ActiveHardwareFaults::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::ActiveRadioFaults::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::ActiveNetworkFaults::kMetadataEntry,
                                                GeneralDiagnostics::Attributes::DeviceLoadStatus::kMetadataEntry,
                                            }));

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
    GeneralDiagnosticsCluster cluster(optionalAttributeSet, BitFlags<GeneralDiagnostics::Feature>(0),
                                      InteractionModelEngine::GetInstance());

    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Invoke TimeSnapshot command and get response
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType response;
    ASSERT_EQ(InvokeTimeSnapshotAndGetResponse(tester, response), CHIP_NO_ERROR);

    // Basic configuration excludes POSIX time
    EXPECT_TRUE(response.posixTimeMs.IsNull());
}

TEST_F(TestGeneralDiagnosticsCluster, TimeSnapshotCommandWithPosixTimeTest)
{
    // Configure cluster with POSIX time support enabled and no optional attributes enabled
    const GeneralDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet;
    ScopedDiagnosticsProvider<NullProvider> nullProvider;
    const GeneralDiagnosticsFunctionsConfig functionsConfig{
        .enablePosixTime       = true,
        .enablePayloadSnapshot = false,
    };
    GeneralDiagnosticsClusterFullConfigurable cluster(optionalAttributeSet, BitFlags<GeneralDiagnostics::Feature>(0),
                                                      InteractionModelEngine::GetInstance(), functionsConfig);

    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Invoke TimeSnapshot command and get response
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType response;
    ASSERT_EQ(InvokeTimeSnapshotAndGetResponse(tester, response), CHIP_NO_ERROR);

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
    GeneralDiagnosticsCluster cluster(optionalAttributeSet, BitFlags<GeneralDiagnostics::Feature>(0),
                                      InteractionModelEngine::GetInstance());

    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // First invocation. Capture initial timestamp
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType firstResponse;
    ASSERT_EQ(InvokeTimeSnapshotAndGetResponse(tester, firstResponse), CHIP_NO_ERROR);

    // Second invocation. Capture subsequent timestamp
    GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType secondResponse;
    ASSERT_EQ(InvokeTimeSnapshotAndGetResponse(tester, secondResponse), CHIP_NO_ERROR);

    // Verify second response is also valid and greater than or equal to first
    EXPECT_GE(secondResponse.systemTimeMs, firstResponse.systemTimeMs);
}

} // namespace
