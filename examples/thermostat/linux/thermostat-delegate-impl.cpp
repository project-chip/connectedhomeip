/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <thermostat-delegate-impl.h>
#include <thermostat-manager.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/support/Span.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;

ThermostatDelegate ThermostatDelegate::sInstance;

ThermostatDelegate::ThermostatDelegate()
{
    mNumberOfPresets                   = kMaxNumberOfPresetTypes * kMaxNumberOfPresetsOfEachType;
    mNextFreeIndexInPresetsList        = 0;
    mNextFreeIndexInPendingPresetsList = 0;

    InitializePresetTypes();
    InitializePresets();

    memset(mActivePresetHandleData, 0, sizeof(mActivePresetHandleData));
    mActivePresetHandleDataSize = 0;
}

void ThermostatDelegate::InitializePresetTypes()
{
    PresetScenarioEnum presetScenarioEnumArray[kMaxNumberOfPresetTypes] = {
        PresetScenarioEnum::kOccupied, PresetScenarioEnum::kUnoccupied, PresetScenarioEnum::kSleep,
        PresetScenarioEnum::kWake,     PresetScenarioEnum::kVacation,   PresetScenarioEnum::kGoingToSleep
    };
    static_assert(ArraySize(presetScenarioEnumArray) <= ArraySize(mPresetTypes));

    uint8_t index = 0;
    for (PresetScenarioEnum presetScenario : presetScenarioEnumArray)
    {
        mPresetTypes[index].presetScenario  = presetScenario;
        mPresetTypes[index].numberOfPresets = kMaxNumberOfPresetsOfEachType;
        mPresetTypes[index].presetTypeFeatures =
            (presetScenario == PresetScenarioEnum::kOccupied || presetScenario == PresetScenarioEnum::kUnoccupied)
            ? PresetTypeFeaturesBitmap::kAutomatic
            : PresetTypeFeaturesBitmap::kSupportsNames;
        index++;
    }
}

void ThermostatDelegate::InitializePresets()
{
    // Initialize the presets with 2 built in presets - occupied and unoccupied.
    PresetScenarioEnum presetScenarioEnumArray[2] = { PresetScenarioEnum::kOccupied, PresetScenarioEnum::kUnoccupied };
    static_assert(ArraySize(presetScenarioEnumArray) <= ArraySize(mPresets));

    uint8_t index = 0;
    for (PresetScenarioEnum presetScenario : presetScenarioEnumArray)
    {
        mPresets[index].SetPresetScenario(presetScenario);

        // Set the preset handle to the preset scenario value as a unique id.
        const uint8_t handle[] = { static_cast<uint8_t>(presetScenario) };
        mPresets[index].SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle)));
        mPresets[index].SetName(NullOptional);
        int16_t coolingSetpointValue = static_cast<int16_t>(2500 + (index * 100));
        mPresets[index].SetCoolingSetpoint(MakeOptional(coolingSetpointValue));

        int16_t heatingSetpointValue = static_cast<int16_t>(2100 - (index * 100));
        mPresets[index].SetHeatingSetpoint(MakeOptional(heatingSetpointValue));
        mPresets[index].SetBuiltIn(DataModel::MakeNullable(true));
        index++;
    }

    // Set the value of the next free index in the presets list.
    mNextFreeIndexInPresetsList = index;
}

CHIP_ERROR ThermostatDelegate::GetPresetTypeAtIndex(size_t index, PresetTypeStruct::Type & presetType)
{
    if (index < ArraySize(mPresetTypes))
    {
        presetType = mPresetTypes[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

uint8_t ThermostatDelegate::GetNumberOfPresets()
{
    return mNumberOfPresets;
}

CHIP_ERROR ThermostatDelegate::GetPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset)
{
    if (index < mNextFreeIndexInPresetsList)
    {
        preset = mPresets[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatDelegate::GetActivePresetHandle(MutableByteSpan & activePresetHandle)
{
    return CopySpanToMutableSpan(ByteSpan(mActivePresetHandleData, mActivePresetHandleDataSize), activePresetHandle);
}

CHIP_ERROR ThermostatDelegate::SetActivePresetHandle(const DataModel::Nullable<ByteSpan> & newActivePresetHandle)
{
    if (!newActivePresetHandle.IsNull())
    {
        size_t newActivePresetHandleSize = newActivePresetHandle.Value().size();
        if (newActivePresetHandleSize > sizeof(mActivePresetHandleData))
        {
            ChipLogError(NotSpecified,
                         "Failed to set ActivePresetHandle. newActivePresetHandle size %u is larger than preset handle size %u",
                         static_cast<uint8_t>(newActivePresetHandleSize), static_cast<uint8_t>(kPresetHandleSize));
            return CHIP_ERROR_NO_MEMORY;
        }
        memcpy(mActivePresetHandleData, newActivePresetHandle.Value().data(), newActivePresetHandleSize);
        mActivePresetHandleDataSize = newActivePresetHandleSize;
        ChipLogDetail(NotSpecified, "Set ActivePresetHandle to ");
        ChipLogByteSpan(NotSpecified, newActivePresetHandle.Value());
    }
    else
    {
        memset(mActivePresetHandleData, 0, sizeof(mActivePresetHandleData));
        mActivePresetHandleDataSize = 0;
        ChipLogDetail(NotSpecified, "Clear ActivePresetHandle");
    }
    return CHIP_NO_ERROR;
}

std::optional<System::Clock::Milliseconds16>
ThermostatDelegate::GetAtomicWriteTimeout(DataModel::DecodableList<AttributeId> attributeRequests,
                                          System::Clock::Milliseconds16 timeoutRequest)
{
    auto attributeIdsIter = attributeRequests.begin();
    bool requestedPresets = false, requestedSchedules = false;
    while (attributeIdsIter.Next())
    {
        auto & attributeId = attributeIdsIter.GetValue();

        switch (attributeId)
        {
        case Attributes::Presets::Id:
            requestedPresets = true;
            break;
        case Attributes::Schedules::Id:
            requestedSchedules = true;
            break;
        default:
            return System::Clock::Milliseconds16(0);
        }
    }
    if (attributeIdsIter.GetStatus() != CHIP_NO_ERROR)
    {
        return System::Clock::Milliseconds16(0);
    }
    auto timeout = System::Clock::Milliseconds16(0);
    if (requestedPresets)
    {
        // If the client expects to edit the presets, then we'll give it 3 seconds to do so
        timeout += std::chrono::milliseconds(3000);
    }
    if (requestedSchedules)
    {
        // If the client expects to edit the schedules, then we'll give it 9 seconds to do so
        timeout += std::chrono::milliseconds(9000);
    }
    // If the client requested an even smaller timeout, then use that one
    return std::min(timeoutRequest, timeout);
}

void ThermostatDelegate::InitializePendingPresets()
{
    mNextFreeIndexInPendingPresetsList = 0;
    for (uint8_t indexInPresets = 0; indexInPresets < mNextFreeIndexInPresetsList; indexInPresets++)
    {
        mPendingPresets[mNextFreeIndexInPendingPresetsList] = mPresets[indexInPresets];
        mNextFreeIndexInPendingPresetsList++;
    }
}

CHIP_ERROR ThermostatDelegate::AppendToPendingPresetList(const PresetStruct::Type & preset)
{
    if (mNextFreeIndexInPendingPresetsList < ArraySize(mPendingPresets))
    {
        mPendingPresets[mNextFreeIndexInPendingPresetsList] = preset;
        if (preset.presetHandle.IsNull())
        {
            // TODO: #34556 Since we support only one preset of each type, using the octet string containing the preset scenario
            // suffices as the unique preset handle. Need to fix this to actually provide unique handles once multiple presets of
            // each type are supported.
            const uint8_t handle[] = { static_cast<uint8_t>(preset.presetScenario) };
            mPendingPresets[mNextFreeIndexInPendingPresetsList].SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle)));
        }
        mNextFreeIndexInPendingPresetsList++;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_WRITE_FAILED;
}

CHIP_ERROR ThermostatDelegate::GetPendingPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset)
{
    if (index < mNextFreeIndexInPendingPresetsList)
    {
        preset = mPendingPresets[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatDelegate::ApplyPendingPresets()
{
    mNextFreeIndexInPresetsList = 0;
    for (uint8_t indexInPendingPresets = 0; indexInPendingPresets < mNextFreeIndexInPendingPresetsList; indexInPendingPresets++)
    {
        const PresetStructWithOwnedMembers & pendingPreset = mPendingPresets[indexInPendingPresets];
        mPresets[mNextFreeIndexInPresetsList]              = pendingPreset;
        mNextFreeIndexInPresetsList++;
    }
    return CHIP_NO_ERROR;
}

void ThermostatDelegate::ClearPendingPresetList()
{
    mNextFreeIndexInPendingPresetsList = 0;
}
