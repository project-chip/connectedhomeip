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

struct TestTimeSynchronizationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestTimeSynchronizationCluster() : testContext(), context(testContext.Create()) {}

    chip::Test::TestServerClusterContext testContext;
    ServerClusterContext context;
};

template <typename ClusterT, typename T>
inline CHIP_ERROR ReadClusterAttribute(ClusterT & cluster, AttributeId attr, T & val)
{
    return chip::Test::ReadClusterAttribute(cluster, ConcreteAttributePath(kRootEndpointId, TimeSynchronization::Id, attr), val);
}

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

        GranularityEnum val{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, val), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type val2{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, val2), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        OptionalAttributeSet<TimeSource::Id> optionalAttributeSet;
        optionalAttributeSet.Set<TimeSource::Id>();
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, optionalAttributeSet, features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        GranularityEnum val{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, val), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type val2{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, val2), CHIP_NO_ERROR);

        TimeSourceEnum val3{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, TimeSource::Id, val3), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeSyncClient };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        GranularityEnum val{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, val), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type val2{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, val2), CHIP_NO_ERROR);

        TrustedTimeSource::TypeInfo::Type val3{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, TrustedTimeSource::Id, val3), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPClient };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        GranularityEnum val{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, val), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type val2{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, val2), CHIP_NO_ERROR);

        DefaultNTP::TypeInfo::Type val3{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, DefaultNTP::Id, val3), CHIP_NO_ERROR);

        SupportsDNSResolve::TypeInfo::Type val4{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, SupportsDNSResolve::Id, val4), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPServer };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        GranularityEnum val{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, val), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type val2{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, val2), CHIP_NO_ERROR);

        NTPServerAvailable::TypeInfo::Type val3{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, NTPServerAvailable::Id, val3), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeZone };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, OptionalAttributeSet(), features, false,
                                                       TimeZoneDatabaseEnum::kNone, TimeSourceEnum::kNone, false);
        ASSERT_EQ(timeSynchronization.Startup(context), CHIP_NO_ERROR);

        GranularityEnum val{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, Granularity::Id, val), CHIP_NO_ERROR);

        UTCTime::TypeInfo::Type val2{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, UTCTime::Id, val2), CHIP_NO_ERROR);

        DataModel::DecodableList<Structs::TimeZoneStruct::Type> val3;
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, TimeZone::Id, val3), CHIP_NO_ERROR);
        // Expect the time zone list is not empty
        EXPECT_TRUE(val3.begin().Next());

        DataModel::DecodableList<Structs::DSTOffsetStruct::Type> val4;
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, DSTOffset::Id, val4), CHIP_NO_ERROR);

        LocalTime::TypeInfo::Type val5{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, LocalTime::Id, val5), CHIP_NO_ERROR);

        TimeZoneDatabaseEnum val6{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, TimeZoneDatabase::Id, val6), CHIP_NO_ERROR);

        TimeZoneListMaxSize::TypeInfo::Type val7{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, TimeZoneListMaxSize::Id, val7), CHIP_NO_ERROR);

        DSTOffsetListMaxSize::TypeInfo::Type val8{};
        ASSERT_EQ(ReadClusterAttribute(timeSynchronization, DSTOffsetListMaxSize::Id, val8), CHIP_NO_ERROR);

        timeSynchronization.Shutdown();
    }
}
