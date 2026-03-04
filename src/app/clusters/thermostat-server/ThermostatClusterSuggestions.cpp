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

#include "ThermostatCluster.h"
#include "ThermostatClusterPresets.h"
#include "ThermostatSuggestionStructWithOwnedMembers.h"

#include <app/reporting/reporting.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::Protocols::InteractionModel;
using namespace System::Clock;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

namespace {

CHIP_ERROR RemoveExpiredSuggestions(Delegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint32_t currentMatterEpochTimestampInSeconds = 0;
    CHIP_ERROR err                                = System::Clock::GetClock_MatterEpochS(currentMatterEpochTimestampInSeconds);
    ReturnErrorOnFailure(err);

    for (int i = static_cast<int>(delegate->GetNumberOfThermostatSuggestions() - 1); i >= 0; i--)
    {

        ThermostatSuggestionStructWithOwnedMembers suggestion;
        err = delegate->GetThermostatSuggestionAtIndex(static_cast<size_t>(i), suggestion);
        ReturnErrorOnFailure(err);

        if (suggestion.GetExpirationTime() <= Seconds32(currentMatterEpochTimestampInSeconds))
        {
            err = delegate->RemoveFromThermostatSuggestionsList(static_cast<size_t>(i));
            ReturnErrorOnFailure(err);
        }
    }
    return err;
}

Status RemoveFromThermostatSuggestionsList(Delegate * delegate, uint8_t uniqueIDToRemove)
{
    VerifyOrReturnValue(delegate != nullptr, Status::Failure);

    size_t uniqueIDMatchedIndex = 0;
    CHIP_ERROR err              = CHIP_NO_ERROR;

    for (size_t index = 0; true; ++index)
    {
        ThermostatSuggestionStructWithOwnedMembers suggestion;
        err = delegate->GetThermostatSuggestionAtIndex(index, suggestion);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return Status::NotFound;
        }

        if (suggestion.GetUniqueID() == uniqueIDToRemove)
        {
            uniqueIDMatchedIndex = index;
            break;
        }
    };

    err = delegate->RemoveFromThermostatSuggestionsList(uniqueIDMatchedIndex);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, Status::Failure);
    return Status::Success;
}

} // anonymous namespace

std::optional<DataModel::ActionReturnStatus>
ThermostatCluster::AddThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                           const Commands::AddThermostatSuggestion::DecodableType & commandData)
{
    if (commandData.presetHandle.size() >= kThermostatSuggestionPresetHandleSize)
    {
        return Status::ConstraintError;
    }

    if (commandData.expirationInMinutes < 30 || commandData.expirationInMinutes > 1440)
    {
        return Status::ConstraintError;
    }

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null for endpoint %u", commandPath.mEndpointId);
        return Status::InvalidInState;
    }

    // If time is not synced, return INVALID_IN_STATE in the AddThermostatSuggestionResponse.
    uint32_t currentMatterEpochTimestampInSeconds = 0;
    if (System::Clock::GetClock_MatterEpochS(currentMatterEpochTimestampInSeconds) != CHIP_NO_ERROR)
    {
        return Status::InvalidInState;
    }

    // If the preset hande doesn't exist in the Presets attribute, return NOT_FOUND.
    if (!IsPresetHandlePresentInPresets(mDelegate, commandData.presetHandle))
    {
        return Status::NotFound;
    }

    // If the thermostat suggestions list is full, return RESOURCE_EXHAUSTED.
    if (mDelegate->GetNumberOfThermostatSuggestions() >= mDelegate->GetMaxThermostatSuggestions())
    {
        commandObj->AddStatus(commandPath, Status::ResourceExhausted);
        return Status::ResourceExhausted;
    }

    // If the effective time in UTC is greater than current time in UTC plus 24 hours, return INVALID_COMMAND.
    const uint32_t kSecondsInDay = 24 * 60 * 60;
    if (!commandData.effectiveTime.IsNull() &&
        (commandData.effectiveTime.Value() > currentMatterEpochTimestampInSeconds + kSecondsInDay))
    {
        return Status::InvalidCommand;
    }

    // Remove any expired suggestions before adding to the list.
    CHIP_ERROR err = RemoveExpiredSuggestions(mDelegate);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to RemoveExpiredSuggestions at endpoint %u with error: %" CHIP_ERROR_FORMAT,
                     commandPath.mEndpointId, err.Format());
        return Status::Failure;
    }

    uint8_t uniqueID = 0;
    err              = mDelegate->GetUniqueID(uniqueID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to GetUniqueID at endpoint %u with error: %" CHIP_ERROR_FORMAT, commandPath.mEndpointId,
                     err.Format());
        return Status::Failure;
    }

    Structs::ThermostatSuggestionStruct::Type thermostatSuggestion;
    thermostatSuggestion.uniqueID     = uniqueID;
    thermostatSuggestion.presetHandle = commandData.presetHandle;

    uint32_t effectiveTime             = commandData.effectiveTime.ValueOr(currentMatterEpochTimestampInSeconds);
    thermostatSuggestion.effectiveTime = effectiveTime;

    thermostatSuggestion.expirationTime = effectiveTime + (commandData.expirationInMinutes * kSecondsPerMinute);

    err = mDelegate->AppendToThermostatSuggestionsList(thermostatSuggestion);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to AppendToThermostatSuggestionsList at endpoint %u with error: %" CHIP_ERROR_FORMAT,
                     commandPath.mEndpointId, err.Format());
        return Status::Failure;
    }

    NotifyAttributeChanged(ThermostatSuggestions::Id);

    // Re-evaluate the current thermostat suggestion.
    TEMPORARY_RETURN_IGNORED mDelegate->ReEvaluateCurrentSuggestion();

    Commands::AddThermostatSuggestionResponse::Type response;
    response.uniqueID = uniqueID;
    commandObj->AddResponse(commandPath, response);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
ThermostatCluster::RemoveThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                              const Commands::RemoveThermostatSuggestion::DecodableType & commandData)
{

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null for endpoint %u", commandPath.mEndpointId);
        return Status::InvalidInState;
    }

    Status status = RemoveFromThermostatSuggestionsList(mDelegate, commandData.uniqueID);

    if (status != Status::Success)
    {
        ChipLogError(Zcl,
                     "Failed to RemoveFromThermostatSuggestionsList at endpoint %u with uniqueID: %u status:" ChipLogFormatIMStatus,
                     commandPath.mEndpointId, commandData.uniqueID, ChipLogValueIMStatus(status));
        return status;
    }

    NotifyAttributeChanged(ThermostatSuggestions::Id);

    // Remove expired suggestions if any and re-evaluate the current thermostat suggestion.
    TEMPORARY_RETURN_IGNORED RemoveExpiredSuggestions(mDelegate);
    TEMPORARY_RETURN_IGNORED mDelegate->ReEvaluateCurrentSuggestion();

    return Status::Success;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
