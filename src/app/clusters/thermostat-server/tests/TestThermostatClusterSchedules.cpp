/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ThermostatTestCommon.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/clusters/thermostat-server/ScheduleStructWithOwnedMembers.h>
#include <clusters/Thermostat/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;
using chip::app::Clusters::Globals::AtomicRequestTypeEnum;

namespace {

constexpr uint8_t kTestMaxSchedules   = 2;
constexpr uint8_t kTestMaxTransitions = 4;
constexpr uint8_t kTestMaxPerDay      = 2;

ScheduleTransitionStruct::Type MakeScheduleTransition(uint16_t transitionTime, ScheduleDayOfWeekBitmap day)
{
    ScheduleTransitionStruct::Type transition;
    transition.transitionTime = transitionTime;
    transition.dayOfWeek      = BitMask<ScheduleDayOfWeekBitmap>(day);
    return transition;
}

ScheduleStruct::Type MakeSchedule(DataModel::Nullable<ByteSpan> handle, DataModel::Nullable<bool> builtIn, Optional<CharSpan> name,
                                  DataModel::List<const ScheduleTransitionStruct::Type> transitions)
{
    ScheduleStruct::Type schedule;
    schedule.scheduleHandle = handle;
    schedule.systemMode     = SystemModeEnum::kHeat;
    schedule.name           = name;
    schedule.presetHandle   = NullOptional;
    schedule.builtIn        = builtIn;
    schedule.transitions    = transitions;
    return schedule;
}

// Builds an AtomicRequest payload of the given request type targeting the Schedules attribute.
Commands::AtomicRequest::Type MakeAtomicRequest(Globals::AtomicRequestTypeEnum requestType)
{
    static const AttributeId kAttrs[] = { Schedules::Id };
    Commands::AtomicRequest::Type request;
    request.requestType       = requestType;
    request.attributeRequests = DataModel::List<const AttributeId>(kAttrs);
    request.timeout           = MakeOptional(static_cast<uint16_t>(3000));
    return request;
}

struct ThermostatSchedulesTestFixture : public ThermostatTestFixture
{
    void SetUp() override
    {
        ThermostatTestFixture::SetUp();

        mSchedulesDelegate.mMaxSchedules                 = kTestMaxSchedules;
        mSchedulesDelegate.mMaxScheduleTransitions       = kTestMaxTransitions;
        mSchedulesDelegate.mMaxScheduleTransitionsPerDay = DataModel::MakeNullable(kTestMaxPerDay);
        mSchedulesDelegate.mScheduleTypes.push_back({
            .systemMode           = SystemModeEnum::kHeat,
            .numberOfSchedules    = kTestMaxSchedules,
            .scheduleTypeFeatures = static_cast<uint16_t>(to_underlying(ScheduleTypeFeaturesBitmap::kSupportsNames) |
                                                          to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints)),
        });
    }

    BitFlags<Feature> Features() { return BitFlags<Feature>(Feature::kHeating, Feature::kMatterScheduleConfiguration); }
};

TEST_F(ThermostatSchedulesTestFixture, SchedulesAttributePresentWhenFeatureEnabled)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.HasAttribute(Schedules::Id));
    EXPECT_TRUE(cluster.HasAttribute(ScheduleTypes::Id));
    EXPECT_TRUE(cluster.HasAttribute(NumberOfSchedules::Id));
    EXPECT_TRUE(cluster.HasAttribute(ActiveScheduleHandle::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, ReadNumberOfSchedulesAndTransitions)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint8_t numberOfSchedules = 0;
    EXPECT_EQ(tester.ReadAttribute(NumberOfSchedules::Id, numberOfSchedules), Status::Success);
    EXPECT_EQ(numberOfSchedules, kTestMaxSchedules);

    uint8_t numberOfTransitions = 0;
    EXPECT_EQ(tester.ReadAttribute(NumberOfScheduleTransitions::Id, numberOfTransitions), Status::Success);
    EXPECT_EQ(numberOfTransitions, kTestMaxTransitions);

    DataModel::Nullable<uint8_t> perDay;
    EXPECT_EQ(tester.ReadAttribute(NumberOfScheduleTransitionPerDay::Id, perDay), Status::Success);
    ASSERT_FALSE(perDay.IsNull());
    EXPECT_EQ(perDay.Value(), kTestMaxPerDay);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, ActiveScheduleHandleDefaultsToNull)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<ByteSpan> activeHandle;
    EXPECT_EQ(tester.ReadAttribute(ActiveScheduleHandle::Id, activeHandle), Status::Success);
    EXPECT_TRUE(activeHandle.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, SetActiveScheduleRequestWithUnknownHandleIsInvalidCommand)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::SetActiveScheduleRequest::Type request;
    const uint8_t handle[] = { 0x42 };
    request.scheduleHandle = ByteSpan(handle);

    auto result = tester.Invoke(request);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidCommand));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, SetActiveScheduleRequestSucceedsAndGeneratesEvent)
{
    mSchedulesDelegate.SeedBuiltInSchedule(0x01);

    ThermostatCluster cluster(kTestEndpointId,
                              BitFlags<Feature>(Feature::kHeating, Feature::kMatterScheduleConfiguration, Feature::kEvents),
                              MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::SetActiveScheduleRequest::Type request;
    const uint8_t handle[] = { 0x01 };
    request.scheduleHandle = ByteSpan(handle);

    auto result = tester.Invoke(request);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(tester.IsAttributeDirty(ActiveScheduleHandle::Id));

    auto event = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    if (event.has_value())
    {
        EXPECT_EQ(event.value().eventOptions.mPath.mEventId, Events::ActiveScheduleChange::Id);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, WriteToSchedulesOutsideAtomicWriteIsRejected)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::NullNullable, DataModel::NullNullable, NullOptional,
                                                 DataModel::List<const ScheduleTransitionStruct::Type>()) };
    EXPECT_EQ(tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll),
              Status::InvalidInState);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, BeginWriteSeedsPendingSchedulesFromExistingList)
{
    mSchedulesDelegate.SeedBuiltInSchedule(0x01);

    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    auto beginResult = tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite));
    ASSERT_TRUE(beginResult.IsSuccess());

    ASSERT_EQ(mSchedulesDelegate.mPendingSchedules.size(), 1u);
    const auto & pending = mSchedulesDelegate.mPendingSchedules[0];
    EXPECT_EQ(pending.GetSystemMode(), SystemModeEnum::kHeat);
    ASSERT_FALSE(pending.GetScheduleHandle().IsNull());
    const uint8_t expectedHandle[] = { 0x01 };
    EXPECT_TRUE(pending.GetScheduleHandle().Value().data_equal(ByteSpan(expectedHandle)));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, PrecommitFailsWhenBuiltInScheduleIsRemoved)
{
    mSchedulesDelegate.SeedBuiltInSchedule(0x01);

    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.OnAtomicWriteBegin(Schedules::Id), Status::Success);
    // Simulate the client clearing the pending list without re-adding the built-in schedule.
    mSchedulesDelegate.ClearPendingScheduleList();

    EXPECT_EQ(cluster.OnAtomicWritePrecommit(Schedules::Id), Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, PrecommitSucceedsWhenBuiltInScheduleIsPreserved)
{
    mSchedulesDelegate.SeedBuiltInSchedule(0x01);

    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.OnAtomicWriteBegin(Schedules::Id), Status::Success);
    // OnAtomicWriteBegin's InitializePendingSchedules() already copied the built-in schedule into the pending
    // list, so it is preserved by default.
    EXPECT_EQ(cluster.OnAtomicWritePrecommit(Schedules::Id), Status::Success);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, AppendPendingScheduleSucceedsAndAddsToPendingList)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());

    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::NullNullable, DataModel::NullNullable, NullOptional,
                                                 DataModel::List<const ScheduleTransitionStruct::Type>()) };
    auto writeStatus =
        tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll);
    EXPECT_TRUE(writeStatus.IsSuccess());

    ASSERT_EQ(mSchedulesDelegate.mPendingSchedules.size(), 1u);
    // A null handle on input gets a handle auto-assigned by the delegate.
    EXPECT_FALSE(mSchedulesDelegate.mPendingSchedules[0].GetScheduleHandle().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, AppendPendingScheduleRejectsOversizedHandle)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());

    uint8_t oversizedHandle[kScheduleHandleSize + 1] = {};
    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::MakeNullable(ByteSpan(oversizedHandle)), DataModel::NullNullable,
                                                 NullOptional, DataModel::List<const ScheduleTransitionStruct::Type>()) };
    auto writeStatus =
        tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, AppendPendingScheduleRejectsOversizedName)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());

    char oversizedName[kScheduleNameSize + 1];
    memset(oversizedName, 'a', sizeof(oversizedName));
    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::NullNullable, DataModel::NullNullable,
                                                 MakeOptional(CharSpan(oversizedName, sizeof(oversizedName))),
                                                 DataModel::List<const ScheduleTransitionStruct::Type>()) };
    auto writeStatus =
        tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, AppendPendingScheduleRejectsWhenTotalTransitionsExceedMaximum)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());

    // kTestMaxTransitions == 4; 5 transitions on 5 different days stays under the per-day limit (2) but exceeds
    // the delegate's total transition budget.
    ScheduleTransitionStruct::Type transitions[] = {
        MakeScheduleTransition(0, ScheduleDayOfWeekBitmap::kSunday),
        MakeScheduleTransition(0, ScheduleDayOfWeekBitmap::kMonday),
        MakeScheduleTransition(0, ScheduleDayOfWeekBitmap::kTuesday),
        MakeScheduleTransition(0, ScheduleDayOfWeekBitmap::kWednesday),
        MakeScheduleTransition(0, ScheduleDayOfWeekBitmap::kThursday),
    };
    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::NullNullable, DataModel::NullNullable, NullOptional,
                                                 DataModel::List<const ScheduleTransitionStruct::Type>(transitions)) };
    auto writeStatus =
        tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, AppendPendingScheduleRejectsWhenPerDayTransitionLimitExceeded)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());

    // kTestMaxPerDay == 2; 3 transitions on the same day exceeds the per-day limit.
    ScheduleTransitionStruct::Type transitions[] = {
        MakeScheduleTransition(0, ScheduleDayOfWeekBitmap::kMonday),
        MakeScheduleTransition(100, ScheduleDayOfWeekBitmap::kMonday),
        MakeScheduleTransition(200, ScheduleDayOfWeekBitmap::kMonday),
    };
    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::NullNullable, DataModel::NullNullable, NullOptional,
                                                 DataModel::List<const ScheduleTransitionStruct::Type>(transitions)) };
    auto writeStatus =
        tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, AppendPendingScheduleRejectsUnknownScheduleHandle)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());

    // Handle doesn't match any entry in the (empty) Schedules attribute list.
    const uint8_t unknownHandle[] = { 0xFF };
    ScheduleStruct::Type list[]   = { MakeSchedule(DataModel::MakeNullable(ByteSpan(unknownHandle)), DataModel::NullNullable,
                                                   NullOptional, DataModel::List<const ScheduleTransitionStruct::Type>()) };
    auto writeStatus =
        tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(NotFound));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, AppendPendingScheduleRejectsBuiltInMismatch)
{
    mSchedulesDelegate.SeedBuiltInSchedule(0x01);

    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());

    // The committed schedule 0x01 is built-in; claiming it is not is a constraint violation.
    const uint8_t handle[]      = { 0x01 };
    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::MakeNullable(ByteSpan(handle)), DataModel::MakeNullable(false),
                                                 NullOptional, DataModel::List<const ScheduleTransitionStruct::Type>()) };
    auto writeStatus =
        tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, AppendPendingScheduleRejectsDuplicatePendingHandle)
{
    mSchedulesDelegate.SeedBuiltInSchedule(0x01);

    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());

    // Both list items reference the same existing schedule handle: ReplaceAll clears the pending list and then
    // appends the items one by one, so the first append succeeds and the second finds a pending entry with that
    // handle already there.
    const uint8_t handle[]      = { 0x01 };
    ScheduleStruct::Type item   = MakeSchedule(DataModel::MakeNullable(ByteSpan(handle)), DataModel::NullNullable, NullOptional,
                                               DataModel::List<const ScheduleTransitionStruct::Type>());
    ScheduleStruct::Type list[] = { item, item };
    auto writeStatus =
        tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, AppendPendingScheduleRejectsWhenScheduleCountExceedsMaximum)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());

    // kTestMaxSchedules == 2; a single ReplaceAll write with 3 new schedules exceeds it on the third entry.
    ScheduleStruct::Type newSchedule = MakeSchedule(DataModel::NullNullable, DataModel::NullNullable, NullOptional,
                                                    DataModel::List<const ScheduleTransitionStruct::Type>());
    ScheduleStruct::Type list[]      = { newSchedule, newSchedule, newSchedule };
    auto writeStatus =
        tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, FullAtomicWriteRoundTripAppendsAndCommitsNewSchedule)
{
    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());

    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::NullNullable, DataModel::NullNullable, NullOptional,
                                                 DataModel::List<const ScheduleTransitionStruct::Type>()) };
    ASSERT_TRUE(tester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), ListWritingPattern::ReplaceAll)
                    .IsSuccess());

    auto commitResult = tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kCommitWrite));
    EXPECT_TRUE(commitResult.IsSuccess());

    ASSERT_EQ(mSchedulesDelegate.mSchedules.size(), 1u);
    EXPECT_FALSE(mSchedulesDelegate.mSchedules[0].GetScheduleHandle().IsNull());
    EXPECT_TRUE(tester.IsAttributeDirty(Schedules::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatSchedulesTestFixture, RollbackWriteClearsPendingScheduleList)
{
    mSchedulesDelegate.SeedBuiltInSchedule(0x01);

    ThermostatCluster cluster(kTestEndpointId, Features(), MakeConfig(), mThermostatDelegate, mHeatingDelegate, mSchedulesDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    // BeginWrite's InitializePendingSchedules() seeded the pending list from the existing schedule.
    ASSERT_EQ(mSchedulesDelegate.mPendingSchedules.size(), 1u);

    ASSERT_TRUE(tester.Invoke(MakeAtomicRequest(AtomicRequestTypeEnum::kRollbackWrite)).IsSuccess());

    EXPECT_TRUE(mSchedulesDelegate.mPendingSchedules.empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
