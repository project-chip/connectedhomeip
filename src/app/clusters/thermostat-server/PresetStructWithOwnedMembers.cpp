/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "PresetStructWithOwnedMembers.h"

using namespace chip;
using namespace chip::app;
using namespace DataModel;
using namespace chip::app::Clusters::Thermostat::Structs;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

PresetStructWithOwnedMembers::PresetStructWithOwnedMembers(const PresetStruct::Type & other)
{
    *this = other;
}

PresetStructWithOwnedMembers & PresetStructWithOwnedMembers::operator=(const PresetStruct::Type & other)
{
    SetPresetScenario(other.presetScenario);
    CHIP_ERROR err = SetPresetHandle(other.presetHandle);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set Preset handle with err %" CHIP_ERROR_FORMAT, err.Format());
    }
    err = SetName(other.name);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set Preset name with err %" CHIP_ERROR_FORMAT, err.Format());
    }
    SetCoolingSetpoint(other.coolingSetpoint);
    SetHeatingSetpoint(other.heatingSetpoint);
    SetBuiltIn(other.builtIn);
    return *this;
}

PresetStructWithOwnedMembers & PresetStructWithOwnedMembers::operator=(const PresetStructWithOwnedMembers & other)
{
    if (this == &other)
    {
        return *this;
    }
    *this = static_cast<const PresetStruct::Type &>(other);
    return *this;
}

void PresetStructWithOwnedMembers::SetPresetScenario(PresetScenarioEnum enumValue)
{
    presetScenario = enumValue;
}

CHIP_ERROR PresetStructWithOwnedMembers::SetPresetHandle(const Nullable<ByteSpan> & newPresetHandle)
{
    if (!newPresetHandle.IsNull())
    {
        size_t newPresetHandleSize = newPresetHandle.Value().size();
        if (newPresetHandleSize > kPresetHandleSize)
        {
            ChipLogError(Zcl, "Failed to set Preset handle. New preset handle size (%u) > allowed preset handle size (%u)",
                         static_cast<unsigned>(newPresetHandleSize), static_cast<unsigned>(kPresetNameSize));
            return CHIP_ERROR_NO_MEMORY;
        }
        MutableByteSpan targetSpan(presetHandleData);
        ReturnErrorOnFailure(CopySpanToMutableSpan(newPresetHandle.Value(), targetSpan));
        presetHandle.SetNonNull(targetSpan);
    }
    else
    {
        presetHandle.SetNull();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PresetStructWithOwnedMembers::SetName(const Optional<DataModel::Nullable<CharSpan>> & newName)
{
    if (newName.HasValue() && !newName.Value().IsNull())
    {
        size_t newNameSize = newName.Value().Value().size();
        if (newNameSize > kPresetNameSize)
        {
            ChipLogError(Zcl, "Failed to set Preset name. New name size (%u) > allowed preset name size (%u)",
                         static_cast<unsigned>(newNameSize), static_cast<unsigned>(kPresetNameSize));
            return CHIP_ERROR_NO_MEMORY;
        }
        MutableCharSpan targetSpan(presetNameData);
        CharSpan newNameSpan = newName.Value().Value();
        ReturnErrorOnFailure(CopyCharSpanToMutableCharSpan(newNameSpan, targetSpan));

        DataModel::Nullable<CharSpan> nullableCharSpan;
        nullableCharSpan.SetNonNull(targetSpan);
        name.SetValue(nullableCharSpan);
    }
    else
    {
        name.ClearValue();
    }
    return CHIP_NO_ERROR;
}

void PresetStructWithOwnedMembers::SetCoolingSetpoint(const Optional<int16_t> & newCoolingSetpoint)
{
    coolingSetpoint = newCoolingSetpoint;
}

void PresetStructWithOwnedMembers::SetHeatingSetpoint(const Optional<int16_t> & newHeatingSetpoint)
{
    heatingSetpoint = newHeatingSetpoint;
}

void PresetStructWithOwnedMembers::SetBuiltIn(DataModel::Nullable<bool> newBuiltIn)
{
    builtIn = newBuiltIn;
}

PresetScenarioEnum PresetStructWithOwnedMembers::GetPresetScenario() const
{
    return presetScenario;
}

DataModel::Nullable<ByteSpan> PresetStructWithOwnedMembers::GetPresetHandle() const
{
    return presetHandle;
}

Optional<DataModel::Nullable<CharSpan>> PresetStructWithOwnedMembers::GetName() const
{
    return name;
}

Optional<int16_t> PresetStructWithOwnedMembers::GetCoolingSetpoint() const
{
    return coolingSetpoint;
}

Optional<int16_t> PresetStructWithOwnedMembers::GetHeatingSetpoint() const
{
    return heatingSetpoint;
}

DataModel::Nullable<bool> PresetStructWithOwnedMembers::GetBuiltIn() const
{
    return builtIn;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
