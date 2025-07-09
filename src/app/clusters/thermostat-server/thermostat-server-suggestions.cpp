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

#include "thermostat-server.h"
#include "thermostat-server-presets.h"
#include "ThermostatSuggestionStructWithOwnedMembers.h"

#include <system/SystemClock.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

extern ThermostatAttrAccess gThermostatAttrAccess;

bool emberAfThermostatClusterAddThermostatSuggestionCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                            const Commands::AddThermostatSuggestion::DecodableType & commandData)
{

    // If time is not synced, return INVALID_IN_STATE in the AddThermostatSuggestionResponse.
    uint32_t currentTimestamp = 0;
    if (System::Clock::GetClock_MatterEpochS(currentTimestamp) != CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    auto delegate = GetDelegate(commandPath.mEndpointId);
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
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
    if (!commandData.effectiveTime.IsNull() && (commandData.effectiveTime.Value() > currentTimestamp + 86400))
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }
    

    uint8_t uniqueID = delegate->GetUniqueID();
    ThermostatSuggestionStructWithOwnedMembers thermostatSuggestion;
    thermostatSuggestion.SetUniqueID(uniqueID);
    thermostatSuggestion.SetPresetHandle(commandData.presetHandle);

    uint32_t effectiveTime = commandData.effectiveTime.ValueOr(currentTimestamp);
    thermostatSuggestion.SetEffectiveTime(effectiveTime);
    thermostatSuggestion.SetExpirationTime(effectiveTime + (commandData.expirationInMinutes * 60));

    CHIP_ERROR err = delegate->AppendToThermostatSuggestionsList(thermostatSuggestion);

    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
    {
        ChipLogError(Zcl, "Failed to AppendToThermostatSuggestionsList with error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    err = delegate->ReEvaluateCurrentSuggestion(currentTimestamp);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to ReEvaluateCurrentSuggestion with error: %" CHIP_ERROR_FORMAT, err.Format());
    }
   
    Commands::AddThermostatSuggestionResponse::Type response;
    response.uniqueID = uniqueID;
    commandObj->AddResponse(commandPath, response); 
    return true;
}

bool emberAfThermostatClusterRemoveThermostatSuggestionCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                            const Commands::RemoveThermostatSuggestion::DecodableType & commandData)
{
    auto delegate = GetDelegate(commandPath.mEndpointId);
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }
    
    CHIP_ERROR err = delegate->RemoveFromThermostatSuggestionsList(commandData.uniqueID);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to RemoveFromThermostatSuggestionsList with uniqueID: %d error: %" CHIP_ERROR_FORMAT, commandData.uniqueID, err.Format());
        commandObj->AddStatus(commandPath, Status::NotFound);
        return true;
    }
    commandObj->AddStatus(commandPath, Status::Success);

    uint32_t currentTimestamp = 0;
    err = System::Clock::GetClock_MatterEpochS(currentTimestamp);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get the current time stamp with error: %" CHIP_ERROR_FORMAT, err.Format());
        return true;
    }

    err = delegate->ReEvaluateCurrentSuggestion(currentTimestamp);

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
    return Thermostat::emberAfThermostatClusterAddThermostatSuggestionCallback(commandObj, commandPath, commandData);
}

bool emberAfThermostatClusterRemoveThermostatSuggestionCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                                           const Clusters::Thermostat::Commands::RemoveThermostatSuggestion::DecodableType & commandData)
{
    return Thermostat::emberAfThermostatClusterRemoveThermostatSuggestionCallback(commandObj, commandPath, commandData);
}

    
