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

#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <credentials/FabricTable.h>
#include <pw_unit_test/framework.h>
#include <system/RAIIMockClock.h>

#include <limits>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;

namespace {

constexpr EndpointId kTestEndpointId = 1;

// A minimal Delegate double: every method returns a safe "empty"/no-op value since these tests only exercise
// setpoint-change tracking, not Presets, Suggestions or Schedules. GetSetpointChangeSource() is overridable per-test
// via SetSetpointChangeSource() to exercise the delegate hook.
class TestDelegate : public Delegate
{
public:
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
        return DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap>();
    }
    CHIP_ERROR AppendToThermostatSuggestionsList(const Structs::ThermostatSuggestionStruct::Type &) override
    {
        return CHIP_ERROR_WRITE_FAILED;
    }
    CHIP_ERROR RemoveFromThermostatSuggestionsList(size_t) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR GetUniqueID(uint8_t & uniqueID) override
    {
        uniqueID = 0;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ReEvaluateCurrentSuggestion() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetScheduleTypeAtIndex(size_t, Structs::ScheduleTypeStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    SetpointChangeSourceEnum GetSetpointChangeSource() override { return mSetpointChangeSource; }
    void SetSetpointChangeSource(SetpointChangeSourceEnum source) { mSetpointChangeSource = source; }

private:
    SetpointChangeSourceEnum mSetpointChangeSource = SetpointChangeSourceEnum::kManual;
};

ThermostatCluster::OptionalAttributes AllSetpointChangeAttributesEnabled()
{
    ThermostatCluster::OptionalAttributes optionalAttributes;
    optionalAttributes.SetpointChangeSource          = true;
    optionalAttributes.SetpointChangeAmount          = true;
    optionalAttributes.SetpointChangeSourceTimestamp = true;
    return optionalAttributes;
}

struct TestThermostatClusterSetpointChangeTracking : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    FabricTable mFabricTable;
    TestDelegate mDelegate;
};

TEST_F(TestThermostatClusterSetpointChangeTracking, DefaultsBeforeAnyChange)
{
    TestServerClusterContext context;
    ThermostatCluster cluster{ kTestEndpointId, BitFlags<Feature>(Feature::kHeating), AllSetpointChangeAttributesEnabled(),
                               ThermostatCluster::DefaultValues(), mFabricTable };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    SetpointChangeSourceEnum source;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSource::Id, source), CHIP_NO_ERROR);
    EXPECT_EQ(source, SetpointChangeSourceEnum::kManual);

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), CHIP_NO_ERROR);
    EXPECT_TRUE(amount.IsNull());

    uint32_t timestamp = 1;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSourceTimestamp::Id, timestamp), CHIP_NO_ERROR);
    EXPECT_EQ(timestamp, 0u);
}

TEST_F(TestThermostatClusterSetpointChangeTracking, WritingOccupiedHeatingSetpointUpdatesTracking)
{
    // SetpointChangeSourceTimestamp requires a synchronized real time clock; mock it so the test
    // does not depend on the platform's wall-clock time being available (e.g. it is not set on
    // some embedded simulators).
    System::Clock::Internal::RAIIMockClock mockClock;
    ASSERT_EQ(mockClock.SetClock_RealTime(System::Clock::Microseconds64(1700000000000000ULL)), CHIP_NO_ERROR);

    TestServerClusterContext context;
    ThermostatCluster cluster{ kTestEndpointId, BitFlags<Feature>(Feature::kHeating), AllSetpointChangeAttributesEnabled(),
                               ThermostatCluster::DefaultValues(), mFabricTable };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&mDelegate);
    ClusterTester tester(cluster);

    // Default OccupiedHeatingSetpoint is 2000 (20C); move it up by 1C.
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(2100)), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), CHIP_NO_ERROR);
    ASSERT_FALSE(amount.IsNull());
    EXPECT_EQ(amount.Value(), 100);

    SetpointChangeSourceEnum source;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSource::Id, source), CHIP_NO_ERROR);
    EXPECT_EQ(source, SetpointChangeSourceEnum::kManual);

    uint32_t timestamp = 0;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSourceTimestamp::Id, timestamp), CHIP_NO_ERROR);
    EXPECT_GT(timestamp, 0u);
}

TEST_F(TestThermostatClusterSetpointChangeTracking, AmountReflectsSignOfDecrease)
{
    TestServerClusterContext context;
    ThermostatCluster cluster{ kTestEndpointId, BitFlags<Feature>(Feature::kHeating), AllSetpointChangeAttributesEnabled(),
                               ThermostatCluster::DefaultValues(), mFabricTable };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // Default OccupiedHeatingSetpoint is 2000 (20C); move it down by 1.5C.
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(1850)), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), CHIP_NO_ERROR);
    ASSERT_FALSE(amount.IsNull());
    EXPECT_EQ(amount.Value(), -150);
}

TEST_F(TestThermostatClusterSetpointChangeTracking, NoOpWriteDoesNotUpdateTracking)
{
    TestServerClusterContext context;
    ThermostatCluster cluster{ kTestEndpointId, BitFlags<Feature>(Feature::kHeating), AllSetpointChangeAttributesEnabled(),
                               ThermostatCluster::DefaultValues(), mFabricTable };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // First, a real change to get a known non-default state.
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(2100)), CHIP_NO_ERROR);
    DataModel::Nullable<int16_t> amountAfterFirstWrite;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amountAfterFirstWrite), CHIP_NO_ERROR);

    // Writing the same value again should be a no-op for the setpoint, and must not touch tracking attributes.
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(2100)), CHIP_NO_ERROR);
    DataModel::Nullable<int16_t> amountAfterNoOpWrite;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amountAfterNoOpWrite), CHIP_NO_ERROR);
    EXPECT_EQ(amountAfterFirstWrite, amountAfterNoOpWrite);
}

TEST_F(TestThermostatClusterSetpointChangeTracking, LimitWritesDoNotUpdateTracking)
{
    TestServerClusterContext context;
    ThermostatCluster::OptionalAttributes optionalAttributes = AllSetpointChangeAttributesEnabled();
    optionalAttributes.MinHeatSetpointLimit                  = true;
    optionalAttributes.MaxCoolSetpointLimit                  = true;
    ThermostatCluster cluster{ kTestEndpointId, BitFlags<Feature>(Feature::kHeating, Feature::kCooling), optionalAttributes,
                               ThermostatCluster::DefaultValues(), mFabricTable };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    EXPECT_EQ(tester.WriteAttribute(MinHeatSetpointLimit::Id, static_cast<int16_t>(1000)), CHIP_NO_ERROR);
    EXPECT_EQ(tester.WriteAttribute(MaxCoolSetpointLimit::Id, static_cast<int16_t>(2900)), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), CHIP_NO_ERROR);
    EXPECT_TRUE(amount.IsNull());

    uint32_t timestamp = 1;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSourceTimestamp::Id, timestamp), CHIP_NO_ERROR);
    EXPECT_EQ(timestamp, 0u);
}

TEST_F(TestThermostatClusterSetpointChangeTracking, DelegateOverridesSource)
{
    TestServerClusterContext context;
    ThermostatCluster cluster{ kTestEndpointId, BitFlags<Feature>(Feature::kHeating), AllSetpointChangeAttributesEnabled(),
                               ThermostatCluster::DefaultValues(), mFabricTable };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&mDelegate);
    ClusterTester tester(cluster);

    mDelegate.SetSetpointChangeSource(SetpointChangeSourceEnum::kExternal);
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(2100)), CHIP_NO_ERROR);

    SetpointChangeSourceEnum source;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSource::Id, source), CHIP_NO_ERROR);
    EXPECT_EQ(source, SetpointChangeSourceEnum::kExternal);
}

TEST_F(TestThermostatClusterSetpointChangeTracking, DisabledOptionalAttributesAreAbsent)
{
    TestServerClusterContext context;
    // Default OptionalAttributes leaves SetpointChange* disabled.
    ThermostatCluster cluster{ kTestEndpointId, BitFlags<Feature>(Feature::kHeating), ThermostatCluster::OptionalAttributes(),
                               ThermostatCluster::DefaultValues(), mFabricTable };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // Not exercising the change at all: just confirm the endpoint doesn't advertise the attributes and a write
    // that changes the setpoint still succeeds without touching them.
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(2100)), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
}

TEST_F(TestThermostatClusterSetpointChangeTracking, SetpointRaiseLowerUpdatesTracking)
{
    TestServerClusterContext context;
    ThermostatCluster cluster{ kTestEndpointId, BitFlags<Feature>(Feature::kHeating), AllSetpointChangeAttributesEnabled(),
                               ThermostatCluster::DefaultValues(), mFabricTable };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    Commands::SetpointRaiseLower::Type request;
    request.mode   = SetpointRaiseLowerModeEnum::kHeat;
    request.amount = 5; // 0.5C, in the command's deci-degree units

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(result.status->IsSuccess());

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), CHIP_NO_ERROR);
    ASSERT_FALSE(amount.IsNull());
    EXPECT_EQ(amount.Value(), 50);
}

TEST_F(TestThermostatClusterSetpointChangeTracking, AmountClampsOnOverflow)
{
    TestServerClusterContext context;
    ThermostatCluster::DefaultValues defaultValues;
    defaultValues.absMinHeatSetpointLimit = std::numeric_limits<int16_t>::min();
    defaultValues.absMaxHeatSetpointLimit = std::numeric_limits<int16_t>::max();
    defaultValues.occupiedHeatingSetpoint = std::numeric_limits<int16_t>::min();

    ThermostatCluster cluster{ kTestEndpointId, BitFlags<Feature>(Feature::kHeating), AllSetpointChangeAttributesEnabled(),
                               defaultValues, mFabricTable };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // The true delta (INT16_MAX - INT16_MIN == 65535) overflows int16_t and must be clamped, not wrapped.
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, std::numeric_limits<int16_t>::max()), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), CHIP_NO_ERROR);
    ASSERT_FALSE(amount.IsNull());
    EXPECT_EQ(amount.Value(), std::numeric_limits<int16_t>::max());
}

} // namespace
