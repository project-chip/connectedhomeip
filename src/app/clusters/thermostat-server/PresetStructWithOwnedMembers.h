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

/****************************************************************************
 * @file
 * @brief This class has a struct PresetStructWithOwnedMembers that inherits from
 *        Structs::PresetStruct::Type and manages the storage of the preset handle
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

static constexpr size_t kPresetHandleSize = 16;

static constexpr size_t kPresetNameSize = 64;

struct PresetStructWithOwnedMembers : protected Structs::PresetStruct::Type
{
public:
    PresetStructWithOwnedMembers() = default;
    PresetStructWithOwnedMembers(const Structs::PresetStruct::Type & other);
    PresetStructWithOwnedMembers & operator=(const Structs::PresetStruct::Type & other);
    PresetStructWithOwnedMembers & operator=(const PresetStructWithOwnedMembers & other);

    void SetPresetScenario(PresetScenarioEnum enumValue);
    CHIP_ERROR SetPresetHandle(const DataModel::Nullable<ByteSpan> & newPresetHandle);
    CHIP_ERROR SetName(const Optional<DataModel::Nullable<CharSpan>> & newName);
    void SetCoolingSetpoint(const Optional<int16_t> & newCoolingSetpoint);
    void SetHeatingSetpoint(const Optional<int16_t> & newHeatingSetpoint);
    void SetBuiltIn(DataModel::Nullable<bool> newBuiltIn);

    PresetScenarioEnum GetPresetScenario() const;
    DataModel::Nullable<ByteSpan> GetPresetHandle() const;
    Optional<DataModel::Nullable<CharSpan>> GetName() const;
    Optional<int16_t> GetCoolingSetpoint() const;
    Optional<int16_t> GetHeatingSetpoint() const;
    DataModel::Nullable<bool> GetBuiltIn() const;

    using Structs::PresetStruct::Type::Encode;
    using Structs::PresetStruct::Type::kIsFabricScoped;

private:
    uint8_t presetHandleData[kPresetHandleSize] = { 0 };
    char presetNameData[kPresetNameSize];
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
