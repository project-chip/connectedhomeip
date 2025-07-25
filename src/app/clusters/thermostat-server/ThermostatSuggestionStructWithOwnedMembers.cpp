/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "ThermostatSuggestionStructWithOwnedMembers.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace System::Clock;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

ThermostatSuggestionStructWithOwnedMembers::ThermostatSuggestionStructWithOwnedMembers(
    const ThermostatSuggestionStruct::Type & other)
{
    *this = other;
}

ThermostatSuggestionStructWithOwnedMembers::ThermostatSuggestionStructWithOwnedMembers(
    const ThermostatSuggestionStructWithOwnedMembers & other) :
    ThermostatSuggestionStructWithOwnedMembers(static_cast<const Structs::ThermostatSuggestionStruct::Type &>(other))
{}

ThermostatSuggestionStructWithOwnedMembers &
ThermostatSuggestionStructWithOwnedMembers::operator=(const ThermostatSuggestionStruct::Type & other)
{
    SetUniqueID(other.uniqueID);
    CHIP_ERROR err = SetPresetHandle(other.presetHandle);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set Preset handle with err %" CHIP_ERROR_FORMAT, err.Format());
    }
    SetEffectiveTime(Seconds32(other.effectiveTime));
    SetExpirationTime(Seconds32(other.expirationTime));
    return *this;
}

ThermostatSuggestionStructWithOwnedMembers &
ThermostatSuggestionStructWithOwnedMembers::operator=(const ThermostatSuggestionStructWithOwnedMembers & other)
{
    if (this == &other)
    {
        return *this;
    }
    *this = static_cast<const ThermostatSuggestionStruct::Type &>(other);
    return *this;
}

void ThermostatSuggestionStructWithOwnedMembers::SetUniqueID(uint8_t newUniqueID)
{
    uniqueID = newUniqueID;
}

CHIP_ERROR ThermostatSuggestionStructWithOwnedMembers::SetPresetHandle(const ByteSpan & newPresetHandle)
{
    size_t newPresetHandleSize = newPresetHandle.size();
    if (newPresetHandleSize > kThermostatSuggestionPresetHandleSize)
    {
        ChipLogError(Zcl, "Failed to set Preset handle. New preset handle size (%u) > allowed preset handle size (%u)",
                     static_cast<unsigned>(newPresetHandleSize), static_cast<unsigned>(kThermostatSuggestionPresetHandleSize));
        return CHIP_ERROR_NO_MEMORY;
    }
    MutableByteSpan targetSpan(mPresetHandleData);
    ReturnErrorOnFailure(CopySpanToMutableSpan(newPresetHandle, targetSpan));

    presetHandle = targetSpan;

    return CHIP_NO_ERROR;
}

void ThermostatSuggestionStructWithOwnedMembers::SetEffectiveTime(const Seconds32 newEffectiveTime)
{
    effectiveTime = newEffectiveTime.count();
}

void ThermostatSuggestionStructWithOwnedMembers::SetExpirationTime(const Seconds32 newExpirationTime)
{
    expirationTime = newExpirationTime.count();
}

uint8_t ThermostatSuggestionStructWithOwnedMembers::GetUniqueID() const
{
    return uniqueID;
}

const ByteSpan & ThermostatSuggestionStructWithOwnedMembers::GetPresetHandle() const
{
    return presetHandle;
}

Seconds32 ThermostatSuggestionStructWithOwnedMembers::GetEffectiveTime() const
{
    return Seconds32(effectiveTime);
}

Seconds32 ThermostatSuggestionStructWithOwnedMembers::GetExpirationTime() const
{
    return Seconds32(expirationTime);
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
