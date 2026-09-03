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
#include <lib/support/CodeUtils.h>
#include <pw_unit_test/framework.h>

#include <cstring>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;

namespace {

constexpr uint8_t kHandleData[] = { 0x01, 0x02, 0x03, 0x04 };
constexpr CharSpan kNameData    = "Weekday schedule"_span;
constexpr uint8_t kPresetData[] = { 0xAA, 0xBB };

ScheduleTransitionStruct::Type MakeTransition(uint16_t transitionTime, const Optional<ByteSpan> & presetHandle = NullOptional)
{
    ScheduleTransitionStruct::Type transition;
    transition.dayOfWeek       = BitMask<ScheduleDayOfWeekBitmap>(ScheduleDayOfWeekBitmap::kMonday);
    transition.transitionTime  = transitionTime;
    transition.systemMode      = MakeOptional(SystemModeEnum::kHeat);
    transition.heatingSetpoint = MakeOptional(static_cast<int16_t>(2000));
    transition.presetHandle    = presetHandle;
    return transition;
}

ScheduleStruct::Type MakeFullSchedule(std::vector<ScheduleTransitionStruct::Type> & storage)
{
    ScheduleStruct::Type schedule;
    schedule.scheduleHandle = DataModel::MakeNullable(ByteSpan(kHandleData));
    schedule.systemMode     = SystemModeEnum::kHeat;
    schedule.name           = MakeOptional(kNameData);
    schedule.presetHandle   = MakeOptional(ByteSpan(kPresetData));
    schedule.builtIn        = DataModel::MakeNullable(true);
    schedule.transitions    = DataModel::List<const ScheduleTransitionStruct::Type>(storage.data(), storage.size());
    return schedule;
}

TEST(TestScheduleStructWithOwnedMembers, DefaultConstructedIsEmpty)
{
    ScheduleStructWithOwnedMembers schedule;
    EXPECT_TRUE(schedule.GetScheduleHandle().IsNull());
    EXPECT_FALSE(schedule.GetName().HasValue());
    EXPECT_FALSE(schedule.GetPresetHandle().HasValue());
    EXPECT_TRUE(schedule.GetBuiltIn().IsNull());
    EXPECT_EQ(schedule.GetTransitions().size(), 0u);
}

TEST(TestScheduleStructWithOwnedMembers, ConstructFromTypeCopiesAllFields)
{
    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(600) };
    ScheduleStruct::Type source = MakeFullSchedule(storage);

    ScheduleStructWithOwnedMembers schedule(source);

    ASSERT_FALSE(schedule.GetScheduleHandle().IsNull());
    EXPECT_TRUE(schedule.GetScheduleHandle().Value().data_equal(ByteSpan(kHandleData)));
    EXPECT_EQ(schedule.GetSystemMode(), SystemModeEnum::kHeat);
    ASSERT_TRUE(schedule.GetName().HasValue());
    EXPECT_TRUE(schedule.GetName().Value().data_equal(kNameData));
    ASSERT_TRUE(schedule.GetPresetHandle().HasValue());
    EXPECT_TRUE(schedule.GetPresetHandle().Value().data_equal(ByteSpan(kPresetData)));
    ASSERT_FALSE(schedule.GetBuiltIn().IsNull());
    EXPECT_TRUE(schedule.GetBuiltIn().Value());
    ASSERT_EQ(schedule.GetTransitions().size(), 1u);
    EXPECT_EQ(schedule.GetTransitions()[0].transitionTime, 600);
}

TEST(TestScheduleStructWithOwnedMembers, CopyAssignmentDeepCopiesBuffers)
{
    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(600, MakeOptional(ByteSpan(kPresetData))) };
    ScheduleStruct::Type source = MakeFullSchedule(storage);

    // Use a temporary that goes out of scope so the copy cannot be aliasing the original's storage.
    ScheduleStructWithOwnedMembers copy;
    {
        ScheduleStructWithOwnedMembers original(source);
        copy = original;
    }

    ASSERT_FALSE(copy.GetScheduleHandle().IsNull());
    EXPECT_TRUE(copy.GetScheduleHandle().Value().data_equal(ByteSpan(kHandleData)));
    ASSERT_TRUE(copy.GetName().HasValue());
    EXPECT_TRUE(copy.GetName().Value().data_equal(kNameData));
    ASSERT_EQ(copy.GetTransitions().size(), 1u);
    EXPECT_EQ(copy.GetTransitions()[0].transitionTime, 600);
    ASSERT_TRUE(copy.GetTransitions()[0].presetHandle.HasValue());
    EXPECT_TRUE(copy.GetTransitions()[0].presetHandle.Value().data_equal(ByteSpan(kPresetData)));
}

TEST(TestScheduleStructWithOwnedMembers, SelfAssignmentIsNoOp)
{
    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(600) };
    ScheduleStructWithOwnedMembers schedule(MakeFullSchedule(storage));

    const ScheduleStructWithOwnedMembers & alias = schedule;
    schedule                                     = alias;

    ASSERT_FALSE(schedule.GetScheduleHandle().IsNull());
    EXPECT_TRUE(schedule.GetScheduleHandle().Value().data_equal(ByteSpan(kHandleData)));
    ASSERT_EQ(schedule.GetTransitions().size(), 1u);
}

TEST(TestScheduleStructWithOwnedMembers, SetScheduleHandleNullClearsHandle)
{
    ScheduleStructWithOwnedMembers schedule;
    EXPECT_EQ(schedule.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(kHandleData))), CHIP_NO_ERROR);
    EXPECT_FALSE(schedule.GetScheduleHandle().IsNull());

    EXPECT_EQ(schedule.SetScheduleHandle(DataModel::Nullable<ByteSpan>()), CHIP_NO_ERROR);
    EXPECT_TRUE(schedule.GetScheduleHandle().IsNull());
}

TEST(TestScheduleStructWithOwnedMembers, SetScheduleHandleTooLargeFails)
{
    uint8_t oversized[kScheduleHandleSize + 1] = { 0 };
    ScheduleStructWithOwnedMembers schedule;
    EXPECT_EQ(schedule.SetScheduleHandle(DataModel::MakeNullable(ByteSpan(oversized))), CHIP_ERROR_NO_MEMORY);
}

TEST(TestScheduleStructWithOwnedMembers, SetNameTooLargeFails)
{
    std::vector<char> oversized(kScheduleNameSize + 1, 'a');
    ScheduleStructWithOwnedMembers schedule;
    EXPECT_EQ(schedule.SetName(MakeOptional(CharSpan(oversized.data(), oversized.size()))), CHIP_ERROR_NO_MEMORY);
}

TEST(TestScheduleStructWithOwnedMembers, SetNameClearsWhenNoValue)
{
    ScheduleStructWithOwnedMembers schedule;
    EXPECT_EQ(schedule.SetName(MakeOptional(kNameData)), CHIP_NO_ERROR);
    EXPECT_TRUE(schedule.GetName().HasValue());

    EXPECT_EQ(schedule.SetName(Optional<CharSpan>()), CHIP_NO_ERROR);
    EXPECT_FALSE(schedule.GetName().HasValue());
}

TEST(TestScheduleStructWithOwnedMembers, SetPresetHandleTooLargeFails)
{
    uint8_t oversized[kScheduleHandleSize + 1] = { 0 };
    ScheduleStructWithOwnedMembers schedule;
    EXPECT_EQ(schedule.SetPresetHandle(MakeOptional(ByteSpan(oversized))), CHIP_ERROR_NO_MEMORY);
}

TEST(TestScheduleStructWithOwnedMembers, SetTransitionsRejectsTooManyEntries)
{
    std::vector<ScheduleTransitionStruct::Type> storage(kScheduleTransitionsMax + 1, MakeTransition(0));
    ScheduleStructWithOwnedMembers schedule;
    EXPECT_EQ(schedule.SetTransitions(DataModel::List<const ScheduleTransitionStruct::Type>(storage.data(), storage.size())),
              CHIP_ERROR_NO_MEMORY);
}

TEST(TestScheduleStructWithOwnedMembers, SetTransitionsRejectsOversizedPresetHandle)
{
    uint8_t oversized[kScheduleHandleSize + 1] = { 0 };
    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(0, MakeOptional(ByteSpan(oversized))) };
    ScheduleStructWithOwnedMembers schedule;
    EXPECT_EQ(schedule.SetTransitions(DataModel::List<const ScheduleTransitionStruct::Type>(storage.data(), storage.size())),
              CHIP_ERROR_NO_MEMORY);
}

TEST(TestScheduleStructWithOwnedMembers, SetTransitionsStoresPresetHandleForEachTransition)
{
    std::vector<ScheduleTransitionStruct::Type> storage{ MakeTransition(100, MakeOptional(ByteSpan(kPresetData))),
                                                         MakeTransition(200) };
    ScheduleStructWithOwnedMembers schedule;
    ASSERT_EQ(schedule.SetTransitions(DataModel::List<const ScheduleTransitionStruct::Type>(storage.data(), storage.size())),
              CHIP_NO_ERROR);

    ASSERT_EQ(schedule.GetTransitions().size(), 2u);
    ASSERT_TRUE(schedule.GetTransitions()[0].presetHandle.HasValue());
    EXPECT_TRUE(schedule.GetTransitions()[0].presetHandle.Value().data_equal(ByteSpan(kPresetData)));
    EXPECT_FALSE(schedule.GetTransitions()[1].presetHandle.HasValue());
}

TEST(TestScheduleStructWithOwnedMembers, SetBuiltInNullable)
{
    ScheduleStructWithOwnedMembers schedule;
    schedule.SetBuiltIn(DataModel::MakeNullable(true));
    ASSERT_FALSE(schedule.GetBuiltIn().IsNull());
    EXPECT_TRUE(schedule.GetBuiltIn().Value());

    schedule.SetBuiltIn(DataModel::Nullable<bool>());
    EXPECT_TRUE(schedule.GetBuiltIn().IsNull());
}

} // namespace
