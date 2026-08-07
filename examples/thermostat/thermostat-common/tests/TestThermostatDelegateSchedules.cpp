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

#include <thermostat-delegate-impl.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>

#include <cstring>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;

namespace {

constexpr uint8_t kCoolHandle[] = { static_cast<uint8_t>(SystemModeEnum::kCool) };
constexpr uint8_t kHeatHandle[] = { static_cast<uint8_t>(SystemModeEnum::kHeat) };

ScheduleStructWithOwnedMembers MakeSchedule(SystemModeEnum systemMode,
                                            DataModel::Nullable<ByteSpan> handle = DataModel::NullNullable)
{
    ScheduleStructWithOwnedMembers schedule;
    schedule.SetSystemMode(systemMode);
    VerifyOrDie(schedule.SetScheduleHandle(handle) == CHIP_NO_ERROR);
    return schedule;
}

class TestThermostatDelegateSchedules : public ::testing::Test
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

    // The delegate is a process-wide singleton (private constructor), so each test resets the
    // schedules and pending-schedules lists to empty via the public Delegate interface rather than
    // constructing a fresh instance.
    void SetUp() override
    {
        auto & delegate = ThermostatDelegate::GetInstance();
        delegate.ClearPendingScheduleList();
        ASSERT_EQ(delegate.CommitPendingSchedules(), CHIP_NO_ERROR);
        ASSERT_EQ(delegate.GetScheduleAtIndex(0, mScratchSchedule), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);

        chip::DeviceLayer::PlatformMgr().LockChipStack();
        ASSERT_EQ(delegate.SetActiveScheduleHandle(DataModel::NullNullable), CHIP_NO_ERROR);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }

    ScheduleStructWithOwnedMembers mScratchSchedule;
};

TEST_F(TestThermostatDelegateSchedules, ReportsConfiguredScheduleLimits)
{
    auto & delegate = ThermostatDelegate::GetInstance();

    EXPECT_EQ(delegate.GetNumberOfSchedules(), kMaxNumberOfSchedulesSupported);
    EXPECT_EQ(delegate.GetNumberOfScheduleTransitions(), kMaxNumberOfScheduleTransitionsSupported);

    auto perDay = delegate.GetNumberOfScheduleTransitionsPerDay();
    ASSERT_FALSE(perDay.IsNull());
    EXPECT_EQ(perDay.Value(), kMaxNumberOfScheduleTransitionsPerDaySupported);
}

TEST_F(TestThermostatDelegateSchedules, AppendAssignsHandleFromSystemModeWhenHandleIsNull)
{
    auto & delegate = ThermostatDelegate::GetInstance();

    ASSERT_EQ(delegate.AppendToPendingScheduleList(MakeSchedule(SystemModeEnum::kHeat)), CHIP_NO_ERROR);

    ScheduleStructWithOwnedMembers pending;
    ASSERT_EQ(delegate.GetPendingScheduleAtIndex(0, pending), CHIP_NO_ERROR);
    ASSERT_FALSE(pending.GetScheduleHandle().IsNull());
    EXPECT_TRUE(pending.GetScheduleHandle().Value().data_equal(ByteSpan(kHeatHandle)));
}

TEST_F(TestThermostatDelegateSchedules, AppendPreservesExplicitHandle)
{
    auto & delegate                   = ThermostatDelegate::GetInstance();
    constexpr uint8_t kCustomHandle[] = { 0xAB, 0xCD };

    ASSERT_EQ(
        delegate.AppendToPendingScheduleList(MakeSchedule(SystemModeEnum::kHeat, DataModel::MakeNullable(ByteSpan(kCustomHandle)))),
        CHIP_NO_ERROR);

    ScheduleStructWithOwnedMembers pending;
    ASSERT_EQ(delegate.GetPendingScheduleAtIndex(0, pending), CHIP_NO_ERROR);
    ASSERT_FALSE(pending.GetScheduleHandle().IsNull());
    EXPECT_TRUE(pending.GetScheduleHandle().Value().data_equal(ByteSpan(kCustomHandle)));
}

TEST_F(TestThermostatDelegateSchedules, AppendFailsWhenPendingListIsFull)
{
    auto & delegate            = ThermostatDelegate::GetInstance();
    constexpr size_t kCapacity = static_cast<size_t>(kMaxNumberOfScheduleTypes) * kMaxNumberOfSchedulesOfEachType;

    for (size_t i = 0; i < kCapacity; i++)
    {
        ASSERT_EQ(delegate.AppendToPendingScheduleList(MakeSchedule(SystemModeEnum::kHeat)), CHIP_NO_ERROR);
    }

    EXPECT_EQ(delegate.AppendToPendingScheduleList(MakeSchedule(SystemModeEnum::kHeat)), CHIP_ERROR_WRITE_FAILED);
}

TEST_F(TestThermostatDelegateSchedules, CommitPendingSchedulesReplacesSchedulesList)
{
    auto & delegate = ThermostatDelegate::GetInstance();

    ASSERT_EQ(delegate.AppendToPendingScheduleList(MakeSchedule(SystemModeEnum::kHeat)), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.AppendToPendingScheduleList(MakeSchedule(SystemModeEnum::kCool)), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.CommitPendingSchedules(), CHIP_NO_ERROR);

    ScheduleStructWithOwnedMembers schedule;
    ASSERT_EQ(delegate.GetScheduleAtIndex(0, schedule), CHIP_NO_ERROR);
    EXPECT_TRUE(schedule.GetScheduleHandle().Value().data_equal(ByteSpan(kHeatHandle)));

    ASSERT_EQ(delegate.GetScheduleAtIndex(1, schedule), CHIP_NO_ERROR);
    EXPECT_TRUE(schedule.GetScheduleHandle().Value().data_equal(ByteSpan(kCoolHandle)));

    EXPECT_EQ(delegate.GetScheduleAtIndex(2, schedule), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
}

TEST_F(TestThermostatDelegateSchedules, InitializePendingSchedulesCopiesExistingSchedules)
{
    auto & delegate = ThermostatDelegate::GetInstance();

    ASSERT_EQ(delegate.AppendToPendingScheduleList(MakeSchedule(SystemModeEnum::kHeat)), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.CommitPendingSchedules(), CHIP_NO_ERROR);

    // Appending without first (re-)initializing the pending list would carry over stale entries from a
    // previous atomic write; InitializePendingSchedules() is what resets it to mirror the committed list.
    delegate.InitializePendingSchedules();

    ScheduleStructWithOwnedMembers pending;
    ASSERT_EQ(delegate.GetPendingScheduleAtIndex(0, pending), CHIP_NO_ERROR);
    EXPECT_TRUE(pending.GetScheduleHandle().Value().data_equal(ByteSpan(kHeatHandle)));
    EXPECT_EQ(delegate.GetPendingScheduleAtIndex(1, pending), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
}

TEST_F(TestThermostatDelegateSchedules, ClearPendingScheduleListEmptiesPendingList)
{
    auto & delegate = ThermostatDelegate::GetInstance();

    ASSERT_EQ(delegate.AppendToPendingScheduleList(MakeSchedule(SystemModeEnum::kHeat)), CHIP_NO_ERROR);
    delegate.ClearPendingScheduleList();

    ScheduleStructWithOwnedMembers pending;
    EXPECT_EQ(delegate.GetPendingScheduleAtIndex(0, pending), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
}

TEST_F(TestThermostatDelegateSchedules, ActiveScheduleHandleRoundTrips)
{
    auto & delegate = ThermostatDelegate::GetInstance();

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    ASSERT_EQ(delegate.SetActiveScheduleHandle(DataModel::MakeNullable(ByteSpan(kHeatHandle))), CHIP_NO_ERROR);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    uint8_t buffer[kScheduleHandleSize];
    auto activeHandle = DataModel::MakeNullable(MutableByteSpan(buffer));
    ASSERT_EQ(delegate.GetActiveScheduleHandle(activeHandle), CHIP_NO_ERROR);
    ASSERT_FALSE(activeHandle.IsNull());
    EXPECT_TRUE(activeHandle.Value().data_equal(ByteSpan(kHeatHandle)));

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    ASSERT_EQ(delegate.SetActiveScheduleHandle(DataModel::NullNullable), CHIP_NO_ERROR);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    auto clearedHandle = DataModel::MakeNullable(MutableByteSpan(buffer));
    ASSERT_EQ(delegate.GetActiveScheduleHandle(clearedHandle), CHIP_NO_ERROR);
    EXPECT_TRUE(clearedHandle.IsNull());
}

TEST_F(TestThermostatDelegateSchedules, SetActiveScheduleHandleRejectsOversizedHandle)
{
    auto & delegate                            = ThermostatDelegate::GetInstance();
    uint8_t oversized[kScheduleHandleSize + 1] = { 0 };

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    auto status = delegate.SetActiveScheduleHandle(DataModel::MakeNullable(ByteSpan(oversized)));
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    EXPECT_EQ(status, CHIP_ERROR_NO_MEMORY);

    uint8_t buffer[kScheduleHandleSize];
    auto activeHandle = DataModel::MakeNullable(MutableByteSpan(buffer));
    ASSERT_EQ(delegate.GetActiveScheduleHandle(activeHandle), CHIP_NO_ERROR);
    EXPECT_TRUE(activeHandle.IsNull());
}

} // namespace
