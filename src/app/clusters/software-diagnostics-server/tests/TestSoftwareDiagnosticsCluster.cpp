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
#include <app/clusters/software-diagnostics-server/software-fault-listener.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
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
using namespace chip::app::Clusters::SoftwareDiagnostics::Attributes;

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

    T & GetProvider() { return mProvider; }

private:
    DeviceLayer::DiagnosticDataProvider * mOldProvider;
    T mProvider;
};

struct TestSoftwareDiagnosticsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestSoftwareDiagnosticsCluster, CompileTest)
{
    // The cluster should compile for any logic
    SoftwareDiagnosticsServerCluster cluster({});

    // Essentially say "code executes"
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, SoftwareDiagnostics::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestSoftwareDiagnosticsCluster, AttributesAndCommandTest)
{
    {
        // everything returns empty here ..
        class NullProvider : public DeviceLayer::DiagnosticDataProvider
        {
        };
        ScopedDiagnosticsProvider<NullProvider> nullProvider;
        SoftwareDiagnosticsLogic diag({});

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

            CHIP_ERROR ResetWatermarks() override
            {
                resetCalled = true;
                return CHIP_NO_ERROR;
            }

            bool resetCalled = false;
        };

        ScopedDiagnosticsProvider<WatermarksProvider> watermarksProvider;
        SoftwareDiagnosticsLogic diag(SoftwareDiagnosticsLogic::OptionalAttributeSet().Set<CurrentHeapHighWatermark::Id>());

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

        // Call the command, and verify it calls through to the provider
        ASSERT_FALSE(watermarksProvider.GetProvider().resetCalled);
        ASSERT_EQ(diag.ResetWatermarks(), CHIP_NO_ERROR);
        ASSERT_TRUE(watermarksProvider.GetProvider().resetCalled);
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

        ScopedDiagnosticsProvider<AllProvider> allProvider;
        SoftwareDiagnosticsLogic diag(SoftwareDiagnosticsLogic::OptionalAttributeSet()
                                          .Set<ThreadMetrics::Id>()
                                          .Set<CurrentHeapFree::Id>()
                                          .Set<CurrentHeapUsed::Id>()
                                          .Set<CurrentHeapHighWatermark::Id>());

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

    // Here should be test for ThreadMetrics attribute, but this will be harder to do without a testing
    // infrastructure for clusters.
}

TEST_F(TestSoftwareDiagnosticsCluster, SoftwareFaultListenerTest)
{
    SoftwareDiagnosticsServerCluster cluster({});
    chip::Test::TestServerClusterContext context;

    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Notify a fault, and verify it is received
    chip::app::Clusters::SoftwareDiagnostics::Events::SoftwareFault::Type fault;
    fault.id = 1234;
    fault.name.SetValue(CharSpan::fromCharString("test"));
    const char faultData[] = "faultdata";
    fault.faultRecording.SetValue(ByteSpan(Uint8::from_const_char(faultData), strlen(faultData)));

    SoftwareDiagnostics::SoftwareFaultListener::GlobalNotifySoftwareFaultDetect(fault);

    chip::app::Clusters::SoftwareDiagnostics::Events::SoftwareFault::DecodableType decodedFault;
    auto event = context.EventsGenerator().GetNextEvent();
    ASSERT_TRUE(event.has_value());
    ASSERT_EQ(event->GetEventData(decodedFault), CHIP_NO_ERROR); // NOLINT(bugprone-unchecked-optional-access)

    ASSERT_EQ(decodedFault.id, fault.id);
    ASSERT_TRUE(decodedFault.name.HasValue());
    ASSERT_TRUE(decodedFault.name.Value().data_equal(fault.name.Value()));
    ASSERT_TRUE(decodedFault.faultRecording.HasValue());
    ASSERT_TRUE(decodedFault.faultRecording.Value().data_equal(fault.faultRecording.Value()));

    cluster.Shutdown();
}

} // namespace
