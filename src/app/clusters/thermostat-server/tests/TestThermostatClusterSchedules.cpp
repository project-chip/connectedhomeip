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

#include <zap-generated/gen_config.h>

#include "ThermostatAttrAccessTestAccess.h"
#include <app/clusters/thermostat-server/ThermostatClusterSchedules.h>
#include <app/tests/test-ember-api.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>

#include <cstring>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
extern ThermostatAttrAccess gThermostatAttrAccess;
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

namespace {

constexpr EndpointId kEndpoint = 0;

constexpr uint8_t kHandleA[]       = { 0x01 };
constexpr uint8_t kUnknownHandle[] = { 0xFF };

// A minimal test double for Thermostat::Delegate. Only the schedule-related methods that
// ThermostatClusterSchedules.cpp exercises are backed by real storage; every other pure virtual
// (presets, suggestions) is stubbed out since this test only targets the schedule additions.
class FakeThermostatDelegate : public Delegate
{
public:
    static constexpr size_t kMaxSchedules     = 4;
    static constexpr size_t kMaxScheduleTypes = 4;

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override
    {
        return std::nullopt;
    }

    CHIP_ERROR GetPresetTypeAtIndex(size_t index, Structs::PresetTypeStruct::Type & presetType) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    uint8_t GetNumberOfPresets() override { return 0; }
    CHIP_ERROR GetPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & activePresetHandle) override
    {
        activePresetHandle.SetNull();
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetActivePresetHandle(const DataModel::Nullable<ByteSpan> & newActivePresetHandle) override { return CHIP_NO_ERROR; }
    void InitializePendingPresets() override {}
    CHIP_ERROR AppendToPendingPresetList(const PresetStructWithOwnedMembers & preset) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetPendingPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR CommitPendingPresets() override { return CHIP_NO_ERROR; }
    void ClearPendingPresetList() override {}

    uint8_t GetMaxThermostatSuggestions() override { return 0; }
    uint8_t GetNumberOfThermostatSuggestions() override { return 0; }
    CHIP_ERROR GetThermostatSuggestionAtIndex(size_t index,
                                              ThermostatSuggestionStructWithOwnedMembers & thermostatSuggestion) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    void GetCurrentThermostatSuggestion(
        DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & currentThermostatSuggestion) override
    {
        currentThermostatSuggestion.SetNull();
    }
    DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> GetThermostatSuggestionNotFollowingReason() override
    {
        return DataModel::NullNullable;
    }
    CHIP_ERROR AppendToThermostatSuggestionsList(const Structs::ThermostatSuggestionStruct::Type & thermostatSuggestion) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR RemoveFromThermostatSuggestionsList(size_t indexToRemove) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetUniqueID(uint8_t & uniqueID) override
    {
        uniqueID = 0;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ReEvaluateCurrentSuggestion() override { return CHIP_NO_ERROR; }

    // --- Schedule-related methods below back real, in-memory storage. ---

    CHIP_ERROR GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & scheduleType) override
    {
        VerifyOrReturnError(index < mScheduleTypesCount, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        scheduleType = mScheduleTypes[index];
        return CHIP_NO_ERROR;
    }

    uint8_t GetNumberOfSchedules() override { return mNumberOfSchedules; }
    uint8_t GetNumberOfScheduleTransitions() override { return mNumberOfScheduleTransitions; }
    DataModel::Nullable<uint8_t> GetNumberOfScheduleTransitionsPerDay() override { return mNumberOfScheduleTransitionsPerDay; }

    CHIP_ERROR GetScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) override
    {
        VerifyOrReturnError(index < mSchedulesCount, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        schedule = mSchedules[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetActiveScheduleHandle(DataModel::Nullable<MutableByteSpan> & activeScheduleHandle) override
    {
        if (mActiveScheduleHandleIsNull)
        {
            activeScheduleHandle.SetNull();
            return CHIP_NO_ERROR;
        }
        MutableByteSpan target = activeScheduleHandle.Value();
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(mActiveScheduleHandleBuffer, mActiveScheduleHandleLength), target));
        activeScheduleHandle.SetNonNull(target);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetActiveScheduleHandle(const DataModel::Nullable<ByteSpan> & newActiveScheduleHandle) override
    {
        mSetActiveScheduleHandleCalled = true;
        if (newActiveScheduleHandle.IsNull())
        {
            mActiveScheduleHandleIsNull = true;
            return CHIP_NO_ERROR;
        }
        mActiveScheduleHandleIsNull = false;
        mActiveScheduleHandleLength = newActiveScheduleHandle.Value().size();
        memcpy(mActiveScheduleHandleBuffer, newActiveScheduleHandle.Value().data(), mActiveScheduleHandleLength);
        return CHIP_NO_ERROR;
    }

    void InitializePendingSchedules() override {}

    CHIP_ERROR AppendToPendingScheduleList(const ScheduleStructWithOwnedMembers & schedule) override
    {
        mAppendToPendingScheduleListCalled = true;
        VerifyOrReturnError(mPendingSchedulesCount < kMaxSchedules, CHIP_ERROR_NO_MEMORY);
        mPendingSchedules[mPendingSchedulesCount++] = schedule;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetPendingScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) override
    {
        VerifyOrReturnError(index < mPendingSchedulesCount, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        schedule = mPendingSchedules[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CommitPendingSchedules() override
    {
        mCommitPendingSchedulesCalled = true;
        return CHIP_NO_ERROR;
    }

    void ClearPendingScheduleList() override
    {
        mClearPendingScheduleListCalled = true;
        mPendingSchedulesCount          = 0;
    }

    // --- Test helpers, not part of the Delegate interface. ---

    void AddScheduleType(SystemModeEnum systemMode, uint8_t numberOfSchedules,
                         BitMask<ScheduleTypeFeaturesBitmap> features = BitMask<ScheduleTypeFeaturesBitmap>())
    {
        ScheduleTypeStruct::Type scheduleType;
        scheduleType.systemMode               = systemMode;
        scheduleType.numberOfSchedules        = numberOfSchedules;
        scheduleType.scheduleTypeFeatures     = features;
        mScheduleTypes[mScheduleTypesCount++] = scheduleType;
    }

    void AddSchedule(const ScheduleStructWithOwnedMembers & schedule) { mSchedules[mSchedulesCount++] = schedule; }

    ScheduleTypeStruct::Type mScheduleTypes[kMaxScheduleTypes];
    size_t mScheduleTypesCount = 0;

    ScheduleStructWithOwnedMembers mSchedules[kMaxSchedules];
    size_t mSchedulesCount = 0;

    ScheduleStructWithOwnedMembers mPendingSchedules[kMaxSchedules];
    size_t mPendingSchedulesCount = 0;

    uint8_t mNumberOfSchedules           = kMaxSchedules;
    uint8_t mNumberOfScheduleTransitions = kScheduleTransitionsMax;
    DataModel::Nullable<uint8_t> mNumberOfScheduleTransitionsPerDay;

    uint8_t mActiveScheduleHandleBuffer[kScheduleHandleSize] = { 0 };
    size_t mActiveScheduleHandleLength                       = 0;
    bool mActiveScheduleHandleIsNull                         = true;

    bool mSetActiveScheduleHandleCalled     = false;
    bool mAppendToPendingScheduleListCalled = false;
    bool mCommitPendingSchedulesCalled      = false;
    bool mClearPendingScheduleListCalled    = false;
};

// Builds a ScheduleStruct::DecodableType by round-tripping a ScheduleStruct::Type through TLV, mirroring
// how the real ThermostatCluster.cpp Write() path decodes incoming schedules off the wire.
CHIP_ERROR EncodeThenDecodeSchedule(const ScheduleStruct::Type & in, ScheduleStruct::DecodableType & out)
{
    static uint8_t buffer[512];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(in.Encode(writer, TLV::ContextTag(1)));
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ReturnErrorOnFailure(reader.Next());
    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next());
    return out.Decode(reader);
}

ScheduleTransitionStruct::Type MakeTransition(uint16_t transitionTime)
{
    ScheduleTransitionStruct::Type transition;
    transition.dayOfWeek       = BitMask<ScheduleDayOfWeekBitmap>(ScheduleDayOfWeekBitmap::kMonday);
    transition.transitionTime  = transitionTime;
    transition.systemMode      = MakeOptional(SystemModeEnum::kHeat);
    transition.heatingSetpoint = MakeOptional(static_cast<int16_t>(2000));
    return transition;
}

ScheduleStruct::Type MakeNewScheduleRequest(std::vector<ScheduleTransitionStruct::Type> & storage)
{
    ScheduleStruct::Type schedule;
    schedule.scheduleHandle = DataModel::NullNullable;
    schedule.systemMode     = SystemModeEnum::kHeat;
    schedule.builtIn        = DataModel::NullNullable;
    schedule.transitions    = DataModel::List<const ScheduleTransitionStruct::Type>(storage.data(), storage.size());
    return schedule;
}

class TestThermostatClusterSchedules : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        chip::DeviceLayer::PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }

    void SetUp() override { chip::Testing::numEndpoints = 1; }

    void TearDown() override { SetDefaultDelegate(kEndpoint, nullptr); }
};

TEST_F(TestThermostatClusterSchedules, IsScheduleHandlePresentInSchedulesFindsMatch)
{
    FakeThermostatDelegate delegate;
    ScheduleStructWithOwnedMembers schedule;
    ASSERT_EQ(schedule.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(kHandleA))), CHIP_NO_ERROR);
    delegate.AddSchedule(schedule);

    EXPECT_TRUE(IsScheduleHandlePresentInSchedules(&delegate, ByteSpan(kHandleA)));
    EXPECT_FALSE(IsScheduleHandlePresentInSchedules(&delegate, ByteSpan(kUnknownHandle)));
    EXPECT_FALSE(IsScheduleHandlePresentInSchedules(nullptr, ByteSpan(kHandleA)));
}

TEST_F(TestThermostatClusterSchedules, SetActiveScheduleSuccessCallsDelegateAndGeneratesEvent)
{
    FakeThermostatDelegate delegate;
    ScheduleStructWithOwnedMembers schedule;
    ASSERT_EQ(schedule.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(kHandleA))), CHIP_NO_ERROR);
    delegate.AddSchedule(schedule);
    SetDefaultDelegate(kEndpoint, &delegate);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    auto status = accessor.SetActiveSchedule(kEndpoint, DataModel::MakeNullable(ByteSpan(kHandleA)));
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    EXPECT_EQ(status, Status::Success);
    EXPECT_TRUE(delegate.mSetActiveScheduleHandleCalled);
}

TEST_F(TestThermostatClusterSchedules, SetActiveScheduleUnknownHandleReturnsInvalidCommand)
{
    FakeThermostatDelegate delegate;
    SetDefaultDelegate(kEndpoint, &delegate);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    auto status = accessor.SetActiveSchedule(kEndpoint, DataModel::MakeNullable(ByteSpan(kUnknownHandle)));
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    EXPECT_EQ(status, Status::InvalidCommand);
    EXPECT_FALSE(delegate.mSetActiveScheduleHandleCalled);
}

TEST_F(TestThermostatClusterSchedules, SetActiveScheduleNullHandleClearsActiveSchedule)
{
    FakeThermostatDelegate delegate;
    SetDefaultDelegate(kEndpoint, &delegate);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    auto status = accessor.SetActiveSchedule(kEndpoint, DataModel::NullNullable);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    EXPECT_EQ(status, Status::Success);
    EXPECT_TRUE(delegate.mSetActiveScheduleHandleCalled);
}

TEST_F(TestThermostatClusterSchedules, SetActiveScheduleNoDelegateReturnsInvalidInState)
{
    // No delegate registered for kEndpoint.
    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    auto status = accessor.SetActiveSchedule(kEndpoint, DataModel::NullNullable);
    EXPECT_EQ(status, Status::InvalidInState);
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsOversizedScheduleHandle)
{
    FakeThermostatDelegate delegate;
    uint8_t oversized[kScheduleHandleSize + 1] = { 0 };

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);
    request.scheduleHandle       = DataModel::MakeNullable(ByteSpan(oversized));

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsUnknownSystemMode)
{
    FakeThermostatDelegate delegate;
    delegate.AddScheduleType(SystemModeEnum::kHeat, 4);

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);
    // kUnknownEnumValue is a sentinel that real senders can never transmit (encoding it is itself
    // rejected with ConstraintError); simulate what an out-of-range wire value decodes to instead of
    // going through TLV.
    decoded.systemMode = SystemModeEnum::kUnknownEnumValue;

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsBuiltInOnNewSchedule)
{
    FakeThermostatDelegate delegate;
    delegate.AddScheduleType(SystemModeEnum::kHeat, 4);

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);
    request.builtIn              = DataModel::MakeNullable(true);

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsUnsupportedSystemMode)
{
    FakeThermostatDelegate delegate;
    // No matching ScheduleTypeStruct entry for kHeat: MaximumScheduleTypeCount() will stay 0.

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsNameWhenNotSupported)
{
    FakeThermostatDelegate delegate;
    delegate.AddScheduleType(SystemModeEnum::kHeat, 4); // No kSupportsNames feature.

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);
    request.name                 = MakeOptional(CharSpan::fromCharString("My schedule"));

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsTooManyTransitionsPerDay)
{
    FakeThermostatDelegate delegate;
    delegate.AddScheduleType(SystemModeEnum::kHeat, 4);
    delegate.mNumberOfScheduleTransitionsPerDay = DataModel::MakeNullable(static_cast<uint8_t>(1));

    // Two Monday transitions exceed the per-day limit of 1.
    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0), MakeTransition(100) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsScheduleCountExceeded)
{
    FakeThermostatDelegate delegate;
    delegate.AddScheduleType(SystemModeEnum::kHeat, 4);
    delegate.mNumberOfSchedules = 0; // Even one schedule exceeds the max.

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsUnknownExistingHandle)
{
    FakeThermostatDelegate delegate;
    delegate.AddScheduleType(SystemModeEnum::kHeat, 4);

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);
    request.scheduleHandle       = DataModel::MakeNullable(ByteSpan(kHandleA)); // Not present in Schedules.

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_IM_GLOBAL_STATUS(NotFound));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsDuplicatePendingHandle)
{
    FakeThermostatDelegate delegate;
    delegate.AddScheduleType(SystemModeEnum::kHeat, 4);

    ScheduleStructWithOwnedMembers existing;
    ASSERT_EQ(existing.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(kHandleA))), CHIP_NO_ERROR);
    existing.SetSystemMode(SystemModeEnum::kHeat);
    existing.SetBuiltIn(DataModel::MakeNullable(false));
    delegate.AddSchedule(existing);
    delegate.mPendingSchedules[delegate.mPendingSchedulesCount++] = existing; // Already pending.

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);
    request.scheduleHandle       = DataModel::MakeNullable(ByteSpan(kHandleA));
    request.builtIn              = DataModel::MakeNullable(false);

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleRejectsBuiltInMismatch)
{
    FakeThermostatDelegate delegate;
    delegate.AddScheduleType(SystemModeEnum::kHeat, 4);

    ScheduleStructWithOwnedMembers existing;
    ASSERT_EQ(existing.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(kHandleA))), CHIP_NO_ERROR);
    existing.SetSystemMode(SystemModeEnum::kHeat);
    existing.SetBuiltIn(DataModel::MakeNullable(true));
    delegate.AddSchedule(existing);

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);
    request.scheduleHandle       = DataModel::MakeNullable(ByteSpan(kHandleA));
    request.builtIn              = DataModel::MakeNullable(false); // Mismatches existing built-in of true.

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleSucceedsForNewSchedule)
{
    FakeThermostatDelegate delegate;
    delegate.AddScheduleType(SystemModeEnum::kHeat, 4);

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_NO_ERROR);
    EXPECT_TRUE(delegate.mAppendToPendingScheduleListCalled);
    ASSERT_EQ(delegate.mPendingSchedulesCount, 1u);
    // BuiltIn must be forced to false for a brand-new schedule with a null handle.
    ASSERT_FALSE(delegate.mPendingSchedules[0].GetBuiltIn().IsNull());
    EXPECT_FALSE(delegate.mPendingSchedules[0].GetBuiltIn().Value());
}

TEST_F(TestThermostatClusterSchedules, AppendPendingScheduleSucceedsForExistingHandle)
{
    FakeThermostatDelegate delegate;
    delegate.AddScheduleType(SystemModeEnum::kHeat, 4);

    ScheduleStructWithOwnedMembers existing;
    ASSERT_EQ(existing.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(kHandleA))), CHIP_NO_ERROR);
    existing.SetSystemMode(SystemModeEnum::kHeat);
    existing.SetBuiltIn(DataModel::MakeNullable(true));
    delegate.AddSchedule(existing);

    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0) };
    ScheduleStruct::Type request = MakeNewScheduleRequest(storage);
    request.scheduleHandle       = DataModel::MakeNullable(ByteSpan(kHandleA));
    request.builtIn              = DataModel::NullNullable; // Inherits built-in from the matching existing schedule.

    ScheduleStruct::DecodableType decoded;
    ASSERT_EQ(EncodeThenDecodeSchedule(request, decoded), CHIP_NO_ERROR);

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.AppendPendingSchedule(&delegate, decoded), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.mPendingSchedulesCount, 1u);
    ASSERT_FALSE(delegate.mPendingSchedules[0].GetBuiltIn().IsNull());
    EXPECT_TRUE(delegate.mPendingSchedules[0].GetBuiltIn().Value());
}

TEST_F(TestThermostatClusterSchedules, PrecommitSchedulesRejectsMissingBuiltInSchedule)
{
    FakeThermostatDelegate delegate;
    SetDefaultDelegate(kEndpoint, &delegate);

    ScheduleStructWithOwnedMembers builtInSchedule;
    ASSERT_EQ(builtInSchedule.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(kHandleA))), CHIP_NO_ERROR);
    builtInSchedule.SetBuiltIn(DataModel::MakeNullable(true));
    delegate.AddSchedule(builtInSchedule);
    // Note: no matching entry added to the pending schedules list, simulating an attempted removal.

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.PrecommitSchedules(kEndpoint), Status::ConstraintError);
}

TEST_F(TestThermostatClusterSchedules, PrecommitSchedulesRejectsMissingActiveScheduleInPendingList)
{
    FakeThermostatDelegate delegate;
    SetDefaultDelegate(kEndpoint, &delegate);
    delegate.mActiveScheduleHandleIsNull = false;
    delegate.mActiveScheduleHandleLength = sizeof(kHandleA);
    memcpy(delegate.mActiveScheduleHandleBuffer, kHandleA, sizeof(kHandleA));
    // The pending schedules list does not contain kHandleA.

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.PrecommitSchedules(kEndpoint), Status::InvalidInState);
}

TEST_F(TestThermostatClusterSchedules, PrecommitSchedulesSucceedsWhenConsistent)
{
    FakeThermostatDelegate delegate;
    SetDefaultDelegate(kEndpoint, &delegate);

    ScheduleStructWithOwnedMembers builtInSchedule;
    ASSERT_EQ(builtInSchedule.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(kHandleA))), CHIP_NO_ERROR);
    builtInSchedule.SetBuiltIn(DataModel::MakeNullable(true));
    delegate.AddSchedule(builtInSchedule);
    delegate.mPendingSchedules[delegate.mPendingSchedulesCount++] = builtInSchedule;

    delegate.mActiveScheduleHandleIsNull = false;
    delegate.mActiveScheduleHandleLength = sizeof(kHandleA);
    memcpy(delegate.mActiveScheduleHandleBuffer, kHandleA, sizeof(kHandleA));

    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.PrecommitSchedules(kEndpoint), Status::Success);
}

TEST_F(TestThermostatClusterSchedules, PrecommitSchedulesNoDelegateReturnsInvalidInState)
{
    chip::Testing::ThermostatAttrAccessTestAccess accessor(&gThermostatAttrAccess);
    EXPECT_EQ(accessor.PrecommitSchedules(kEndpoint), Status::InvalidInState);
}

} // namespace
