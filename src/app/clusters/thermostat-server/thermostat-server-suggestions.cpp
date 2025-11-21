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
#include "thermostat-server-presets.h"
#include "thermostat-server.h"

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

bool AddThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                             const Commands::AddThermostatSuggestion::DecodableType & commandData)
{
    ChipLogError(Zcl, "checking preset handle size");

    // Check constraints for PresetHandle and ExpirationInMinutes field.
    if (commandData.presetHandle.size() >= kThermostatSuggestionPresetHandleSize)
    {
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }

    ChipLogError(Zcl, "checking expirationInMinutes");
    if (commandData.expirationInMinutes < 30 || commandData.expirationInMinutes > 1440)
    {
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }

    EndpointId endpoint = commandPath.mEndpointId;
    auto delegate       = GetDelegate(endpoint);
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null for endpoint %u", endpoint);
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    ChipLogError(Zcl, "checking delegate");
    // If time is not synced, return INVALID_IN_STATE in the AddThermostatSuggestionResponse.
    uint32_t currentMatterEpochTimestampInSeconds = 0;
    if (System::Clock::GetClock_MatterEpochS(currentMatterEpochTimestampInSeconds) != CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    ChipLogError(Zcl, "calling IsPresetHandlePresentInPresets");
    // If the preset hande doesn't exist in the Presets attribute, return NOT_FOUND.
    if (!IsPresetHandlePresentInPresets(delegate, commandData.presetHandle))
    {
        commandObj->AddStatus(commandPath, Status::NotFound);
        return true;
    }

    ChipLogError(Zcl, "checking thermostat suggestions list");
    // If the thermostat suggestions list is full, return RESOURCE_EXHAUSTED.
    if (delegate->GetNumberOfThermostatSuggestions() >= delegate->GetMaxThermostatSuggestions())
    {
        commandObj->AddStatus(commandPath, Status::ResourceExhausted);
        return true;
    }

    ChipLogError(Zcl, "checking time < 24");
    // If the effective time in UTC is greater than current time in UTC plus 24 hours, return INVALID_COMMAND.
    const uint32_t kSecondsInDay = 24 * 60 * 60;
    if (!commandData.effectiveTime.IsNull() &&
        (commandData.effectiveTime.Value() > currentMatterEpochTimestampInSeconds + kSecondsInDay))
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    // Remove any expired suggestions before adding to the list.
    CHIP_ERROR err = RemoveExpiredSuggestions(delegate);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to RemoveExpiredSuggestions at endpoint %u with error: %" CHIP_ERROR_FORMAT, endpoint,
                     err.Format());
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    uint8_t uniqueID = 0;
    err              = delegate->GetUniqueID(uniqueID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to GetUniqueID at endpoint %u with error: %" CHIP_ERROR_FORMAT, endpoint, err.Format());
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    Structs::ThermostatSuggestionStruct::Type thermostatSuggestion;
    thermostatSuggestion.uniqueID     = uniqueID;
    thermostatSuggestion.presetHandle = commandData.presetHandle;

    uint32_t effectiveTime             = commandData.effectiveTime.ValueOr(currentMatterEpochTimestampInSeconds);
    thermostatSuggestion.effectiveTime = effectiveTime;

    thermostatSuggestion.expirationTime = effectiveTime + (commandData.expirationInMinutes * kSecondsPerMinute);

    err = delegate->AppendToThermostatSuggestionsList(thermostatSuggestion);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to AppendToThermostatSuggestionsList at endpoint %u with error: %" CHIP_ERROR_FORMAT, endpoint,
                     err.Format());
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    MatterReportingAttributeChangeCallback(endpoint, Thermostat::Id, ThermostatSuggestions::Id);

    // Re-evaluate the current thermostat suggestion.
    TEMPORARY_RETURN_IGNORED delegate->ReEvaluateCurrentSuggestion();

    Commands::AddThermostatSuggestionResponse::Type response;
    response.uniqueID = uniqueID;
    commandObj->AddResponse(commandPath, response);

    return true;
}

bool RemoveThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                const Commands::RemoveThermostatSuggestion::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;
    auto delegate       = GetDelegate(endpoint);
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null for endpoint %u", endpoint);
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    Status status = RemoveFromThermostatSuggestionsList(delegate, commandData.uniqueID);

    if (status != Status::Success)
    {
        ChipLogError(Zcl,
                     "Failed to RemoveFromThermostatSuggestionsList at endpoint %u with uniqueID: %u status:" ChipLogFormatIMStatus,
                     endpoint, commandData.uniqueID, ChipLogValueIMStatus(status));
        commandObj->AddStatus(commandPath, status);
        return true;
    }

    MatterReportingAttributeChangeCallback(endpoint, Thermostat::Id, ThermostatSuggestions::Id);

    // Remove expired suggestions if any and re-evaluate the current thermostat suggestion.
    TEMPORARY_RETURN_IGNORED RemoveExpiredSuggestions(delegate);
    TEMPORARY_RETURN_IGNORED delegate->ReEvaluateCurrentSuggestion();

    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfThermostatClusterAddThermostatSuggestionCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                             const Commands::AddThermostatSuggestion::DecodableType & commandData)
{
    return Thermostat::AddThermostatSuggestion(commandObj, commandPath, commandData);
}

bool emberAfThermostatClusterRemoveThermostatSuggestionCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Clusters::Thermostat::Commands::RemoveThermostatSuggestion::DecodableType & commandData)
{
    return Thermostat::RemoveThermostatSuggestion(commandObj, commandPath, commandData);
}
