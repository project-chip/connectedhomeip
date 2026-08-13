/**
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

#include "ThermostatClusterSuggestions.h"
#include "ThermostatCluster.h"
#include "ThermostatSuggestionStructWithOwnedMembers.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/reporting/reporting.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>

using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::Protocols::InteractionModel;
using namespace chip::System::Clock;

namespace {

constexpr uint16_t kMinExpirationInMinutes = 30;
constexpr uint16_t kMaxExpirationInMinutes = 1440;

CHIP_ERROR RemoveExpiredSuggestions(chip::app::Clusters::Thermostat::ThermostatSuggestions::Delegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint32_t currentMatterEpochTimestampInSeconds = 0;
    CHIP_ERROR err                                = chip::System::Clock::GetClock_MatterEpochS(currentMatterEpochTimestampInSeconds);
    ReturnErrorOnFailure(err);

    size_t suggestionCount = delegate->GetNumberOfThermostatSuggestions();

    if (suggestionCount == 0)
    {
        return CHIP_NO_ERROR;
    }

    for (int i = static_cast<int>(suggestionCount - 1); i >= 0; i--)
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

Status RemoveFromThermostatSuggestionsList(chip::app::Clusters::Thermostat::ThermostatSuggestions::Delegate * delegate,
                                           uint8_t uniqueIDToRemove)
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

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

std::optional<DataModel::ActionReturnStatus> ThermostatSuggestions::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                  AttributeValueEncoder & encoder)
{
    if (mDelegate == nullptr)
    {
        return std::nullopt;
    }

    switch (request.path.mAttributeId)
    {
    case MaxThermostatSuggestions::Id:
        return encoder.Encode(mDelegate->GetMaxThermostatSuggestions());
    case Attributes::ThermostatSuggestions::Id: {
        auto & delegate = mDelegate;
        return encoder.EncodeList([delegate](const auto & enc) -> CHIP_ERROR {
            for (size_t i = 0; true; i++)
            {
                ThermostatSuggestionStructWithOwnedMembers thermostatSuggestion;
                auto err = delegate->GetThermostatSuggestionAtIndex(i, thermostatSuggestion);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(enc.Encode(thermostatSuggestion));
            }
        });
    }
    case CurrentThermostatSuggestion::Id: {
        DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> currentThermostatSuggestion;
        mDelegate->GetCurrentThermostatSuggestion(currentThermostatSuggestion);
        return encoder.Encode(currentThermostatSuggestion);
    }
    case ThermostatSuggestionNotFollowingReason::Id:
        return encoder.Encode(mDelegate->GetThermostatSuggestionNotFollowingReason());
    default:
        return std::nullopt;
    }
}

std::optional<DataModel::ActionReturnStatus> ThermostatSuggestions::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                  TLV::TLVReader & input_arguments,
                                                                                  CommandHandler * handler, bool & handled)
{
    if (mDelegate == nullptr)
    {
        return std::nullopt;
    }

    switch (request.path.mCommandId)
    {
    case Commands::AddThermostatSuggestion::Id: {
        Commands::AddThermostatSuggestion::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return AddThermostatSuggestion(handler, request.path, commandData, handled);
    }
    case Commands::RemoveThermostatSuggestion::Id: {
        Commands::RemoveThermostatSuggestion::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return RemoveThermostatSuggestion(handler, request.path, commandData);
    }
    default:
        return std::nullopt;
    }
}

std::optional<DataModel::ActionReturnStatus>
ThermostatSuggestions::AddThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                               const Commands::AddThermostatSuggestion::DecodableType & commandData, bool & handled)
{
    if (commandData.presetHandle.size() > kThermostatSuggestionPresetHandleSize)
    {
        return Status::ConstraintError;
    }

    if (commandData.expirationInMinutes < kMinExpirationInMinutes || commandData.expirationInMinutes > kMaxExpirationInMinutes)
    {
        return Status::ConstraintError;
    }

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "SuggestionsDelegate is null for endpoint %u", commandPath.mEndpointId);
        return Status::InvalidInState;
    }

    // If time is not synced, return INVALID_IN_STATE in the AddThermostatSuggestionResponse.
    uint32_t currentMatterEpochTimestampInSeconds = 0;
    if (chip::System::Clock::GetClock_MatterEpochS(currentMatterEpochTimestampInSeconds) != CHIP_NO_ERROR)
    {
        return Status::InvalidInState;
    }

    // If the preset hande doesn't exist in the Presets attribute, return NOT_FOUND.
    if (!mPresets.IsPresetHandlePresentInPresets(commandData.presetHandle))
    {
        return Status::NotFound;
    }

    // If the thermostat suggestions list is full, return RESOURCE_EXHAUSTED.
    if (mDelegate->GetNumberOfThermostatSuggestions() >= mDelegate->GetMaxThermostatSuggestions())
    {
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

    uint32_t effectiveTime              = commandData.effectiveTime.ValueOr(currentMatterEpochTimestampInSeconds);
    thermostatSuggestion.effectiveTime  = effectiveTime;
    thermostatSuggestion.expirationTime = effectiveTime + (commandData.expirationInMinutes * kSecondsPerMinute);

    err = mDelegate->AppendToThermostatSuggestionsList(thermostatSuggestion);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to AppendToThermostatSuggestionsList at endpoint %u with error: %" CHIP_ERROR_FORMAT,
                     commandPath.mEndpointId, err.Format());
        return Status::Failure;
    }

    mCluster.NotifyAttributeChanged(Attributes::ThermostatSuggestions::Id);

    // Re-evaluate the current thermostat suggestion.
    ReEvaluateCurrentSuggestion();

    Commands::AddThermostatSuggestionResponse::Type response;
    response.uniqueID = uniqueID;
    commandObj->AddResponse(commandPath, response);
    handled = true;

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
ThermostatSuggestions::RemoveThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                  const Commands::RemoveThermostatSuggestion::DecodableType & commandData)
{
    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "SuggestionsDelegate is null for endpoint %u", commandPath.mEndpointId);
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

    mCluster.NotifyAttributeChanged(Attributes::ThermostatSuggestions::Id);

    // Remove expired suggestions if any and re-evaluate the current thermostat suggestion.
    LogErrorOnFailure(RemoveExpiredSuggestions(mDelegate));
    ReEvaluateCurrentSuggestion();

    return Status::Success;
}

void ThermostatSuggestions::ReEvaluateCurrentSuggestion()
{
    if (mDelegate == nullptr)
    {
        return;
    }

    DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> currentSuggestionBeforeReevaluation;
    mDelegate->GetCurrentThermostatSuggestion(currentSuggestionBeforeReevaluation);

    uint8_t buffer[kPresetHandleSize];
    MutableByteSpan beforeReevaluationHandleSpan(buffer);
    auto beforeReevaluationHandle = DataModel::MakeNullable(beforeReevaluationHandleSpan);

    if (auto presetsDelegate = mPresets.GetDelegate(); presetsDelegate != nullptr)
    {
        CHIP_ERROR err = presetsDelegate->GetActivePresetHandle(beforeReevaluationHandle);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Failed to GetActivePresetHandle with error: %" CHIP_ERROR_FORMAT, err.Format());
            return;
        }
    }

    CHIP_ERROR err = mDelegate->ReEvaluateCurrentSuggestion();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to ReEvaluateCurrentSuggestion with error: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> currentSuggestionAfterReevaluation;
    mDelegate->GetCurrentThermostatSuggestion(currentSuggestionAfterReevaluation);

    bool suggestionChanged = currentSuggestionBeforeReevaluation.IsNull() != currentSuggestionAfterReevaluation.IsNull();
    if (!suggestionChanged && !currentSuggestionAfterReevaluation.IsNull())
    {
        suggestionChanged =
            currentSuggestionBeforeReevaluation.Value().GetUniqueID() != currentSuggestionAfterReevaluation.Value().GetUniqueID();
    }

    if (suggestionChanged)
    {
        // If the current thermostat suggestion changed, notify the attribute changed.
        mCluster.NotifyAttributeChanged(CurrentThermostatSuggestion::Id);
    }

    if (auto presetsDelegate = mPresets.GetDelegate(); presetsDelegate != nullptr)
    {
        uint8_t afterReevaluationBuffer[kPresetHandleSize];
        MutableByteSpan afterReevaluationHandleSpan(afterReevaluationBuffer);
        auto afterReevaluationHandle = DataModel::MakeNullable(afterReevaluationHandleSpan);

        err = presetsDelegate->GetActivePresetHandle(afterReevaluationHandle);
        if (err != CHIP_NO_ERROR)
        {
            return;
        }

        if (beforeReevaluationHandle.IsNull() && afterReevaluationHandle.IsNull())
        {
            return;
        }

        if (!beforeReevaluationHandle.IsNull() && !afterReevaluationHandle.IsNull() &&
            beforeReevaluationHandle.Value().data_equal(afterReevaluationHandle.Value()))
        {
            return;
        }

        // If the active preset handle changed, notify the attribute changed.
        mCluster.NotifyAttributeChanged(ActivePresetHandle::Id);
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
