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
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/StatusIB.h>
#include <app/ReadClient.h>
#include <app/clusters/time-synchronization-server/TimeSynchronizationCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/TimeSynchronization/Attributes.h>
#include <clusters/TimeSynchronization/Commands.h>
#include <clusters/TimeSynchronization/Enums.h>
#include <clusters/TimeSynchronization/Events.h>
#include <clusters/TimeSynchronization/Metadata.h>
#include <clusters/TimeSynchronization/Structs.h>
#include <credentials/FabricTable.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <lib/core/CancelableOperation.h>
#include <lib/support/Defer.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <protocols/interaction_model/Constants.h>
#include <system/RAIIMockClock.h>

#include <optional>
#include <utility>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TimeSynchronization;
using namespace chip::app::Clusters::TimeSynchronization::Attributes;
using namespace chip::Testing;

// A platform time request that stays pending until the test completes it, standing in for the
// asynchronous work a real delegate would do. Start() and Complete() are public because the test
// drives the request from outside.
class PlatformTimeRequest : public chip::Callback::CancelableOperation<TimeSourceEnum, GranularityEnum>
{
    using Base = chip::Callback::CancelableOperation<TimeSourceEnum, GranularityEnum>;

public:
    using Base::Complete;
    using Base::Start;
};

// Test delegate that stubs UpdateTimeFromPlatformSource to prevent AttemptToGetTime() from
// falling back to AttemptToGetTimeFromTrustedNode() on platforms where GetClock_RealTime() is
// not implemented (e.g. Zephyr / nrf-native-sim). Falling back would attempt to establish a CASE
// session via CASESessionManager, which is uninitialized in unit test fixtures and would segfault.
struct TestTimeSyncDelegate : public TimeSynchronization::DefaultTimeSyncDelegate
{
    CHIP_ERROR UpdateTimeFromPlatformSource(chip::Callback::Callback<TimeSynchronization::OnTimeSyncCompletion> * callback) override
    {
        // Ideally the delegate API would take the Owned token directly (see CHIPCallback.h)
        chip::Callback::Callback<TimeSynchronization::OnTimeSyncCompletion>::Owned onCompletion(callback);
        VerifyOrReturnError(!mPlatformTimeRequest.IsPending(), CHIP_ERROR_BUSY);

        if (mPlatformTimeResult.has_value())
        {
            onCompletion.Invoke(mPlatformTimeResult->first, mPlatformTimeResult->second);
            return CHIP_NO_ERROR;
        }
        mPlatformTimeRequest.Start(std::move(onCompletion));
        return CHIP_NO_ERROR;
    }

    // If set, UpdateTimeFromPlatformSource() calls the callback synchronously with this result.
    // Otherwise the request stays pending in mPlatformTimeRequest.
    std::optional<std::pair<TimeSourceEnum, GranularityEnum>> mPlatformTimeResult;
    PlatformTimeRequest mPlatformTimeRequest;
};

struct TestTimeSynchronizationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestTimeSynchronizationCluster() {}

    // Returns kInvalidEventId if there is no further event.
    EventId NextEventId()
    {
        auto event = testContext.EventsGenerator().GetNextEvent();
        return event.has_value() ? event->eventOptions.mPath.mEventId : kInvalidEventId;
    }

    TestServerClusterContext testContext;
    TestTimeSyncDelegate delegate;
    const TimeSynchronizationCluster::Context context = { .fabricTable            = testContext.GetFabricTable(),
                                                          .caseSessionManager     = testContext.GetCASESessionManager(),
                                                          .platformManager        = testContext.GetPlatformManager(),
                                                          .interactionModelEngine = testContext.GetInteractionModelEngine() };
};

} // namespace

#if TIME_SYNC_ENABLE_TSC_FEATURE
// Guard against a caller that passes apData == nullptr with a SUCCESS status.
TEST_F(TestTimeSynchronizationCluster, OnAttributeDataToleratesNullData)
{
    const BitFlags<Feature> features{ Feature::kTimeSyncClient };
    TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
                                                   TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate },
                                                   context);
    ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadClient::Callback & callback = timeSynchronization;
    const StatusIB success{ Protocols::InteractionModel::Status::Success };
    const StatusIB failure{ Protocols::InteractionModel::Status::Failure };

    callback.OnAttributeData(ConcreteDataAttributePath{ kRootEndpointId, Id, UTCTime::Id }, nullptr, success);
    callback.OnAttributeData(ConcreteDataAttributePath{ kRootEndpointId, Id, Granularity::Id }, nullptr, success);
    callback.OnAttributeData(ConcreteDataAttributePath{ kRootEndpointId, Id, UTCTime::Id }, nullptr, failure);
    callback.OnAttributeData(ConcreteDataAttributePath{ kRootEndpointId, Id, ClusterRevision::Id }, nullptr, success);

    timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
}
#endif // TIME_SYNC_ENABLE_TSC_FEATURE

TEST_F(TestTimeSynchronizationCluster, AttributeTest)
{
    {
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        TimeSynchronizationCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<TimeSource::Id>();
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features, optionalAttributeSet,
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate },
                                                       context);
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                                TimeSource::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeSyncClient };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                                TrustedTimeSource::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPClient };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                                DefaultNTP::kMetadataEntry,
                                                SupportsDNSResolve::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPServer };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_TRUE(IsAttributesListEqualTo(timeSynchronization,
                                            {
                                                UTCTime::kMetadataEntry,
                                                Granularity::kMetadataEntry,
                                                NTPServerAvailable::kMetadataEntry,
                                            }));

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeZone };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
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

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestTimeSynchronizationCluster, ReadAttributeTest)
{
    {
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
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

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        TimeSynchronizationCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<TimeSource::Id>();
        const BitFlags<Feature> features{ 0U };
        TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features, optionalAttributeSet,
                                                       TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate },
                                                       context);
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

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeSyncClient };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
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

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPClient };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
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

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kNTPServer };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
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

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const BitFlags<Feature> features{ Feature::kTimeZone };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
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

        timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestTimeSynchronizationCluster, SetTrustedTimeSourceTest)
{
    const BitFlags<Feature> features{ Feature::kTimeSyncClient };
    TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
                                                   TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate },
                                                   context);
    ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(timeSynchronization);

    // 1. Invoke SetTrustedTimeSource with a valid (non-null) time source
    Commands::SetTrustedTimeSource::Type request;
    Structs::FabricScopedTrustedTimeSourceStruct::Type tts;
    tts.nodeID   = 1234;
    tts.endpoint = 0;
    request.trustedTimeSource.SetNonNull(tts);

    auto result = tester.Invoke(request);
    EXPECT_TRUE(result.IsSuccess());

    // Verify that NO event (specifically no TimeFailure event) is emitted on successful configuration
    EXPECT_FALSE(testContext.EventsGenerator().GetNextEvent().has_value());

    // Verify the attribute was updated
    TrustedTimeSource::TypeInfo::DecodableType readBackTTS{};
    ASSERT_EQ(tester.ReadAttribute(TrustedTimeSource::Id, readBackTTS), CHIP_NO_ERROR);
    ASSERT_FALSE(readBackTTS.IsNull());
    EXPECT_EQ(readBackTTS.Value().nodeID, 1234u);
    EXPECT_EQ(readBackTTS.Value().endpoint, 0u);

    // 2. Invoke SetTrustedTimeSource with null to clear the source
    Commands::SetTrustedTimeSource::Type clearRequest;
    clearRequest.trustedTimeSource.SetNull();

    auto clearResult = tester.Invoke(clearRequest);
    EXPECT_TRUE(clearResult.IsSuccess());

    // Verify that MissingTrustedTimeSource event is emitted when set to null
    auto eventInfo = testContext.EventsGenerator().GetNextEvent();
    ASSERT_TRUE(eventInfo.has_value());
    auto & event = *eventInfo;
    EXPECT_EQ(event.eventOptions.mPath.mEventId, Events::MissingTrustedTimeSource::Id);
    Events::MissingTrustedTimeSource::DecodableType missingEvent;
    EXPECT_EQ(event.GetEventData(missingEvent), CHIP_NO_ERROR);

    // Verify no further events
    EXPECT_FALSE(testContext.EventsGenerator().GetNextEvent().has_value());

    // Verify attribute is now null
    ASSERT_EQ(tester.ReadAttribute(TrustedTimeSource::Id, readBackTTS), CHIP_NO_ERROR);
    EXPECT_TRUE(readBackTTS.IsNull());

    timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A failed platform time sync (kNoTimeGranularity) falls back to the trusted node and then to NTP.
// With neither configured, the fallback ends in a TimeFailure event.
TEST_F(TestTimeSynchronizationCluster, PlatformTimeFailureFallsBack)
{
    const std::pair<TimeSourceEnum, GranularityEnum> failureResults[] = {
        { TimeSourceEnum::kNone, GranularityEnum::kNoTimeGranularity },  // documented failure result
        { TimeSourceEnum::kAdmin, GranularityEnum::kNoTimeGranularity }, // timeSource is ignored on failure
    };

    for (const auto & result : failureResults)
    {
        delegate.mPlatformTimeResult = result;
        const BitFlags<Feature> features{ Feature::kTimeSyncClient };
        TimeSynchronizationCluster timeSynchronization(
            kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
            TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate }, context);
        ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);
        auto deferShutdown =
            MakeDefer([&timeSynchronization] { timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown); });
        ClusterTester tester(timeSynchronization);

        // Clearing the trusted time source makes the cluster ask the delegate for the time.
        Commands::SetTrustedTimeSource::Type request;
        request.trustedTimeSource.SetNull();
        EXPECT_TRUE(tester.Invoke(request).IsSuccess());

        EXPECT_EQ(NextEventId(), Events::MissingTrustedTimeSource::Id);
        EXPECT_EQ(NextEventId(), Events::TimeFailure::Id);
        EXPECT_EQ(NextEventId(), kInvalidEventId);

        Granularity::TypeInfo::DecodableType granularity{};
        EXPECT_EQ(tester.ReadAttribute(Granularity::Id, granularity), CHIP_NO_ERROR);
        EXPECT_EQ(granularity, GranularityEnum::kNoTimeGranularity);
    }
}

TEST_F(TestTimeSynchronizationCluster, PlatformTimeSuccessSetsGranularity)
{
    const BitFlags<Feature> features{ Feature::kTimeSyncClient };
    TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
                                                   TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate },
                                                   context);
    ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);
    auto deferShutdown = MakeDefer([&timeSynchronization] { timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown); });
    ClusterTester tester(timeSynchronization);

    Commands::SetTrustedTimeSource::Type request;
    request.trustedTimeSource.SetNull();
    EXPECT_TRUE(tester.Invoke(request).IsSuccess());
    ASSERT_TRUE(delegate.mPlatformTimeRequest.IsPending());
    delegate.mPlatformTimeRequest.Complete(TimeSourceEnum::kAdmin, GranularityEnum::kSecondsGranularity);

    EXPECT_EQ(NextEventId(), Events::MissingTrustedTimeSource::Id);
    EXPECT_EQ(NextEventId(), kInvalidEventId);

    Granularity::TypeInfo::DecodableType granularity{};
    EXPECT_EQ(tester.ReadAttribute(Granularity::Id, granularity), CHIP_NO_ERROR);
    EXPECT_EQ(granularity, GranularityEnum::kSecondsGranularity);
}

// SetUTCTime makes a pending platform time request moot, so the cluster cancels it.
TEST_F(TestTimeSynchronizationCluster, SetUTCTimeCancelsPlatformTimeRequest)
{
    System::Clock::Internal::RAIIMockClock mockClock;

    // SetUTCTime updates the Last Known Good Time, which needs an initialized FabricTable.
    TestPersistentStorageDelegate storage;
    Credentials::PersistentStorageOpCertStore opCertStore;
    ASSERT_SUCCESS(opCertStore.Init(&storage));
    auto deferOpCertStoreFinish = MakeDefer([&opCertStore] { opCertStore.Finish(); });
    FabricTable fabricTable;
    FabricTable::InitParams initParams;
    initParams.storage     = &storage;
    initParams.opCertStore = &opCertStore;
    ASSERT_SUCCESS(fabricTable.Init(initParams));
    auto deferFabricTableShutdown = MakeDefer([&fabricTable] { fabricTable.Shutdown(); });

    const TimeSynchronizationCluster::Context contextWithFabricTable = { .fabricTable            = fabricTable,
                                                                         .caseSessionManager     = context.caseSessionManager,
                                                                         .platformManager        = context.platformManager,
                                                                         .interactionModelEngine = context.interactionModelEngine };

    const BitFlags<Feature> features{ Feature::kTimeSyncClient };
    TimeSynchronizationCluster timeSynchronization(kRootEndpointId, features, TimeSynchronizationCluster::OptionalAttributeSet(),
                                                   TimeSynchronizationCluster::StartupConfiguration{ .delegate = &delegate },
                                                   contextWithFabricTable);
    ASSERT_EQ(timeSynchronization.Startup(testContext.Get()), CHIP_NO_ERROR);
    auto deferShutdown = MakeDefer([&timeSynchronization] { timeSynchronization.Shutdown(ClusterShutdownType::kClusterShutdown); });
    ClusterTester tester(timeSynchronization);

    Commands::SetTrustedTimeSource::Type setTrustedTimeSource;
    setTrustedTimeSource.trustedTimeSource.SetNull();
    EXPECT_TRUE(tester.Invoke(setTrustedTimeSource).IsSuccess());
    EXPECT_EQ(NextEventId(), Events::MissingTrustedTimeSource::Id);
    ASSERT_TRUE(delegate.mPlatformTimeRequest.IsPending());

    Commands::SetUTCTime::Type setUTCTime;
    setUTCTime.UTCTime     = 1'000'000'000'000'000; // 2031 in the CHIP epoch, after the firmware build time
    setUTCTime.granularity = GranularityEnum::kMillisecondsGranularity;
    EXPECT_TRUE(tester.Invoke(setUTCTime).IsSuccess());
    EXPECT_FALSE(delegate.mPlatformTimeRequest.IsPending());

    Granularity::TypeInfo::DecodableType granularity{};
    EXPECT_EQ(tester.ReadAttribute(Granularity::Id, granularity), CHIP_NO_ERROR);
    EXPECT_EQ(granularity, GranularityEnum::kMillisecondsGranularity);
}
