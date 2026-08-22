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

#include <app/clusters/thermostat-server/PresetStructWithOwnedMembers.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/clusters/thermostat-server/ThermostatDelegate.h>
#include <app/clusters/thermostat-server/ThermostatSuggestionStructWithOwnedMembers.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/FabricTestFixture.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <lib/support/CodeUtils.h>
#include <pw_unit_test/framework.h>

#include <algorithm>

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
constexpr uint8_t kTestMaxPresets     = 4;
constexpr uint8_t kTestMaxSuggestions = 4;

// A Delegate double with real (array-backed) behavior for Presets and ThermostatSuggestions, since these tests
// exercise the cascade from a Presets atomic write commit to the ThermostatSuggestions list.
class TestDelegate : public Delegate
{
public:
    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(AttributeId) override
    {
        return System::Clock::Milliseconds16(3000);
    }

    // --- Presets: real, array-backed behavior. ---
    CHIP_ERROR GetPresetTypeAtIndex(size_t, Structs::PresetTypeStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    uint8_t GetNumberOfPresets() override { return mNumPresets; }
    CHIP_ERROR GetPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) override
    {
        if (mFailPresetEnumerationAtIndex.has_value() && index == *mFailPresetEnumerationAtIndex)
        {
            return CHIP_ERROR_INTERNAL;
        }
        mPresetEnumerationCallCount++;
        if (mFailPresetEnumerationOnCallNumber.has_value() && mPresetEnumerationCallCount == *mFailPresetEnumerationOnCallNumber)
        {
            return CHIP_ERROR_INTERNAL;
        }
        if (index < mNumPresets)
        {
            preset = mPresets[index];
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & handle) override
    {
        handle.SetNull();
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetActivePresetHandle(const DataModel::Nullable<ByteSpan> &) override { return CHIP_NO_ERROR; }
    void InitializePendingPresets() override
    {
        mNumPendingPresets = 0;
        for (uint8_t i = 0; i < mNumPresets; i++)
        {
            mPendingPresets[mNumPendingPresets++] = mPresets[i];
        }
    }
    CHIP_ERROR AppendToPendingPresetList(const PresetStructWithOwnedMembers & preset) override
    {
        VerifyOrReturnError(mNumPendingPresets < MATTER_ARRAY_SIZE(mPendingPresets), CHIP_ERROR_WRITE_FAILED);
        mPendingPresets[mNumPendingPresets++] = preset;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetPendingPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) override
    {
        if (index < mNumPendingPresets)
        {
            preset = mPendingPresets[index];
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR CommitPendingPresets() override
    {
        mNumPresets = 0;
        for (uint8_t i = 0; i < mNumPendingPresets; i++)
        {
            mPresets[mNumPresets++] = mPendingPresets[i];
        }
        return CHIP_NO_ERROR;
    }
    void ClearPendingPresetList() override { mNumPendingPresets = 0; }

    // --- ThermostatSuggestions: real, array-backed behavior. ---
    uint8_t GetMaxThermostatSuggestions() override { return kTestMaxSuggestions; }
    uint8_t GetNumberOfThermostatSuggestions() override { return mNumSuggestions; }
    CHIP_ERROR GetThermostatSuggestionAtIndex(size_t index, ThermostatSuggestionStructWithOwnedMembers & suggestion) override
    {
        if (index < mNumSuggestions)
        {
            suggestion = mSuggestions[index];
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    void GetCurrentThermostatSuggestion(DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & current) override
    {
        if (mCurrentSuggestionIsNull)
        {
            current.SetNull();
        }
        else
        {
            current.SetNonNull(mCurrentSuggestion);
        }
    }
    DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> GetThermostatSuggestionNotFollowingReason() override
    {
        return DataModel::NullNullable;
    }
    CHIP_ERROR AppendToThermostatSuggestionsList(const Structs::ThermostatSuggestionStruct::Type &) override
    {
        return CHIP_ERROR_WRITE_FAILED;
    }
    CHIP_ERROR RemoveFromThermostatSuggestionsList(size_t indexToRemove) override
    {
        VerifyOrReturnError(indexToRemove < mNumSuggestions, CHIP_ERROR_INVALID_ARGUMENT);

        // Per the API contract, removing the entry that is the CurrentThermostatSuggestion nulls it out.
        bool wasCurrent =
            !mCurrentSuggestionIsNull && mCurrentSuggestion.GetUniqueID() == mSuggestions[indexToRemove].GetUniqueID();

        for (size_t i = indexToRemove; i + 1 < mNumSuggestions; i++)
        {
            mSuggestions[i] = mSuggestions[i + 1];
        }
        mNumSuggestions--;

        if (wasCurrent)
        {
            mCurrentSuggestionIsNull = true;
        }
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetUniqueID(uint8_t & uniqueID) override
    {
        uniqueID = mNextUniqueID++;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ReEvaluateCurrentSuggestion() override { return CHIP_NO_ERROR; }

    // --- ScheduleTypes: unused by these tests, stubbed out. ---
    CHIP_ERROR GetScheduleTypeAtIndex(size_t, Structs::ScheduleTypeStruct::Type &) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    // --- Test-only helpers ---

    // Seeds a single committed preset with the given handle byte.
    void SeedPreset(uint8_t handleByte)
    {
        PresetStructWithOwnedMembers preset;
        preset.SetPresetScenario(PresetScenarioEnum::kOccupied);
        const uint8_t handle[] = { handleByte };
        ASSERT_EQ(preset.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle))), CHIP_NO_ERROR);
        ASSERT_EQ(preset.SetName(NullOptional), CHIP_NO_ERROR);
        mPresets[mNumPresets++] = preset;
    }

    // Seeds a single committed ThermostatSuggestions entry referencing the preset with the given handle byte.
    void SeedSuggestion(uint8_t uniqueID, uint8_t presetHandleByte, bool makeCurrent)
    {
        ThermostatSuggestionStructWithOwnedMembers suggestion;
        suggestion.SetUniqueID(uniqueID);
        const uint8_t handle[] = { presetHandleByte };
        ASSERT_EQ(suggestion.SetPresetHandle(ByteSpan(handle)), CHIP_NO_ERROR);
        suggestion.SetEffectiveTime(System::Clock::Seconds32(0));
        suggestion.SetExpirationTime(System::Clock::Seconds32(1000));
        mSuggestions[mNumSuggestions++] = suggestion;
        if (makeCurrent)
        {
            mCurrentSuggestion       = suggestion;
            mCurrentSuggestionIsNull = false;
        }
    }

    // Makes GetPresetAtIndex() fail with a non-list-exhausted error at the given index, simulating a transient
    // enumeration failure.
    void FailPresetEnumerationAtIndex(size_t index) { mFailPresetEnumerationAtIndex = index; }

    // Makes the Nth call to GetPresetAtIndex() (1-indexed, counted across the whole cascade) fail with a
    // non-list-exhausted error, regardless of which index is being enumerated. Used to simulate a delegate that
    // starts failing partway through a cascade that checks multiple suggestions' presets in turn.
    void FailPresetEnumerationOnCallNumber(int callNumber) { mFailPresetEnumerationOnCallNumber = callNumber; }

    bool IsCurrentSuggestionNull() const { return mCurrentSuggestionIsNull; }

private:
    PresetStructWithOwnedMembers mPresets[kTestMaxPresets];
    PresetStructWithOwnedMembers mPendingPresets[kTestMaxPresets];
    uint8_t mNumPresets        = 0;
    uint8_t mNumPendingPresets = 0;

    ThermostatSuggestionStructWithOwnedMembers mSuggestions[kTestMaxSuggestions];
    uint8_t mNumSuggestions = 0;
    uint8_t mNextUniqueID   = 0;

    ThermostatSuggestionStructWithOwnedMembers mCurrentSuggestion;
    bool mCurrentSuggestionIsNull = true;

    std::optional<size_t> mFailPresetEnumerationAtIndex;
    std::optional<int> mFailPresetEnumerationOnCallNumber;
    int mPresetEnumerationCallCount = 0;
};

struct TestThermostatClusterSuggestions : public ::testing::Test
{
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
                                BitFlags<Thermostat::Feature>(Feature::kHeating, Feature::kPresets,
                                                              Feature::kThermostatSuggestions),
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
};

// Per spec § 4.3.11.50, committing a Presets atomic write that removes a preset must cascade: any
// ThermostatSuggestions entry referencing the removed preset must be deleted, and CurrentThermostatSuggestion must
// be nulled out if it referenced the removed preset.
TEST_F(TestThermostatClusterSuggestions, CommitAtomicWriteOfPresetsCascadesRemovalToSuggestions)
{
    mDelegate.SeedPreset(0x01);
    mDelegate.SeedPreset(0x02);
    mDelegate.SeedSuggestion(/* uniqueID = */ 7, /* presetHandleByte = */ 0x02, /* makeCurrent = */ true);

    ASSERT_EQ(mCluster.OnAtomicWriteBegin(Presets::Id), Status::Success);
    // Simulate the client's pending list dropping the preset with handle 0x02 (re-committing only 0x01).
    mDelegate.ClearPendingPresetList();
    PresetStructWithOwnedMembers survivingPreset;
    survivingPreset.SetPresetScenario(PresetScenarioEnum::kOccupied);
    const uint8_t survivingHandle[] = { 0x01 };
    ASSERT_EQ(survivingPreset.SetPresetHandle(DataModel::MakeNullable(ByteSpan(survivingHandle))), CHIP_NO_ERROR);
    ASSERT_EQ(survivingPreset.SetName(NullOptional), CHIP_NO_ERROR);
    ASSERT_EQ(mDelegate.AppendToPendingPresetList(survivingPreset), CHIP_NO_ERROR);

    EXPECT_EQ(mCluster.OnAtomicWriteCommit(Presets::Id), Status::Success);

    EXPECT_EQ(mDelegate.GetNumberOfThermostatSuggestions(), 0);
    EXPECT_TRUE(mDelegate.IsCurrentSuggestionNull());
}

// Removing the CurrentThermostatSuggestion's preset with no other suggestion available to replace it must still
// notify CurrentThermostatSuggestion as changed. ReEvaluateCurrentSuggestion() alone can't detect this transition:
// by the time it takes its "before" snapshot, the cascade has already nulled the delegate's CurrentThermostatSuggestion.
TEST_F(TestThermostatClusterSuggestions, CommitAtomicWriteNotifiesCurrentThermostatSuggestionWhenClearedWithNoReplacement)
{
    mDelegate.SeedPreset(0x01);
    mDelegate.SeedPreset(0x02);
    mDelegate.SeedSuggestion(/* uniqueID = */ 7, /* presetHandleByte = */ 0x02, /* makeCurrent = */ true);

    ASSERT_EQ(mCluster.OnAtomicWriteBegin(Presets::Id), Status::Success);
    mDelegate.ClearPendingPresetList();
    PresetStructWithOwnedMembers survivingPreset;
    survivingPreset.SetPresetScenario(PresetScenarioEnum::kOccupied);
    const uint8_t survivingHandle[] = { 0x01 };
    ASSERT_EQ(survivingPreset.SetPresetHandle(DataModel::MakeNullable(ByteSpan(survivingHandle))), CHIP_NO_ERROR);
    ASSERT_EQ(survivingPreset.SetName(NullOptional), CHIP_NO_ERROR);
    ASSERT_EQ(mDelegate.AppendToPendingPresetList(survivingPreset), CHIP_NO_ERROR);

    EXPECT_EQ(mCluster.OnAtomicWriteCommit(Presets::Id), Status::Success);

    EXPECT_TRUE(mDelegate.IsCurrentSuggestionNull());

    // mTester (ClusterTester) owns its own TestServerClusterContext, distinct from mTestContext above, which is what
    // mCluster is actually attached to via Startup(). Check dirty attributes against mTestContext's listener.
    const auto & dirtyList = mTestContext.ChangeListener().DirtyList();
    ConcreteAttributePath thermostatSuggestionsPath(kTestEndpointId, Thermostat::Id, ThermostatSuggestions::Id);
    ConcreteAttributePath currentThermostatSuggestionPath(kTestEndpointId, Thermostat::Id, CurrentThermostatSuggestion::Id);
    EXPECT_NE(std::find(dirtyList.begin(), dirtyList.end(), thermostatSuggestionsPath), dirtyList.end());
    EXPECT_NE(std::find(dirtyList.begin(), dirtyList.end(), currentThermostatSuggestionPath), dirtyList.end());
}

// A suggestion whose preset survives the commit must not be touched by the cascade.
TEST_F(TestThermostatClusterSuggestions, CommitAtomicWriteOfPresetsPreservesSuggestionsForSurvivingPresets)
{
    mDelegate.SeedPreset(0x01);
    mDelegate.SeedSuggestion(/* uniqueID = */ 7, /* presetHandleByte = */ 0x01, /* makeCurrent = */ true);

    ASSERT_EQ(mCluster.OnAtomicWriteBegin(Presets::Id), Status::Success);
    // BeginWrite's InitializePendingPresets() already seeded the pending list from the existing preset, so
    // committing without modification preserves it.
    EXPECT_EQ(mCluster.OnAtomicWriteCommit(Presets::Id), Status::Success);

    EXPECT_EQ(mDelegate.GetNumberOfThermostatSuggestions(), 1);
    EXPECT_FALSE(mDelegate.IsCurrentSuggestionNull());
}

// If enumerating the committed Presets list fails partway through (e.g. a transient delegate error), the cascade
// must not treat the suggestion's preset as removed: deleting on an inconclusive answer would cause data loss.
TEST_F(TestThermostatClusterSuggestions, CommitAtomicWriteDoesNotDropSuggestionsOnPresetEnumerationError)
{
    mDelegate.SeedPreset(0x01);
    mDelegate.SeedSuggestion(/* uniqueID = */ 7, /* presetHandleByte = */ 0x01, /* makeCurrent = */ true);
    mDelegate.FailPresetEnumerationAtIndex(0);

    ASSERT_EQ(mCluster.OnAtomicWriteBegin(Presets::Id), Status::Success);
    // The Presets commit itself still succeeds; only the best-effort suggestions cascade is affected by the
    // enumeration error.
    EXPECT_EQ(mCluster.OnAtomicWriteCommit(Presets::Id), Status::Success);

    EXPECT_EQ(mDelegate.GetNumberOfThermostatSuggestions(), 1);
    EXPECT_FALSE(mDelegate.IsCurrentSuggestionNull());
}

// If the Presets enumeration fails only partway through a cascade that has to check multiple suggestions, none of
// the suggestions checked before the failure may be removed either: the cascade must be all-or-nothing, not leave a
// partially-cleaned ThermostatSuggestions list behind an error.
TEST_F(TestThermostatClusterSuggestions, CommitAtomicWriteDoesNotPartiallyRemoveSuggestionsWhenLaterEnumerationFails)
{
    mDelegate.SeedPreset(0x01);
    // Both suggestions reference presets that no longer exist (only 0x01 is committed).
    mDelegate.SeedSuggestion(/* uniqueID = */ 1, /* presetHandleByte = */ 0x02, /* makeCurrent = */ false);
    mDelegate.SeedSuggestion(/* uniqueID = */ 2, /* presetHandleByte = */ 0x03, /* makeCurrent = */ false);
    // The first suggestion's preset check takes 2 GetPresetAtIndex() calls to confirm absence (one non-matching
    // preset, then list-exhausted); the 3rd call, made while checking the second suggestion, fails.
    mDelegate.FailPresetEnumerationOnCallNumber(3);

    ASSERT_EQ(mCluster.OnAtomicWriteBegin(Presets::Id), Status::Success);
    EXPECT_EQ(mCluster.OnAtomicWriteCommit(Presets::Id), Status::Success);

    // Neither suggestion was removed: the first suggestion's preset absence was confirmed before the failure, but
    // must not be acted upon since the cascade as a whole couldn't complete.
    EXPECT_EQ(mDelegate.GetNumberOfThermostatSuggestions(), 2);
}

} // namespace
