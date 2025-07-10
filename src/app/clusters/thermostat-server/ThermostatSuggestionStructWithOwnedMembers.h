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

/****************************************************************************
 * @file
 * @brief This class has a struct ThermostatSuggestionStructWithOwnedMembers that inherits from
 *        Structs::ThermostatSuggestionStruct::Type and manages the storage of the preset handle
 *        member which it owns.
 *
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include <app/util/attribute-storage.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

static constexpr size_t kThermostatSuggestionPresetHandleSize = 16;

struct ThermostatSuggestionStructWithOwnedMembers : protected Structs::ThermostatSuggestionStruct::Type
{
public:
    ThermostatSuggestionStructWithOwnedMembers() = default;
    ThermostatSuggestionStructWithOwnedMembers(const ThermostatSuggestionStructWithOwnedMembers & other);
    ThermostatSuggestionStructWithOwnedMembers(const Structs::ThermostatSuggestionStruct::Type & other);
    ThermostatSuggestionStructWithOwnedMembers & operator=(const Structs::ThermostatSuggestionStruct::Type & other);
    ThermostatSuggestionStructWithOwnedMembers & operator=(const ThermostatSuggestionStructWithOwnedMembers & other);

    void SetUniqueID(uint8_t newUniqueID);
    CHIP_ERROR SetPresetHandle(const ByteSpan & newPresetHandle);
    void SetEffectiveTime(const uint32_t newEffectiveTime);
    void SetExpirationTime(const uint32_t newExpirationTime);

    uint8_t GetUniqueID() const;
    const ByteSpan & GetPresetHandle() const;
    uint32_t GetEffectiveTime() const;
    uint32_t GetExpirationTime() const;

    using Structs::ThermostatSuggestionStruct::Type::Encode;
    using Structs::ThermostatSuggestionStruct::Type::kIsFabricScoped;

private:
    uint8_t presetHandleData[kThermostatSuggestionPresetHandleSize] = { 0 };
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
