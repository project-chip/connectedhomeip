/*
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
#include <pw_unit_test/framework.h>

#include <app/clusters/time-synchronization-server/TimeSynchronizationCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/TimeSynchronization/Attributes.h>
#include <clusters/TimeSynchronization/Enums.h>
#include <clusters/TimeSynchronization/Metadata.h>
#include <clusters/TimeSynchronization/Structs.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TimeSynchronization;
using namespace chip::app::Clusters::TimeSynchronization::Attributes;
using namespace chip::Testing;

struct TestTimeSynchronizationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestTimeSynchronizationCluster() {}

    TestServerClusterContext testContext;
    TimeSynchronization::DefaultTimeSyncDelegate delegate;
};

} // namespace

TEST_F(TestTimeSynchronizationCluster, AttributeTest)
{
    {
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features,
                                                       TimeSynchronizationCluster::OptionalAttributeSet(),
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown();
    }

    {
        TimeSynchronizationCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<TimeSource::Id>();
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features, optionalAttributeSet,
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                                TimeSource::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeSyncClient };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features,
                                                       TimeSynchronizationCluster::OptionalAttributeSet(),
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                                TrustedTimeSource::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPClient };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features,
                                                       TimeSynchronizationCluster::OptionalAttributeSet(),
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                                DefaultNTP::kMetadataEntry,
                                                SupportsDNSResolve::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPServer };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features,
                                                       TimeSynchronizationCluster::OptionalAttributeSet(),
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                                NTPServerAvailable::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeZone };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features,
                                                       TimeSynchronizationCluster::OptionalAttributeSet(),
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                                TimeZone::kMetadataEntry,
                                                DSTOffset::kMetadataEntry,
                                                LocalTime::kMetadataEntry,
                                                TimeZoneDatabase::kMetadataEntry,
                                                TimeZoneListMaxSize::kMetadataEntry,
                                                DSTOffsetListMaxSize::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown();
    }
}

TEST_F(TestTimeSynchronizationCluster, ReadAttributeTest)
{
    {
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features,
                                                       TimeSynchronizationCluster::OptionalAttributeSet(),
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(timeSynchronization);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(tester.ReadAttribute(Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::DecodableType utcTime{};
        ASSERT_EQ(tester.ReadAttribute(UTCTime::Id, utcTime), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        TimeSynchronizationCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<TimeSource::Id>();
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features, optionalAttributeSet,
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(timeSynchronization);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(tester.ReadAttribute(Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::DecodableType utcTime{};
        ASSERT_EQ(tester.ReadAttribute(UTCTime::Id, utcTime), CHIP_NO_ERROR);

        TimeSourceEnum timeSource{};
        ASSERT_EQ(tester.ReadAttribute(TimeSource::Id, timeSource), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeSyncClient };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features,
                                                       TimeSynchronizationCluster::OptionalAttributeSet(),
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(timeSynchronization);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(tester.ReadAttribute(Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::DecodableType utcTime{};
        ASSERT_EQ(tester.ReadAttribute(UTCTime::Id, utcTime), CHIP_NO_ERROR);

        TrustedTimeSource::TypeInfo::DecodableType trustedTimeSource{};
        ASSERT_EQ(tester.ReadAttribute(TrustedTimeSource::Id, trustedTimeSource), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPClient };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features,
                                                       TimeSynchronizationCluster::OptionalAttributeSet(),
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(timeSynchronization);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(tester.ReadAttribute(Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::DecodableType utcTime{};
        ASSERT_EQ(tester.ReadAttribute(UTCTime::Id, utcTime), CHIP_NO_ERROR);

        DefaultNTP::TypeInfo::DecodableType defaultNTP{};
        ASSERT_EQ(tester.ReadAttribute(DefaultNTP::Id, defaultNTP), CHIP_NO_ERROR);

        SupportsDNSResolve::TypeInfo::DecodableType supportsDNSResolve{};
        ASSERT_EQ(tester.ReadAttribute(SupportsDNSResolve::Id, supportsDNSResolve), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPServer };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features,
                                                       TimeSynchronizationCluster::OptionalAttributeSet(),
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(timeSynchronization);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(tester.ReadAttribute(Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::DecodableType utcTime{};
        ASSERT_EQ(tester.ReadAttribute(UTCTime::Id, utcTime), CHIP_NO_ERROR);

        NTPServerAvailable::TypeInfo::DecodableType ntpServerAvailable{};
        ASSERT_EQ(tester.ReadAttribute(NTPServerAvailable::Id, ntpServerAvailable), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeZone };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features,
                                                       TimeSynchronizationCluster::OptionalAttributeSet(),
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate });
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(timeSynchronization);

        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(tester.ReadAttribute(Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::DecodableType utcTime{};
        ASSERT_EQ(tester.ReadAttribute(UTCTime::Id, utcTime), CHIP_NO_ERROR);

        DataModel::DecodableList<Structs::TimeZoneStruct::DecodableType> timeZoneList;
        ASSERT_EQ(tester.ReadAttribute(TimeZone::Id, timeZoneList), CHIP_NO_ERROR);

        DataModel::DecodableList<Structs::DSTOffsetStruct::DecodableType> dstOffsetList;
        ASSERT_EQ(tester.ReadAttribute(DSTOffset::Id, dstOffsetList), CHIP_NO_ERROR);

        LocalTime::TypeInfo::DecodableType localTime{};
        ASSERT_EQ(tester.ReadAttribute(LocalTime::Id, localTime), CHIP_NO_ERROR);

        TimeZoneDatabaseEnum timeZoneDatabase{};
        ASSERT_EQ(tester.ReadAttribute(TimeZoneDatabase::Id, timeZoneDatabase), CHIP_NO_ERROR);

        TimeZoneListMaxSize::TypeInfo::DecodableType timeZoneListMaxSize{};
        ASSERT_EQ(tester.ReadAttribute(TimeZoneListMaxSize::Id, timeZoneListMaxSize), CHIP_NO_ERROR);

        DSTOffsetListMaxSize::TypeInfo::DecodableType dstOffsetListMaxSize{};
        ASSERT_EQ(tester.ReadAttribute(DSTOffsetListMaxSize::Id, dstOffsetListMaxSize), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }
}
