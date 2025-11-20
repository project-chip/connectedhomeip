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
#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
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
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoftwareDiagnostics;

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
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestSoftwareDiagnosticsCluster, CompileTest)
{
    // The cluster should compile for any logic
    SoftwareDiagnosticsServerCluster cluster({});

    // Essentially say "code executes"
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, SoftwareDiagnostics::Id }),
              BitFlags<app::DataModel::ClusterQualityFlags>());
}

TEST_F(TestSoftwareDiagnosticsCluster, AttributesAndCommandTest)
{
    {
        // everything returns empty here ..
        class NullProvider : public DeviceLayer::DiagnosticDataProvider
        {
        };
        ScopedDiagnosticsProvider<NullProvider> nullProvider;
        SoftwareDiagnosticsServerCluster cluster({});
        chip::Test::ClusterTester tester(cluster);

        // without watermarks, no commands are accepted
        EXPECT_TRUE(Testing::IsAcceptedCommandsListEqualTo(cluster, {}));

        // feature map attribute
        Attributes::FeatureMap::TypeInfo::DecodableType featureMap{};
        ASSERT_TRUE(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap).IsSuccess());
        EXPECT_EQ(featureMap, BitFlags<SoftwareDiagnostics::Feature>{}.Raw());

        // Everything is unimplemented, so attributes are just the global ones.
        // This is really not a useful cluster, but possible...
        EXPECT_TRUE(Testing::IsAttributesListEqualTo(cluster, {}));
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
        SoftwareDiagnosticsServerCluster cluster(
            SoftwareDiagnosticsLogic::OptionalAttributeSet().Set<Attributes::CurrentHeapHighWatermark::Id>());
        chip::Test::ClusterTester tester(cluster);

        ASSERT_TRUE(Testing::IsAcceptedCommandsListEqualTo(cluster, { Commands::ResetWatermarks::kMetadataEntry }));

        // feature map attribute
        Attributes::FeatureMap::TypeInfo::DecodableType featureMap{};
        ASSERT_TRUE(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap).IsSuccess());
        EXPECT_EQ(featureMap, BitFlags<SoftwareDiagnostics::Feature>{ SoftwareDiagnostics::Feature::kWatermarks }.Raw());

        // attribute list (only heap high watermark supported)
        EXPECT_TRUE(Testing::IsAttributesListEqualTo(
            cluster, { SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::kMetadataEntry }));

        // Call the command, and verify it calls through to the provider
        EXPECT_FALSE(watermarksProvider.GetProvider().resetCalled);
        EXPECT_TRUE(tester.Invoke(Commands::ResetWatermarks::Id, Commands::ResetWatermarks::Type{}).IsSuccess());
        EXPECT_TRUE(watermarksProvider.GetProvider().resetCalled);
    }

    {
        class AllProvider : public DeviceLayer::DiagnosticDataProvider
        {
        public:
            bool releaseCalled = false;
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
            CHIP_ERROR GetThreadMetrics(DeviceLayer::ThreadMetrics ** threadMetricsList) override
            {
                static DeviceLayer::ThreadMetrics metrics[2];
                metrics[0].Next = &metrics[1];
                metrics[0].id   = 1;
                metrics[1].Next = nullptr;
                metrics[1].id   = 2;
                metrics[1].stackFreeMinimum.SetValue(567u);
                *threadMetricsList = metrics;
                return CHIP_NO_ERROR;
            }
            void ReleaseThreadMetrics(DeviceLayer::ThreadMetrics * threadMetricsList) override { releaseCalled = true; }
        };

        ScopedDiagnosticsProvider<AllProvider> allProvider;
        SoftwareDiagnosticsServerCluster cluster(SoftwareDiagnosticsLogic::OptionalAttributeSet()
                                                     .Set<Attributes::ThreadMetrics::Id>()
                                                     .Set<Attributes::CurrentHeapFree::Id>()
                                                     .Set<Attributes::CurrentHeapUsed::Id>()
                                                     .Set<Attributes::CurrentHeapHighWatermark::Id>());

        chip::Test::ClusterTester tester(cluster);

        // accepted commands list
        ASSERT_TRUE(Testing::IsAcceptedCommandsListEqualTo(cluster, { Commands::ResetWatermarks::kMetadataEntry }));

        // generated commands list
        EXPECT_TRUE(Testing::IsGeneratedCommandsListEqualTo(cluster, {}));

        // attribute list
        ASSERT_TRUE(Testing::IsAttributesListEqualTo(cluster,
                                                     {
                                                         SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::kMetadataEntry,
                                                         SoftwareDiagnostics::Attributes::CurrentHeapFree::kMetadataEntry,
                                                         SoftwareDiagnostics::Attributes::CurrentHeapUsed::kMetadataEntry,
                                                         SoftwareDiagnostics::Attributes::ThreadMetrics::kMetadataEntry,
                                                     }));

        // Test all attributes
        // cluster revision
        Attributes::ClusterRevision::TypeInfo::DecodableType clusterRevision;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision).IsSuccess());
        EXPECT_EQ(clusterRevision, SoftwareDiagnostics::kRevision);

        // feature map
        Attributes::FeatureMap::TypeInfo::DecodableType featureMap;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap).IsSuccess());
        EXPECT_EQ(featureMap, BitFlags<SoftwareDiagnostics::Feature>{ SoftwareDiagnostics::Feature::kWatermarks }.Raw());

        // heapfree
        Attributes::CurrentHeapFree::TypeInfo::DecodableType heapFree;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::CurrentHeapFree::Id, heapFree).IsSuccess());
        EXPECT_EQ(heapFree, 123u);

        // heapused
        Attributes::CurrentHeapUsed::TypeInfo::DecodableType heapUsed;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::CurrentHeapUsed::Id, heapUsed).IsSuccess());
        EXPECT_EQ(heapUsed, 234u);

        // highwatermark
        Attributes::CurrentHeapHighWatermark::TypeInfo::DecodableType highWatermark;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::CurrentHeapHighWatermark::Id, highWatermark).IsSuccess());
        EXPECT_EQ(highWatermark, 456u);

        // threadmetrics
        Attributes::ThreadMetrics::TypeInfo::DecodableType threadMetrics;
        ASSERT_TRUE(tester.ReadAttribute(Attributes::ThreadMetrics::Id, threadMetrics).IsSuccess());
        EXPECT_TRUE(allProvider.GetProvider().releaseCalled);
        {
            auto it = threadMetrics.begin();
            ASSERT_TRUE(it.Next());
            const auto & tm1 = it.GetValue();
            EXPECT_EQ(tm1.id, 1u);
            EXPECT_FALSE(tm1.name.HasValue());
            EXPECT_FALSE(tm1.stackFreeCurrent.HasValue());
            EXPECT_FALSE(tm1.stackFreeMinimum.HasValue());
            EXPECT_FALSE(tm1.stackSize.HasValue());

            ASSERT_TRUE(it.Next());
            const auto & tm2 = it.GetValue();
            EXPECT_EQ(tm2.id, 2u);
            EXPECT_FALSE(tm2.name.HasValue());
            EXPECT_FALSE(tm2.stackFreeCurrent.HasValue());
            ASSERT_TRUE(tm2.stackFreeMinimum.HasValue());
            EXPECT_EQ(tm2.stackFreeMinimum.Value(), 567u);
            EXPECT_FALSE(tm2.stackSize.HasValue());

            EXPECT_FALSE(it.Next());
            EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
        }
    }
}

TEST_F(TestSoftwareDiagnosticsCluster, TestEventGeneration)
{
    SoftwareDiagnosticsServerCluster cluster({});
    chip::Test::ClusterTester tester(cluster);

    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Notify a fault, and verify it is received
    const char faultData[] = "faultdata";
    Events::SoftwareFault::Type fault{
        .id             = 1234,
        .name           = Optional{ "test"_span },
        .faultRecording = Optional{ ByteSpan(Uint8::from_const_char(faultData), strlen(faultData)) },
    };

    SoftwareDiagnostics::SoftwareFaultListener::GlobalNotifySoftwareFaultDetect(fault);

    Events::SoftwareFault::DecodableType decodedFault;
    auto event = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    ASSERT_EQ(event->GetEventData(decodedFault), CHIP_NO_ERROR); // NOLINT(bugprone-unchecked-optional-access)

    EXPECT_EQ(decodedFault.id, fault.id);
    ASSERT_TRUE(decodedFault.name.HasValue());
    EXPECT_TRUE(decodedFault.name.Value().data_equal(fault.name.Value()));
    ASSERT_TRUE(decodedFault.faultRecording.HasValue());
    EXPECT_TRUE(decodedFault.faultRecording.Value().data_equal(fault.faultRecording.Value()));

    cluster.Shutdown();
}

} // namespace
