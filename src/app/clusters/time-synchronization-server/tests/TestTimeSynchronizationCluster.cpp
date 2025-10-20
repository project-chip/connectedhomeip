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

#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/TestReadWriteAttribute.h>
#include <app/clusters/time-synchronization-server/time-synchronization-cluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
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
using namespace chip::Test;

struct TestTimeSynchronizationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestTimeSynchronizationCluster() : testContext(), context(testContext.Create()) {}

    chip::Test::TestServerClusterContext testContext;
    ServerClusterContext context;
};

} // namespace

TEST_F(TestTimeSynchronizationCluster, AttributeTest)
{
    {
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(timeSynchronization.Attributes(ConcreteClusterPath(kRootEndpointId, TimeSynchronization::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        timeSynchronization.Shutdown();
    }

    {
        const DataModel::AttributeEntry optionalAttributes[] = { TimeSource::kMetadataEntry };
        OptionalAttributeSet<TimeSource::Id> optionalAttributeSet;
        optionalAttributeSet.Set<TimeSource::Id>();
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, optionalAttributeSet, features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(timeSynchronization.Attributes(ConcreteClusterPath(kRootEndpointId, TimeSynchronization::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeSyncClient };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(timeSynchronization.Attributes(ConcreteClusterPath(kRootEndpointId, TimeSynchronization::Id), attributes),
                  CHIP_NO_ERROR);

        const DataModel::AttributeEntry optionalAttributes[] = { TrustedTimeSource::kMetadataEntry };
        OptionalAttributeSet<TrustedTimeSource::Id> optionalAttributeSet;
        optionalAttributeSet.Set<TrustedTimeSource::Id>();

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPClient };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(timeSynchronization.Attributes(ConcreteClusterPath(kRootEndpointId, TimeSynchronization::Id), attributes),
                  CHIP_NO_ERROR);

        const DataModel::AttributeEntry optionalAttributes[] = { DefaultNTP::kMetadataEntry, SupportsDNSResolve::kMetadataEntry };
        OptionalAttributeSet<DefaultNTP::Id, SupportsDNSResolve::Id> optionalAttributeSet;
        optionalAttributeSet.Set<DefaultNTP::Id>();
        optionalAttributeSet.Set<SupportsDNSResolve::Id>();

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPServer };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(timeSynchronization.Attributes(ConcreteClusterPath(kRootEndpointId, TimeSynchronization::Id), attributes),
                  CHIP_NO_ERROR);

        const DataModel::AttributeEntry optionalAttributes[] = { NTPServerAvailable::kMetadataEntry };
        OptionalAttributeSet<NTPServerAvailable::Id> optionalAttributeSet;
        optionalAttributeSet.Set<NTPServerAvailable::Id>();

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeZone };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(timeSynchronization.Attributes(ConcreteClusterPath(kRootEndpointId, TimeSynchronization::Id), attributes),
                  CHIP_NO_ERROR);

        const DataModel::AttributeEntry optionalAttributes[] = {
            TimeZone::kMetadataEntry,         DSTOffset::kMetadataEntry,           LocalTime::kMetadataEntry,
            TimeZoneDatabase::kMetadataEntry, TimeZoneListMaxSize::kMetadataEntry, DSTOffsetListMaxSize::kMetadataEntry
        };
        OptionalAttributeSet<TimeZone::Id, DSTOffset::Id, LocalTime::Id, TimeZoneDatabase::Id, TimeZoneListMaxSize::Id,
                             DSTOffsetListMaxSize::Id>
            optionalAttributeSet;
        optionalAttributeSet.Set<TimeZone::Id>();
        optionalAttributeSet.Set<DSTOffset::Id>();
        optionalAttributeSet.Set<LocalTime::Id>();
        optionalAttributeSet.Set<TimeZoneDatabase::Id>();
        optionalAttributeSet.Set<TimeZoneListMaxSize::Id>();
        optionalAttributeSet.Set<DSTOffsetListMaxSize::Id>();

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        timeSynchronization.Shutdown();
    }
}

TEST_F(TestTimeSynchronizationCluster, ReadAttributeTest)
{
    {
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        uint16_t revision{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type utcTime{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, utcTime), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        OptionalAttributeSet<TimeSource::Id> optionalAttributeSet;
        optionalAttributeSet.Set<TimeSource::Id>();
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, optionalAttributeSet, features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        uint16_t revision{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type utcTime{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, utcTime), CHIP_NO_ERROR);

        TimeSourceEnum timeSource{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, TimeSource::Id, timeSource), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeSyncClient };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        uint16_t revision{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type utcTime{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, utcTime), CHIP_NO_ERROR);

        TrustedTimeSource::TypeInfo::Type trustedTimeSource{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, TrustedTimeSource::Id, trustedTimeSource), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPClient };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        uint16_t revision{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type utcTime{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, utcTime), CHIP_NO_ERROR);

        DefaultNTP::TypeInfo::Type defaultNTP{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, DefaultNTP::Id, defaultNTP), CHIP_NO_ERROR);

        SupportsDNSResolve::TypeInfo::Type supportsDNSResolve{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, SupportsDNSResolve::Id, supportsDNSResolve), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPServer };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        uint16_t revision{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type utcTime{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, utcTime), CHIP_NO_ERROR);

        NTPServerAvailable::TypeInfo::Type ntpServerAvailable{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, NTPServerAvailable::Id, ntpServerAvailable), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeZone };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        uint16_t revision{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, ClusterRevision::Id, revision), CHIP_NO_ERROR);

        uint32_t featureMap{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, FeatureMap::Id, featureMap), CHIP_NO_ERROR);

        GranularityEnum granularity{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, granularity), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type utcTime{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, utcTime), CHIP_NO_ERROR);

        // TODO: It's not safe to use ReadClusterAttribute() for a list
        // DataModel::DecodableList<Structs::TimeZoneStruct::Type> timeZoneList;
        // ASSERT_EQ(ReadClusterAttribute(timeSynchronization, TimeZone::Id, timeZoneList), CHIP_NO_ERROR);

        // TODO: It's not safe to use ReadClusterAttribute() for a list
        // DataModel::DecodableList<Structs::DSTOffsetStruct::Type> dstOffsetList;
        // ASSERT_EQ(ReadClusterAttribute(timeSynchronization, DSTOffset::Id, dstOffsetList), CHIP_NO_ERROR);

        LocalTime::TypeInfo::Type localTime{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, LocalTime::Id, localTime), CHIP_NO_ERROR);

        TimeZoneDatabaseEnum timeZoneDatabase{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, TimeZoneDatabase::Id, timeZoneDatabase), CHIP_NO_ERROR);

        TimeZoneListMaxSize::TypeInfo::Type timeZoneListMaxSize{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, TimeZoneListMaxSize::Id, timeZoneListMaxSize), CHIP_NO_ERROR);

        DSTOffsetListMaxSize::TypeInfo::Type dstOffsetListMaxSize{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, DSTOffsetListMaxSize::Id, dstOffsetListMaxSize), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }
}
