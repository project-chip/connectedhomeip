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
#include <app/tests/test-ember-api.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::Protocols::InteractionModel;

namespace {

class TestThermostatDelegate : public Delegate
{
public:
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

    CHIP_ERROR GetActivePresetHandle(chip::app::DataModel::Nullable<MutableByteSpan> & activePresetHandle) override
    {
        activePresetHandle.SetNull();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetActivePresetHandle(const chip::app::DataModel::Nullable<ByteSpan> & newActivePresetHandle) override
    {
        mSetActivePresetHandleCalled = true;
        return CHIP_NO_ERROR;
    }

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
        chip::app::DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & currentThermostatSuggestion) override
    {
        currentThermostatSuggestion.SetNull();
    }

    chip::app::DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap>
    GetThermostatSuggestionNotFollowingReason() override
    {
        return chip::app::DataModel::NullNullable;
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

    CHIP_ERROR GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & scheduleType) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    bool mSetActivePresetHandleCalled = false;
};

} // namespace

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

TEST(TestThermostatServerPresets, SetActivePresetSuccessCallsDelegate)
{
    constexpr EndpointId kEndpoint = 0;

    chip::Testing::numEndpoints = 1;

    TestThermostatDelegate delegate;
    SetDefaultDelegate(kEndpoint, &delegate);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    chip::Testing::ThermostatAttrAccessTestAccess accessor(&chip::app::Clusters::Thermostat::gThermostatAttrAccess);
    auto status = accessor.SetActivePreset(kEndpoint, chip::app::DataModel::NullNullable);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    EXPECT_EQ(status, Status::Success);
    EXPECT_TRUE(delegate.mSetActivePresetHandleCalled);
}

} // namespace
