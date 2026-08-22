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

    uint8_t mActiveScheduleHandleData[kScheduleHandleSize];
    size_t mActiveScheduleHandleSize = 0;
    bool mActiveScheduleHandleIsNull = true;
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

    // Runs a full atomic write cycle (BeginWrite -> mutate -> CommitWrite) against the Schedules attribute.
    Commands::AtomicRequest::Type MakeAtomicRequest(Globals::AtomicRequestTypeEnum requestType)
    {
        static const AttributeId kAttrs[] = { Schedules::Id };
        Commands::AtomicRequest::Type request;
        request.requestType       = requestType;
        request.attributeRequests = DataModel::List<const AttributeId>(kAttrs);
        request.timeout           = MakeOptional(static_cast<uint16_t>(3000));
        return request;
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

// NOTE: `AtomicWriteSession::BeginAtomicWrite` records the writer's identity from
// `GetSourceScopedNodeId(commandObj)`, which is derived from the command's real transport session (a CASE/group
// session peer node ID). `Testing::ClusterTester`'s `WriteAttribute()` derives its subject descriptor independently
// (not tied to a session either). `chip::Testing::MockCommandHandler::GetExchangeContext()` always returns nullptr
// by design, so the two identities can never be made to match here, and `WriteAttribute()` on `Schedules` after a
// real `AtomicRequest`/BeginWrite always observes a different "owner" and gets rejected with Busy. This is a
// pre-existing gap in the test harness shared with Presets (which has no atomic-write test coverage either), not
// specific to the Schedules feature, so the full BeginWrite -> WriteAttribute -> CommitWrite round trip isn't
// exercised here. What *is* covered below: the write-outside-atomic-write rejection, BeginWrite's wiring to
// InitializePendingSchedules(), and Precommit's built-in-schedule-removal rule (via the public
// OnAtomicWrite{Begin,Precommit} callbacks directly, which don't depend on session identity).

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
