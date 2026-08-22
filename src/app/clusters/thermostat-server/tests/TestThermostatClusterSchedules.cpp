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

#include <app/clusters/thermostat-server/ScheduleStructWithOwnedMembers.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/FabricTestFixture.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <lib/support/CodeUtils.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;

namespace {

constexpr EndpointId kTestEndpointId  = 1;
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

// A minimal Delegate double. Only the Schedule-related surface has real (array-backed) behavior; every other
// method returns a safe "empty"/no-op value since these tests do not exercise Presets or ThermostatSuggestions.
class TestDelegate : public Delegate
{
public:
    // --- Presets / Suggestions: unused by these tests, stubbed out. ---
    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(AttributeId) override
    {
        return System::Clock::Milliseconds16(3000);
    }
    CHIP_ERROR GetPresetTypeAtIndex(size_t, Structs::PresetTypeStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    uint8_t GetNumberOfPresets() override { return 0; }
    CHIP_ERROR GetPresetAtIndex(size_t, PresetStructWithOwnedMembers &) override { return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; }
    CHIP_ERROR GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & handle) override
    {
        handle.SetNull();
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetActivePresetHandle(const DataModel::Nullable<ByteSpan> &) override { return CHIP_NO_ERROR; }
    void InitializePendingPresets() override {}
    CHIP_ERROR AppendToPendingPresetList(const PresetStructWithOwnedMembers &) override { return CHIP_ERROR_WRITE_FAILED; }
    CHIP_ERROR GetPendingPresetAtIndex(size_t, PresetStructWithOwnedMembers &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR CommitPendingPresets() override { return CHIP_NO_ERROR; }
    void ClearPendingPresetList() override {}
    uint8_t GetMaxThermostatSuggestions() override { return 0; }
    uint8_t GetNumberOfThermostatSuggestions() override { return 0; }
    CHIP_ERROR GetThermostatSuggestionAtIndex(size_t, ThermostatSuggestionStructWithOwnedMembers &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    void GetCurrentThermostatSuggestion(DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & current) override
    {
        current.SetNull();
    }
    DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> GetThermostatSuggestionNotFollowingReason() override
    {
        return DataModel::NullNullable;
    }
    CHIP_ERROR AppendToThermostatSuggestionsList(const Structs::ThermostatSuggestionStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR RemoveFromThermostatSuggestionsList(size_t) override { return CHIP_ERROR_INVALID_ARGUMENT; }
    CHIP_ERROR GetUniqueID(uint8_t & uniqueID) override
    {
        uniqueID = 0;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ReEvaluateCurrentSuggestion() override { return CHIP_NO_ERROR; }

    // --- Schedules: real, array-backed behavior so the cluster's validation logic is actually exercised. ---
    CHIP_ERROR GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & scheduleType) override
    {
        if (index < MATTER_ARRAY_SIZE(mScheduleTypes))
        {
            scheduleType = mScheduleTypes[index];
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    uint8_t GetNumberOfSchedules() override { return kTestMaxSchedules; }
    uint8_t GetNumberOfScheduleTransitions() override { return kTestMaxTransitions; }
    DataModel::Nullable<uint8_t> GetNumberOfScheduleTransitionsPerDay() override { return DataModel::MakeNullable(kTestMaxPerDay); }

    CHIP_ERROR GetScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) override
    {
        if (index < mNumSchedules)
        {
            schedule = mSchedules[index];
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    CHIP_ERROR GetActiveScheduleHandle(DataModel::Nullable<MutableByteSpan> & activeScheduleHandle) override
    {
        if (mActiveScheduleHandleIsNull)
        {
            activeScheduleHandle.SetNull();
        }
        else
        {
            ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(mActiveScheduleHandleData, mActiveScheduleHandleSize),
                                                       activeScheduleHandle.Value()));
            activeScheduleHandle.Value().reduce_size(mActiveScheduleHandleSize);
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetActiveScheduleHandle(const DataModel::Nullable<ByteSpan> & newHandle) override
    {
        if (newHandle.IsNull())
        {
            mActiveScheduleHandleIsNull = true;
            mActiveScheduleHandleSize   = 0;
            return CHIP_NO_ERROR;
        }
        VerifyOrReturnError(newHandle.Value().size() <= sizeof(mActiveScheduleHandleData), CHIP_ERROR_NO_MEMORY);
        memcpy(mActiveScheduleHandleData, newHandle.Value().data(), newHandle.Value().size());
        mActiveScheduleHandleSize   = newHandle.Value().size();
        mActiveScheduleHandleIsNull = false;
        return CHIP_NO_ERROR;
    }

    void InitializePendingSchedules() override
    {
        mNumPendingSchedules = 0;
        for (uint8_t i = 0; i < mNumSchedules; i++)
        {
            mPendingSchedules[mNumPendingSchedules++] = mSchedules[i];
        }
    }

    CHIP_ERROR AppendToPendingScheduleList(const ScheduleStructWithOwnedMembers & schedule) override
    {
        VerifyOrReturnError(mNumPendingSchedules < MATTER_ARRAY_SIZE(mPendingSchedules), CHIP_ERROR_WRITE_FAILED);
        mPendingSchedules[mNumPendingSchedules] = schedule;
        if (schedule.GetScheduleHandle().IsNull())
        {
            const uint8_t handle[] = { static_cast<uint8_t>(mNextHandleValue++) };
            ReturnErrorOnFailure(
                mPendingSchedules[mNumPendingSchedules].SetScheduleHandle(DataModel::MakeNullable(ByteSpan(handle))));
        }
        mNumPendingSchedules++;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetPendingScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) override
    {
        if (index < mNumPendingSchedules)
        {
            schedule = mPendingSchedules[index];
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    CHIP_ERROR CommitPendingSchedules() override
    {
        mNumSchedules = 0;
        for (uint8_t i = 0; i < mNumPendingSchedules; i++)
        {
            mSchedules[mNumSchedules++] = mPendingSchedules[i];
        }
        return CHIP_NO_ERROR;
    }

    void ClearPendingScheduleList() override { mNumPendingSchedules = 0; }

    // --- Test-only helpers ---

    // Seeds a single built-in schedule with the given handle byte and no transitions, so PrecommitSchedules'
    // "built-in schedules cannot be silently removed" rule has something to exercise.
    void SeedBuiltInSchedule(uint8_t handleByte)
    {
        ASSERT_LT(mNumSchedules, MATTER_ARRAY_SIZE(mSchedules));
        ScheduleStructWithOwnedMembers schedule;
        schedule.SetSystemMode(SystemModeEnum::kHeat);
        const uint8_t handle[] = { handleByte };
        ASSERT_EQ(schedule.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(handle))), CHIP_NO_ERROR);
        ASSERT_EQ(schedule.SetName(NullOptional), CHIP_NO_ERROR);
        ASSERT_EQ(schedule.SetPresetHandle(NullOptional), CHIP_NO_ERROR);
        schedule.SetBuiltIn(DataModel::MakeNullable(true));
        mSchedules[mNumSchedules++] = schedule;
    }

private:
    Structs::ScheduleTypeStruct::Type mScheduleTypes[1] = { {
        .systemMode           = SystemModeEnum::kHeat,
        .numberOfSchedules    = kTestMaxSchedules,
        .scheduleTypeFeatures = static_cast<uint16_t>(to_underlying(ScheduleTypeFeaturesBitmap::kSupportsNames) |
                                                      to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints)),
    } };

    ScheduleStructWithOwnedMembers mSchedules[kTestMaxSchedules];
    ScheduleStructWithOwnedMembers mPendingSchedules[kTestMaxSchedules];
    uint8_t mNumSchedules        = 0;
    uint8_t mNumPendingSchedules = 0;
    uint8_t mNextHandleValue     = 100;

    uint8_t mActiveScheduleHandleData[kScheduleHandleSize] = { 0 };
    size_t mActiveScheduleHandleSize                       = 0;
    bool mActiveScheduleHandleIsNull                       = true;
};

struct TestThermostatClusterSchedules : public ::testing::Test
{
    // AtomicRequest's BeginWrite schedules a real timer (to auto-expire the write) via DeviceLayer::SystemLayer(),
    // so the device layer must be initialized for BeginWrite to actually open the write instead of silently
    // rolling back due to a timer-scheduling failure.
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }

    TestServerClusterContext mTestContext;
    FabricTestFixture mFabricHelper{ &mTestContext.StorageDelegate() };

    TestDelegate mDelegate;
    ThermostatCluster mCluster{ kTestEndpointId,
                                BitFlags<Thermostat::Feature>(Feature::kHeating, Feature::kMatterScheduleConfiguration),
                                {},
                                {},
                                mFabricHelper.GetFabricTable() };
    Testing::ClusterTester mTester{ mCluster };
    FabricIndex mFabricIndex = 1;

    void SetUp() override
    {
        mCluster.SetDelegate(&mDelegate);
        ASSERT_EQ(mCluster.Startup(mTestContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(mFabricHelper.SetUpTestFabric(mFabricIndex), CHIP_NO_ERROR);
        mTester.SetFabricIndex(mFabricIndex);
    }

    void TearDown() override
    {
        mTester.SetFabricIndex(kUndefinedFabricIndex);
        mCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        ASSERT_EQ(mFabricHelper.TearDownTestFabric(mFabricIndex), CHIP_NO_ERROR);
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

    // `AtomicWriteSession::BeginAtomicWrite` records the write session's owning identity via
    // `GetSourceScopedNodeId(commandObj)`, which reads the command's real exchange context.
    // `chip::Testing::MockCommandHandler::GetExchangeContext()` always returns nullptr, so that identity is
    // always the default `ScopedNodeId()` (NodeId 0 / FabricIndex 0), no matter what subject descriptor
    // `MockCommandHandler` otherwise carries. `WriteAttribute()`'s `InAtomicWrite(subjectDescriptor, ...)`
    // check only requires CASE auth mode plus a subject/fabricIndex pair that maps to that same default
    // `ScopedNodeId()`, so a subject descriptor with CASE auth mode and the (default) zero subject/fabricIndex
    // makes the two identities match. Call this after BeginWrite to drive the full
    // `BeginWrite -> WriteAttribute -> CommitWrite` pipeline in a test.
    void UseAtomicWriteOwnerIdentity()
    {
        Access::SubjectDescriptor identity;
        identity.authMode = Access::AuthMode::kCase;
        mTester.SetSubjectDescriptor(identity);
    }
};

TEST_F(TestThermostatClusterSchedules, SchedulesAttributePresentWhenFeatureEnabled)
{
    EXPECT_TRUE(mCluster.HasAttribute(Schedules::Id));
    EXPECT_TRUE(mCluster.HasAttribute(ScheduleTypes::Id));
    EXPECT_TRUE(mCluster.HasAttribute(NumberOfSchedules::Id));
    EXPECT_TRUE(mCluster.HasAttribute(ActiveScheduleHandle::Id));
}

TEST_F(TestThermostatClusterSchedules, ReadNumberOfSchedulesAndTransitions)
{
    uint8_t numberOfSchedules;
    ASSERT_EQ(mTester.ReadAttribute(NumberOfSchedules::Id, numberOfSchedules), CHIP_NO_ERROR);
    EXPECT_EQ(numberOfSchedules, kTestMaxSchedules);

    uint8_t numberOfTransitions;
    ASSERT_EQ(mTester.ReadAttribute(NumberOfScheduleTransitions::Id, numberOfTransitions), CHIP_NO_ERROR);
    EXPECT_EQ(numberOfTransitions, kTestMaxTransitions);

    DataModel::Nullable<uint8_t> perDay;
    ASSERT_EQ(mTester.ReadAttribute(NumberOfScheduleTransitionPerDay::Id, perDay), CHIP_NO_ERROR);
    ASSERT_FALSE(perDay.IsNull());
    EXPECT_EQ(perDay.Value(), kTestMaxPerDay);
}

TEST_F(TestThermostatClusterSchedules, ActiveScheduleHandleDefaultsToNull)
{
    DataModel::Nullable<ByteSpan> activeHandle;
    ASSERT_EQ(mTester.ReadAttribute(ActiveScheduleHandle::Id, activeHandle), CHIP_NO_ERROR);
    EXPECT_TRUE(activeHandle.IsNull());
}

TEST_F(TestThermostatClusterSchedules, SetActiveScheduleRequestWithUnknownHandleIsInvalidCommand)
{
    Commands::SetActiveScheduleRequest::Type request;
    const uint8_t handle[] = { 0x42 };
    request.scheduleHandle = ByteSpan(handle);

    auto result = mTester.Invoke(request);
    ASSERT_TRUE(result.status.has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::InvalidCommand);
}

TEST_F(TestThermostatClusterSchedules, WriteToSchedulesOutsideAtomicWriteIsRejected)
{
    ScheduleStruct::Type schedule;
    schedule.scheduleHandle = DataModel::NullNullable;
    schedule.systemMode     = SystemModeEnum::kHeat;
    schedule.name           = NullOptional;
    schedule.presetHandle   = NullOptional;
    schedule.builtIn        = DataModel::NullNullable;
    schedule.transitions    = DataModel::List<const ScheduleTransitionStruct::Type>();

    ScheduleStruct::Type list[] = { schedule };
    auto writeStatus            = mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list),
                                                         Testing::ListWritingPattern::ClearAllThenAppendItems);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(InvalidInState));
}

TEST_F(TestThermostatClusterSchedules, BeginWriteSeedsPendingSchedulesFromExistingList)
{
    mDelegate.SeedBuiltInSchedule(0x01);

    auto beginResult = mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite));
    ASSERT_TRUE(beginResult.IsSuccess());

    ScheduleStructWithOwnedMembers pending;
    ASSERT_EQ(mDelegate.GetPendingScheduleAtIndex(0, pending), CHIP_NO_ERROR);
    EXPECT_EQ(pending.GetSystemMode(), SystemModeEnum::kHeat);
    ASSERT_FALSE(pending.GetScheduleHandle().IsNull());
    const uint8_t expectedHandle[] = { 0x01 };
    EXPECT_TRUE(pending.GetScheduleHandle().Value().data_equal(ByteSpan(expectedHandle)));
}

TEST_F(TestThermostatClusterSchedules, PrecommitFailsWhenBuiltInScheduleIsRemoved)
{
    mDelegate.SeedBuiltInSchedule(0x01);

    ASSERT_EQ(mCluster.OnAtomicWriteBegin(Schedules::Id), Status::Success);
    // Simulate the client clearing the pending list without re-adding the built-in schedule.
    mDelegate.ClearPendingScheduleList();

    EXPECT_EQ(mCluster.OnAtomicWritePrecommit(Schedules::Id), Status::ConstraintError);
}

TEST_F(TestThermostatClusterSchedules, PrecommitSucceedsWhenBuiltInScheduleIsPreserved)
{
    mDelegate.SeedBuiltInSchedule(0x01);

    ASSERT_EQ(mCluster.OnAtomicWriteBegin(Schedules::Id), Status::Success);
    // InitializePendingSchedules (called by OnAtomicWriteBegin) already copied the built-in schedule into the
    // pending list, so it is preserved by default.
    EXPECT_EQ(mCluster.OnAtomicWritePrecommit(Schedules::Id), Status::Success);
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleSucceedsAndAddsToPendingList)
{
    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    UseAtomicWriteOwnerIdentity();

    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::NullNullable, DataModel::NullNullable, NullOptional,
                                                 DataModel::List<const ScheduleTransitionStruct::Type>()) };
    auto writeStatus =
        mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), Testing::ListWritingPattern::ReplaceAll);
    EXPECT_TRUE(writeStatus.IsSuccess());

    ScheduleStructWithOwnedMembers pending;
    ASSERT_EQ(mDelegate.GetPendingScheduleAtIndex(0, pending), CHIP_NO_ERROR);
    // A null handle on input gets a handle auto-assigned by the delegate.
    EXPECT_FALSE(pending.GetScheduleHandle().IsNull());
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsOversizedHandle)
{
    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    UseAtomicWriteOwnerIdentity();

    uint8_t oversizedHandle[kScheduleHandleSize + 1] = {};
    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::MakeNullable(ByteSpan(oversizedHandle)), DataModel::NullNullable,
                                                 NullOptional, DataModel::List<const ScheduleTransitionStruct::Type>()) };
    auto writeStatus =
        mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), Testing::ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsOversizedName)
{
    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    UseAtomicWriteOwnerIdentity();

    char oversizedName[kScheduleNameSize + 1];
    memset(oversizedName, 'a', sizeof(oversizedName));
    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::NullNullable, DataModel::NullNullable,
                                                 MakeOptional(CharSpan(oversizedName, sizeof(oversizedName))),
                                                 DataModel::List<const ScheduleTransitionStruct::Type>()) };
    auto writeStatus =
        mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), Testing::ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsWhenTotalTransitionsExceedMaximum)
{
    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    UseAtomicWriteOwnerIdentity();

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
        mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), Testing::ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsWhenPerDayTransitionLimitExceeded)
{
    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    UseAtomicWriteOwnerIdentity();

    // kTestMaxPerDay == 2; 3 transitions on the same day exceeds the per-day limit.
    ScheduleTransitionStruct::Type transitions[] = {
        MakeScheduleTransition(0, ScheduleDayOfWeekBitmap::kMonday),
        MakeScheduleTransition(100, ScheduleDayOfWeekBitmap::kMonday),
        MakeScheduleTransition(200, ScheduleDayOfWeekBitmap::kMonday),
    };
    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::NullNullable, DataModel::NullNullable, NullOptional,
                                                 DataModel::List<const ScheduleTransitionStruct::Type>(transitions)) };
    auto writeStatus =
        mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), Testing::ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsUnknownScheduleHandle)
{
    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    UseAtomicWriteOwnerIdentity();

    // Handle doesn't match any entry in the (empty) Schedules attribute list.
    const uint8_t unknownHandle[] = { 0xFF };
    ScheduleStruct::Type list[]   = { MakeSchedule(DataModel::MakeNullable(ByteSpan(unknownHandle)), DataModel::NullNullable,
                                                   NullOptional, DataModel::List<const ScheduleTransitionStruct::Type>()) };
    auto writeStatus =
        mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), Testing::ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(NotFound));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsBuiltInMismatch)
{
    mDelegate.SeedBuiltInSchedule(0x01);

    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    UseAtomicWriteOwnerIdentity();

    // The committed schedule 0x01 is built-in; claiming it is not is a constraint violation.
    const uint8_t handle[]      = { 0x01 };
    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::MakeNullable(ByteSpan(handle)), DataModel::MakeNullable(false),
                                                 NullOptional, DataModel::List<const ScheduleTransitionStruct::Type>()) };
    auto writeStatus =
        mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), Testing::ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsDuplicatePendingHandle)
{
    mDelegate.SeedBuiltInSchedule(0x01);

    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    UseAtomicWriteOwnerIdentity();

    // Both list items reference the same existing schedule handle: ReplaceAll clears the pending list and then
    // appends the items one by one, so the first append succeeds and the second finds a pending entry with that
    // handle already there.
    const uint8_t handle[]      = { 0x01 };
    ScheduleStruct::Type item   = MakeSchedule(DataModel::MakeNullable(ByteSpan(handle)), DataModel::NullNullable, NullOptional,
                                               DataModel::List<const ScheduleTransitionStruct::Type>());
    ScheduleStruct::Type list[] = { item, item };
    auto writeStatus =
        mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), Testing::ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsWhenScheduleCountExceedsMaximum)
{
    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    UseAtomicWriteOwnerIdentity();

    // kTestMaxSchedules == 2; a single ReplaceAll write with 3 new schedules exceeds it on the third entry.
    ScheduleStruct::Type newSchedule = MakeSchedule(DataModel::NullNullable, DataModel::NullNullable, NullOptional,
                                                    DataModel::List<const ScheduleTransitionStruct::Type>());
    ScheduleStruct::Type list[]      = { newSchedule, newSchedule, newSchedule };
    auto writeStatus =
        mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), Testing::ListWritingPattern::ReplaceAll);
    EXPECT_EQ(writeStatus, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
}

TEST_F(TestThermostatClusterSchedules, FullAtomicWriteRoundTripAppendsAndCommitsNewSchedule)
{
    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    UseAtomicWriteOwnerIdentity();

    ScheduleStruct::Type list[] = { MakeSchedule(DataModel::NullNullable, DataModel::NullNullable, NullOptional,
                                                 DataModel::List<const ScheduleTransitionStruct::Type>()) };
    ASSERT_TRUE(
        mTester.WriteAttribute(Schedules::Id, DataModel::List<ScheduleStruct::Type>(list), Testing::ListWritingPattern::ReplaceAll)
            .IsSuccess());

    ASSERT_EQ(mCluster.OnAtomicWritePrecommit(Schedules::Id), Status::Success);
    ASSERT_EQ(mCluster.OnAtomicWriteCommit(Schedules::Id), Status::Success);

    ScheduleStructWithOwnedMembers committed;
    ASSERT_EQ(mDelegate.GetScheduleAtIndex(0, committed), CHIP_NO_ERROR);
    EXPECT_FALSE(committed.GetScheduleHandle().IsNull());
}

TEST_F(TestThermostatClusterSchedules, OnAtomicWriteCommitCommitsPendingSchedulesAndNotifies)
{
    mDelegate.SeedBuiltInSchedule(0x01);

    ASSERT_EQ(mCluster.OnAtomicWriteBegin(Schedules::Id), Status::Success);
    // InitializePendingSchedules (called by OnAtomicWriteBegin) already seeded the pending list from the
    // existing schedule, so committing without further changes should succeed and leave it unchanged.
    ASSERT_EQ(mCluster.OnAtomicWritePrecommit(Schedules::Id), Status::Success);

    // mCluster is started against mTestContext (not mTester's own internal context, which FabricTestFixture
    // doesn't share), so dirty-attribute tracking has to be observed through mTestContext's listener rather
    // than mTester.IsAttributeDirty().
    const auto & clusterPath = mCluster.GetPaths()[0];
    ConcreteAttributePath schedulesPath(clusterPath.mEndpointId, clusterPath.mClusterId, Schedules::Id);
    EXPECT_FALSE(mTestContext.ChangeListener().IsDirty(schedulesPath));
    EXPECT_EQ(mCluster.OnAtomicWriteCommit(Schedules::Id), Status::Success);
    EXPECT_TRUE(mTestContext.ChangeListener().IsDirty(schedulesPath));

    ScheduleStructWithOwnedMembers committed;
    ASSERT_EQ(mDelegate.GetScheduleAtIndex(0, committed), CHIP_NO_ERROR);
    ASSERT_FALSE(committed.GetScheduleHandle().IsNull());
    const uint8_t expectedHandle[] = { 0x01 };
    EXPECT_TRUE(committed.GetScheduleHandle().Value().data_equal(ByteSpan(expectedHandle)));
}

TEST_F(TestThermostatClusterSchedules, RollbackWriteClearsPendingScheduleList)
{
    mDelegate.SeedBuiltInSchedule(0x01);

    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kBeginWrite)).IsSuccess());
    // BeginWrite's InitializePendingSchedules() seeded the pending list from the existing schedule.
    ScheduleStructWithOwnedMembers pendingBeforeRollback;
    ASSERT_EQ(mDelegate.GetPendingScheduleAtIndex(0, pendingBeforeRollback), CHIP_NO_ERROR);

    ASSERT_TRUE(mTester.Invoke(MakeAtomicRequest(Globals::AtomicRequestTypeEnum::kRollbackWrite)).IsSuccess());

    ScheduleStructWithOwnedMembers pendingAfterRollback;
    EXPECT_EQ(mDelegate.GetPendingScheduleAtIndex(0, pendingAfterRollback), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
}

} // namespace
