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

extern ThermostatAttrAccess gThermostatAttrAccess;

bool AddThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                             const Commands::AddThermostatSuggestion::DecodableType & commandData)
{
    // Check constraints for PresetHandle and ExpirationInMinutes field.
    if (commandData.presetHandle.size() >= kThermostatSuggestionPresetHandleSize)
    {
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }

    if (commandData.expirationInMinutes < 30 || commandData.expirationInMinutes > 1440)
    {
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }

    EndpointId endpoint = commandPath.mEndpointId;
    auto delegate       = GetDelegate(endpoint);
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    // If time is not synced, return INVALID_IN_STATE in the AddThermostatSuggestionResponse.
    uint32_t currentMatterEpochTimestampInSeconds = 0;
    if (System::Clock::GetClock_MatterEpochS(currentMatterEpochTimestampInSeconds) != CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    // If the preset hande doesn't exist in the Presets attribute, return NOT_FOUND.
    if (!IsPresetHandlePresentInPresets(delegate, commandData.presetHandle))
    {
        commandObj->AddStatus(commandPath, Status::NotFound);
        return true;
    }

    // If the thermostat suggestions list is full, return RESOURCE_EXHAUSTED.
    if (delegate->GetNumberOfThermostatSuggestions() >= delegate->GetMaxThermostatSuggestions())
    {
        commandObj->AddStatus(commandPath, Status::ResourceExhausted);
        return true;
    }

    // If the effective time in UTC is greater than current time in UTC plus 24 hours, return INVALID_COMMAND.
    const uint32_t kSecondsInDay = 24 * 60 * 60;
    if (!commandData.effectiveTime.IsNull() &&
        (commandData.effectiveTime.Value() > currentMatterEpochTimestampInSeconds + kSecondsInDay))
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    uint8_t uniqueID = delegate->GetUniqueID();

    Structs::ThermostatSuggestionStruct::Type thermostatSuggestion;
    thermostatSuggestion.uniqueID     = uniqueID;
    thermostatSuggestion.presetHandle = commandData.presetHandle;

    uint32_t effectiveTime             = commandData.effectiveTime.ValueOr(currentMatterEpochTimestampInSeconds);
    thermostatSuggestion.effectiveTime = effectiveTime;

    const uint32_t kSecondsInMinute     = 60;
    thermostatSuggestion.expirationTime = effectiveTime + (commandData.expirationInMinutes * kSecondsInMinute);

    CHIP_ERROR err = delegate->AppendToThermostatSuggestionsList(thermostatSuggestion);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to AppendToThermostatSuggestionsList with error: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    MatterReportingAttributeChangeCallback(endpoint, Thermostat::Id, ThermostatSuggestions::Id);

    Commands::AddThermostatSuggestionResponse::Type response;
    response.uniqueID = uniqueID;
    commandObj->AddResponse(commandPath, response);

    // The re-evalaution is done after sending a response since the result of the re-evaluation doesn't affect the response.
    err = delegate->ReEvaluateCurrentSuggestion();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to ReEvaluateCurrentSuggestion with error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return true;
}

bool RemoveThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                const Commands::RemoveThermostatSuggestion::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;
    auto delegate       = GetDelegate(endpoint);
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    CHIP_ERROR err = delegate->RemoveFromThermostatSuggestionsList(commandData.uniqueID);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to RemoveFromThermostatSuggestionsList with uniqueID: %u error: %" CHIP_ERROR_FORMAT,
                     commandData.uniqueID, err.Format());
        commandObj->AddStatus(commandPath, Status::NotFound);
        return true;
    }

    MatterReportingAttributeChangeCallback(endpoint, Thermostat::Id, ThermostatSuggestions::Id);
    commandObj->AddStatus(commandPath, Status::Success);

    // The re-evalaution is done after sending a response since the result of the re-evaluation doesn't affect the response status.
    err = delegate->ReEvaluateCurrentSuggestion();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to ReEvaluateCurrentSuggestion with error: %" CHIP_ERROR_FORMAT, err.Format());
    }
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
