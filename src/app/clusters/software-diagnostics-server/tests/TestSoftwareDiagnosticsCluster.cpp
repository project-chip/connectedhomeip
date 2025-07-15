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

#include <app/clusters/software-diagnostics-server/software-diagnostics-cluster.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-logic.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/SoftwareDiagnostics/Enums.h>
#include <clusters/SoftwareDiagnostics/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DiagnosticDataProvider.h>

#include <cmath>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;

struct TestSoftwareDiagnosticsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestSoftwareDiagnosticsCluster, CompileTest)
{
    const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
        .enableThreadMetrics     = false,
        .enableCurrentHeapFree   = false,
        .enableCurrentHeapUsed   = false,
        .enableCurrentWatermarks = false,
    };

    // The cluster should compile for any logic
    SoftwareDiagnosticsServerCluster<DeviceLayerSoftwareDiagnosticsLogic> cluster(enabledAttributes);

    // Essentially say "code executes"
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, SoftwareDiagnostics::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestSoftwareDiagnosticsCluster, AttributesTest)
{
    {
        // everything returns empty here ..
        class NullProvider : public DeviceLayer::DiagnosticDataProvider
        {
        };
        const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
            .enableThreadMetrics     = false,
            .enableCurrentHeapFree   = false,
            .enableCurrentHeapUsed   = false,
            .enableCurrentWatermarks = false,
        };
        NullProvider nullProvider;
        InjectedDiagnosticsSoftwareDiagnosticsLogic diag(nullProvider, enabledAttributes);

        // without watermarks, no commands are accepted
        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(diag.AcceptedCommands(commandsBuilder), CHIP_NO_ERROR);
        ASSERT_EQ(commandsBuilder.TakeBuffer().size(), 0u);

        ASSERT_EQ(diag.GetFeatureMap(), BitFlags<SoftwareDiagnostics::Feature>{});

        // Everything is unimplemented, so attributes are just the global ones.
        // This is really not a useful cluster, but possible...
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(diag.Attributes(attributesBuilder), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), DefaultServerCluster::GlobalAttributes()));
    }

    {
        class WatermarksProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            bool SupportsWatermarks() override { return true; }
        };

        const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
            .enableThreadMetrics     = false,
            .enableCurrentHeapFree   = false,
            .enableCurrentHeapUsed   = false,
            .enableCurrentWatermarks = true,
        };

        WatermarksProvider watermarksProvider;
        InjectedDiagnosticsSoftwareDiagnosticsLogic diag(watermarksProvider, enabledAttributes);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(diag.AcceptedCommands(commandsBuilder), CHIP_NO_ERROR);

        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> commands = commandsBuilder.TakeBuffer();
        ASSERT_EQ(commands.size(), 1u);
        ASSERT_EQ(commands[0].commandId, SoftwareDiagnostics::Commands::ResetWatermarks::Id);
        ASSERT_EQ(commands[0].GetInvokePrivilege(),
                  SoftwareDiagnostics::Commands::ResetWatermarks::kMetadataEntry.GetInvokePrivilege());

        ASSERT_EQ(diag.GetFeatureMap(), BitFlags<SoftwareDiagnostics::Feature>{ SoftwareDiagnostics::Feature::kWatermarks });

        // Everything is unimplemented, so attributes are just the global ones.
        // This is really not a useful cluster, but possible...
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(diag.Attributes(attributesBuilder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({ SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::kMetadataEntry }),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    {
        class AllProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            bool SupportsWatermarks() override { return true; }

            CHIP_ERROR GetCurrentHeapFree(uint64_t & v) override
            {
                v = 123;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetCurrentHeapUsed(uint64_t & v) override
            {
                v = 234;
                return CHIP_NO_ERROR;
            }
            CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & v) override
            {
                v = 456;
                return CHIP_NO_ERROR;
            }
        };

        const SoftwareDiagnosticsEnabledAttributes enabledAttributes{
            .enableThreadMetrics     = true,
            .enableCurrentHeapFree   = true,
            .enableCurrentHeapUsed   = true,
            .enableCurrentWatermarks = true,
        };

        AllProvider allProvider;
        InjectedDiagnosticsSoftwareDiagnosticsLogic diag(allProvider, enabledAttributes);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;
        ASSERT_EQ(diag.AcceptedCommands(commandsBuilder), CHIP_NO_ERROR);

        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> commands = commandsBuilder.TakeBuffer();
        ASSERT_EQ(commands.size(), 1u);
        ASSERT_EQ(commands[0].commandId, SoftwareDiagnostics::Commands::ResetWatermarks::Id);
        ASSERT_EQ(commands[0].GetInvokePrivilege(),
                  SoftwareDiagnostics::Commands::ResetWatermarks::kMetadataEntry.GetInvokePrivilege());

        ASSERT_EQ(diag.GetFeatureMap(), BitFlags<SoftwareDiagnostics::Feature>{ SoftwareDiagnostics::Feature::kWatermarks });

        // Everything is unimplemented, so attributes are just the global ones.
        // This is really not a useful cluster, but possible...
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(diag.Attributes(attributesBuilder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.AppendElements({
                      SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::kMetadataEntry,
                      SoftwareDiagnostics::Attributes::CurrentHeapFree::kMetadataEntry,
                      SoftwareDiagnostics::Attributes::CurrentHeapUsed::kMetadataEntry,
                      SoftwareDiagnostics::Attributes::ThreadMetrics::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));

        // assert values are read correctly
        uint64_t value = 0;

        EXPECT_EQ(diag.GetCurrentHeapFree(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 123u);

        EXPECT_EQ(diag.GetCurrentHeapUsed(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 234u);

        EXPECT_EQ(diag.GetCurrentHighWatermark(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 456u);
    }
}

} // namespace
