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

#include "thermostat-server.h"

#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::Protocols::InteractionModel;

namespace {

/**
 * @brief Check if a preset is valid.
 *
 * @param[in] preset The preset to check.
 *
 * @return true If the preset is valid i.e the PresetHandle (if not null) fits within size constraints and the presetScenario enum
 *         value is valid. Otherwise, return false.
 */
bool IsValidPresetEntry(const PresetStructWithOwnedMembers & preset)
{
    // Check that the preset handle is not too long.
    if (!preset.GetPresetHandle().IsNull() && preset.GetPresetHandle().Value().size() > kPresetHandleSize)
    {
        return false;
    }

    // Ensure we have a valid PresetScenario.
    return (preset.GetPresetScenario() != PresetScenarioEnum::kUnknownEnumValue);
}

/**
 * @brief Checks if the preset is built-in
 *
 * @param[in] preset The preset to check.
 *
 * @return true If the preset is built-in, false otherwise.
 */
bool IsBuiltIn(const PresetStructWithOwnedMembers & preset)
{
    return preset.GetBuiltIn().ValueOr(false);
}

/**
 * @brief Checks if the presets are matching i.e the presetHandles are the same.
 *
 * @param[in] preset The preset to check.
 * @param[in] presetToMatch The preset to match with.
 *
 * @return true If the presets match, false otherwise. If both preset handles are null, returns false
 */
bool PresetHandlesExistAndMatch(const PresetStructWithOwnedMembers & preset, const PresetStructWithOwnedMembers & presetToMatch)
{
    return !preset.GetPresetHandle().IsNull() && !presetToMatch.GetPresetHandle().IsNull() &&
        preset.GetPresetHandle().Value().data_equal(presetToMatch.GetPresetHandle().Value());
}

/**
 * @brief Finds an entry in the pending presets list that matches a preset.
 *        The presetHandle of the two presets must match.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] presetToMatch The preset to match with.
 *
 * @return true if a matching entry was found in the pending presets list, false otherwise.
 */
bool MatchingPendingPresetExists(Delegate * delegate, const PresetStructWithOwnedMembers & presetToMatch)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers preset;
        CHIP_ERROR err = delegate->GetPendingPresetAtIndex(i, preset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "MatchingPendingPresetExists: GetPendingPresetAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return false;
        }

        if (PresetHandlesExistAndMatch(preset, presetToMatch))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Finds and returns an entry in the Presets attribute list that matches
 *        a preset, if such an entry exists. The presetToMatch must have a preset handle.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] presetToMatch The preset to match with.
 * @param[out] matchingPreset The preset in the Presets attribute list that has the same PresetHandle as the presetToMatch.
 *
 * @return true if a matching entry was found in the  presets attribute list, false otherwise.
 */
bool GetMatchingPresetInPresets(Delegate * delegate, const DataModel::Nullable<ByteSpan> & presetHandle,
                                PresetStructWithOwnedMembers & matchingPreset)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        CHIP_ERROR err = delegate->GetPresetAtIndex(i, matchingPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "GetMatchingPresetInPresets: GetPresetAtIndex failed with error %" CHIP_ERROR_FORMAT, err.Format());
            return false;
        }

        // Note: presets coming from our delegate always have a handle.
        if (presetHandle.Value().data_equal(matchingPreset.GetPresetHandle().Value()))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Gets the maximum number of presets allowed for a given preset scenario.
 *
 * @param[in]  delegate The delegate to use.
 * @param[in]  presetScenario The presetScenario to match with.
 * @param[out] count The maximum number of presets for the specified presetScenario
 * @return CHIP_NO_ERROR if the maximum number was determined, or an error if not
 */
CHIP_ERROR MaximumPresetScenarioCount(Delegate * delegate, PresetScenarioEnum presetScenario, size_t & count)
{
    count = 0;
    for (uint8_t i = 0; true; i++)
    {
        PresetTypeStruct::Type presetType;
        auto err = delegate->GetPresetTypeAtIndex(i, presetType);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            // We exhausted the list trying to find the preset scenario
            return CHIP_NO_ERROR;
        }
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        if (presetType.presetScenario == presetScenario)
        {
            count = presetType.numberOfPresets;
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_NO_ERROR;
}

/**
 * @brief Returns the count of preset entries in the pending presets list that have the matching presetHandle.
 * @param[in] delegate The delegate to use.
 * @param[in] presetHandleToMatch The preset handle to match.
 *
 * @return count of the number of presets found with the matching presetHandle. Returns 0 if no matching presets were found.
 */
uint8_t CountPresetsInPendingListWithPresetHandle(Delegate * delegate, const ByteSpan & presetHandleToMatch)
{
    uint8_t count = 0;
    VerifyOrReturnValue(delegate != nullptr, count);

    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers preset;
        auto err = delegate->GetPendingPresetAtIndex(i, preset);
        if (err != CHIP_NO_ERROR)
        {
            return count;
        }

        DataModel::Nullable<ByteSpan> presetHandle = preset.GetPresetHandle();
        if (!presetHandle.IsNull() && presetHandle.Value().data_equal(presetHandleToMatch))
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief Checks if the presetType for the given preset scenario supports name in the presetTypeFeatures bitmap.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] presetScenario The presetScenario to match with.
 *
 * @return true if the presetType for the given preset scenario supports name, false otherwise.
 */
bool PresetTypeSupportsNames(Delegate * delegate, PresetScenarioEnum scenario)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        PresetTypeStruct::Type presetType;
        auto err = delegate->GetPresetTypeAtIndex(i, presetType);
        if (err != CHIP_NO_ERROR)
        {
            return false;
        }

        if (presetType.presetScenario == scenario)
        {
            return (presetType.presetTypeFeatures.Has(PresetTypeFeaturesBitmap::kSupportsNames));
        }
    }
    return false;
}

/**
 * @brief Checks if the given preset handle is present in the  presets attribute
 * @param[in] delegate The delegate to use.
 * @param[in] presetHandleToMatch The preset handle to match with.
 *
 * @return true if the given preset handle is present in the  presets attribute list, false otherwise.
 */
bool IsPresetHandlePresentInPresets(Delegate * delegate, const ByteSpan & presetHandleToMatch)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    PresetStructWithOwnedMembers matchingPreset;
    for (uint8_t i = 0; true; i++)
    {
        CHIP_ERROR err = delegate->GetPresetAtIndex(i, matchingPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return false;
        }

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "IsPresetHandlePresentInPresets: GetPresetAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return false;
        }

        if (!matchingPreset.GetPresetHandle().IsNull() && matchingPreset.GetPresetHandle().Value().data_equal(presetHandleToMatch))
        {
            return true;
        }
    }
    return false;
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

extern ThermostatAttrAccess gThermostatAttrAccess;
extern int16_t EnforceHeatingSetpointLimits(int16_t HeatingSetpoint, EndpointId endpoint);
extern int16_t EnforceCoolingSetpointLimits(int16_t CoolingSetpoint, EndpointId endpoint);

Status ThermostatAttrAccess::SetActivePreset(EndpointId endpoint, DataModel::Nullable<ByteSpan> presetHandle)
{

    auto delegate = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        return Status::InvalidInState;
    }

    // If the preset handle passed in the command is not present in the Presets attribute, return INVALID_COMMAND.
    if (!presetHandle.IsNull() && !IsPresetHandlePresentInPresets(delegate, presetHandle.Value()))
    {
        return Status::InvalidCommand;
    }

    CHIP_ERROR err = delegate->SetActivePresetHandle(presetHandle);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set ActivePresetHandle with error %" CHIP_ERROR_FORMAT, err.Format());
        return StatusIB(err).mStatus;
    }

    return Status::Success;
}

CHIP_ERROR ThermostatAttrAccess::AppendPendingPreset(Thermostat::Delegate * delegate, const PresetStruct::Type & newPreset)
{
    PresetStructWithOwnedMembers preset = newPreset;
    if (!IsValidPresetEntry(preset))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (preset.GetPresetHandle().IsNull())
    {
        if (IsBuiltIn(preset))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        // Force to be false, if passed as null
        preset.SetBuiltIn(false);
    }
    else
    {
        // Per spec we need to check that:
        // (a) There is an existing non-pending preset with this handle.
        PresetStructWithOwnedMembers matchingPreset;
        if (!GetMatchingPresetInPresets(delegate, preset.GetPresetHandle().Value(), matchingPreset))
        {
            return CHIP_IM_GLOBAL_STATUS(NotFound);
        }

        // (b) There is no existing pending preset with this handle.
        if (CountPresetsInPendingListWithPresetHandle(delegate, preset.GetPresetHandle().Value()) > 0)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        const auto & presetBuiltIn         = preset.GetBuiltIn();
        const auto & matchingPresetBuiltIn = matchingPreset.GetBuiltIn();
        // (c)/(d) The built-in fields do not have a mismatch.
        if (presetBuiltIn.IsNull())
        {
            if (matchingPresetBuiltIn.IsNull())
            {
                // This really shouldn't happen; internal presets should alway have built-in set
                return CHIP_IM_GLOBAL_STATUS(InvalidInState);
            }
            preset.SetBuiltIn(matchingPresetBuiltIn.Value());
        }
        else
        {
            if (matchingPresetBuiltIn.IsNull())
            {
                // This really shouldn't happen; internal presets should alway have built-in set
                return CHIP_IM_GLOBAL_STATUS(InvalidInState);
            }
            if (presetBuiltIn.Value() != matchingPresetBuiltIn.Value())
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }
    }

    size_t maximumPresetCount         = delegate->GetNumberOfPresets();
    size_t maximumPresetScenarioCount = 0;
    if (MaximumPresetScenarioCount(delegate, preset.GetPresetScenario(), maximumPresetScenarioCount) != CHIP_NO_ERROR)
    {
        return CHIP_IM_GLOBAL_STATUS(InvalidInState);
    }

    if (maximumPresetScenarioCount == 0)
    {
        // This is not a supported preset scenario
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (preset.GetName().HasValue() && !PresetTypeSupportsNames(delegate, preset.GetPresetScenario()))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    // Before adding this preset to the pending presets, if the expected length of the pending presets' list
    // exceeds the total number of presets supported, return RESOURCE_EXHAUSTED. Note that the preset has not been appended yet.

    // We're going to append this preset, so let's assume a count as though it had already been inserted
    size_t presetCount         = 1;
    size_t presetScenarioCount = 1;
    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers otherPreset;
        CHIP_ERROR err = delegate->GetPendingPresetAtIndex(i, otherPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            return CHIP_IM_GLOBAL_STATUS(InvalidInState);
        }
        presetCount++;
        if (preset.GetPresetScenario() == otherPreset.GetPresetScenario())
        {
            presetScenarioCount++;
        }
    }

    if (presetCount > maximumPresetCount)
    {
        ChipLogError(Zcl, "Preset count exceeded %u: %u ", static_cast<unsigned>(maximumPresetCount),
                     static_cast<unsigned>(presetCount));
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    if (presetScenarioCount > maximumPresetScenarioCount)
    {
        ChipLogError(Zcl, "Preset scenario count exceeded %u: %u ", static_cast<unsigned>(maximumPresetScenarioCount),
                     static_cast<unsigned>(presetScenarioCount));
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    return delegate->AppendToPendingPresetList(preset);
}

Status ThermostatAttrAccess::PrecommitPresets(EndpointId endpoint)
{
    auto delegate = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        return Status::InvalidInState;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    // For each preset in the presets attribute, check that the matching preset in the pending presets list does not
    // violate any spec constraints.
    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers preset;
        err = delegate->GetPresetAtIndex(i, preset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl,
                         "PrecommitPresets: GetPresetAtIndex failed with error "
                         "%" CHIP_ERROR_FORMAT,
                         err.Format());
            return Status::InvalidInState;
        }

        bool found = MatchingPendingPresetExists(delegate, preset);

        // If a built in preset in the Presets attribute list is removed and not found in the pending presets list, return
        // CONSTRAINT_ERROR.
        if (IsBuiltIn(preset) && !found)
        {
            return Status::ConstraintError;
        }
    }

    // If there is an ActivePresetHandle set, find the preset in the pending presets list that matches the ActivePresetHandle
    // attribute. If a preset is not found with the same presetHandle, return INVALID_IN_STATE. If there is no ActivePresetHandle
    // attribute set, continue with other checks.
    uint8_t buffer[kPresetHandleSize];
    MutableByteSpan activePresetHandleSpan(buffer);
    auto activePresetHandle = DataModel::MakeNullable(activePresetHandleSpan);

    err = delegate->GetActivePresetHandle(activePresetHandle);

    if (err != CHIP_NO_ERROR)
    {
        return Status::InvalidInState;
    }

    if (!activePresetHandle.IsNull())
    {
        uint8_t count = CountPresetsInPendingListWithPresetHandle(delegate, activePresetHandle.Value());
        if (count == 0)
        {
            return Status::InvalidInState;
        }
    }

    // For each preset in the pending presets list, check that the preset does not violate any spec constraints.
    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers pendingPreset;
        err = delegate->GetPendingPresetAtIndex(i, pendingPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl,
                         "PrecommitPresets: GetPendingPresetAtIndex failed with error "
                         "%" CHIP_ERROR_FORMAT,
                         err.Format());
            return Status::InvalidInState;
        }

        // Enforce the Setpoint Limits for both the cooling and heating setpoints in the pending preset.
        // TODO: This code does not work, because it's modifying our temporary copy.
        Optional<int16_t> coolingSetpointValue = pendingPreset.GetCoolingSetpoint();
        if (coolingSetpointValue.HasValue())
        {
            pendingPreset.SetCoolingSetpoint(MakeOptional(EnforceCoolingSetpointLimits(coolingSetpointValue.Value(), endpoint)));
        }

        Optional<int16_t> heatingSetpointValue = pendingPreset.GetHeatingSetpoint();
        if (heatingSetpointValue.HasValue())
        {
            pendingPreset.SetHeatingSetpoint(MakeOptional(EnforceHeatingSetpointLimits(heatingSetpointValue.Value(), endpoint)));
        }
    }

    return Status::Success;
}

bool emberAfThermostatClusterSetActivePresetRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                            const Commands::SetActivePresetRequest::DecodableType & commandData)
{
    auto status = gThermostatAttrAccess.SetActivePreset(commandPath.mEndpointId, commandData.presetHandle);
    commandObj->AddStatus(commandPath, status);
    return true;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfThermostatClusterSetActivePresetRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                            const Commands::SetActivePresetRequest::DecodableType & commandData)
{
    return Thermostat::emberAfThermostatClusterSetActivePresetRequestCallback(commandObj, commandPath, commandData);
}
