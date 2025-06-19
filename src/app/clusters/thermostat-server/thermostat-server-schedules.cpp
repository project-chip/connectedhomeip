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
 * @brief Checks if the given schedule handle is present in the schedules attribute
 * @param[in] delegate The delegate to use.
 * @param[in] scheduleHandleToMatch The schedule handle to match with.
 *
 * @return true if the given schedule handle is present in the schedules attribute list, false otherwise.
 */
bool IsScheduleHandlePresentInSchedules(Delegate * delegate, const ByteSpan & scheduleHandleToMatch)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    ScheduleStructWithOwnedMembers matchingSchedule;
    for (uint8_t i = 0; true; i++)
    {
        CHIP_ERROR err = delegate->GetScheduleAtIndex(i, matchingSchedule);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return false;
        }

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "IsScheduleHandlePresentInSchedules: GetScheduleAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return false;
        }

        if (!matchingSchedule.GetScheduleHandle().IsNull() &&
            matchingSchedule.GetScheduleHandle().Value().data_equal(scheduleHandleToMatch))
        {
            return true;
        }
    }

    return false;
}

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

/**
 * @brief Check if a schedule is valid.
 *
 * @param[in] schedule The schedule to check.
 *
 * @return true If the schedule is valid i.e the ScheduleHandle (if not null) fits within size constraints and the SystemMode enum
 *         value is valid. Otherwise, return false.
 */
bool IsValidScheduleEntry(const ScheduleStructWithOwnedMembers & schedule)
{
    // Check that the schedule handle is not too long.
    if (!schedule.GetScheduleHandle().IsNull() && schedule.GetScheduleHandle().Value().size() > kScheduleHandleSize)
    {
        return false;
    }

    // Ensure we have a valid SystemMode.
    return (schedule.GetSystemMode() != SystemModeEnum::kUnknownEnumValue);
}

/**
 * @brief Checks if the schedule is built-in
 *
 * @param[in] schedule The schedule to check.
 *
 * @return true If the schedule is built-in, false otherwise.
 */
bool IsBuiltIn(const ScheduleStructWithOwnedMembers & schedule)
{
    return schedule.GetBuiltIn().ValueOr(false);
}

/**
 * @brief Finds and returns an entry in the Schedules attribute list that matches
 *        a schedule, if such an entry exists. The scheduleToMatch must have a schedule handle.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] scheduleHandle The schedule to match with.
 * @param[out] matchingSchedule The schedule in the Schedules attribute list that has the same ScheduleHandle as the
 * scheduleToMatch.
 *
 * @return true if a matching entry was found in the schedule attribute list, false otherwise.
 */
bool GetMatchingScheduleInSchedules(Delegate * delegate, const DataModel::Nullable<ByteSpan> & scheduleHandle,
                                    ScheduleStructWithOwnedMembers & matchingSchedule)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        CHIP_ERROR err = delegate->GetScheduleAtIndex(i, matchingSchedule);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "GetMatchingScheduleInSchedules: GetScheduleAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return false;
        }

        // Note: schedules coming from our delegate always have a handle.
        if (scheduleHandle.Value().data_equal(matchingSchedule.GetScheduleHandle().Value()))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Returns the count of schedule entries in the pending schedules list that have the matching scheduleHandle.
 * @param[in] delegate The delegate to use.
 * @param[in] scheduleHandleToMatch The schedule handle to match.
 *
 * @return count of the number of schedules found with the matching scheduleHandle. Returns 0 if no matching schedules were found.
 */
uint8_t CountSchedulesInPendingListWithScheduleHandle(Delegate * delegate, const ByteSpan & scheduleHandleToMatch)
{
    uint8_t count = 0;
    VerifyOrReturnValue(delegate != nullptr, count);

    for (uint8_t i = 0; true; i++)
    {
        ScheduleStructWithOwnedMembers schedule;
        auto err = delegate->GetPendingScheduleAtIndex(i, schedule);
        if (err != CHIP_NO_ERROR)
        {
            return count;
        }

        DataModel::Nullable<ByteSpan> scheduleHandle = schedule.GetScheduleHandle();
        if (!scheduleHandle.IsNull() && scheduleHandle.Value().data_equal(scheduleHandleToMatch))
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief Checks if the systemMode is present in the ScheduleTypes attribute.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] systemMode The systemMode to match with.
 *
 * @return true if the systemMode is found, false otherwise.
 */
bool SystemModeExistsInScheduleTypes(Delegate * delegate, SystemModeEnum systemMode)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        ScheduleTypeStruct::Type scheduleType;
        auto err = delegate->GetScheduleTypeAtIndex(i, scheduleType);
        if (err != CHIP_NO_ERROR)
        {
            return false;
        }

        if (scheduleType.systemMode == systemMode)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks if the scheduleType for the given system mode supports name in the scheduleTypeFeatures bitmap.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] systemMode The systemMode to match with.
 *
 * @return true if the scheduleType for the given system mode supports name, false otherwise.
 */
bool ScheduleTypeSupportsFeature(Delegate * delegate, SystemModeEnum systemMode, ScheduleTypeFeaturesBitmap feature)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        ScheduleTypeStruct::Type scheduleType;
        auto err = delegate->GetScheduleTypeAtIndex(i, scheduleType);
        if (err != CHIP_NO_ERROR)
        {
            return false;
        }

        if (scheduleType.systemMode == systemMode)
        {
            return (scheduleType.scheduleTypeFeatures.Has(feature));
        }
    }
    return false;
}

/**
 * @brief Returns the length of the list of schedules if the pending schedules were to be applied. The size of the pending schedules
 * list calculated, after all the constraint checks are done, is the new size of the updated Schedules attribute since the pending
 *        schedule list is expected to have all existing schedules with or without edits plus new schedules.
 *        This is called before changes are actually applied.
 *
 * @param[in] delegate The delegate to use.
 *
 * @return count of the updated Schedules attribute if the pending schedules were applied to it. Return 0 for error cases.
 */
uint8_t CountNumberOfPendingSchedules(Delegate * delegate)
{
    uint8_t numberOfPendingSchedules = 0;

    VerifyOrReturnValue(delegate != nullptr, 0);

    for (uint8_t i = 0; true; i++)
    {
        ScheduleStructWithOwnedMembers pendingSchedule;
        CHIP_ERROR err = delegate->GetPendingScheduleAtIndex(i, pendingSchedule);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "CountNumberOfPendingSchedules: GetPendingScheduleAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return 0;
        }
        numberOfPendingSchedules++;
    }

    return numberOfPendingSchedules;
}

/**
 * @brief Checks if the schedules are matching i.e the scheduleHandles are the same.
 *
 * @param[in] schedule The schedule to check.
 * @param[in] scheduleToMatch The schedule to match with.
 *
 * @return true If the schedules match, false otherwise. If both schedule handles are null, returns false
 */
bool ScheduleHandlesExistAndMatch(const ScheduleStructWithOwnedMembers & schedule,
                                  const ScheduleStructWithOwnedMembers & scheduleToMatch)
{
    return !schedule.GetScheduleHandle().IsNull() && !scheduleToMatch.GetScheduleHandle().IsNull() &&
        schedule.GetScheduleHandle().Value().data_equal(scheduleToMatch.GetScheduleHandle().Value());
}

/**
 * @brief Finds an entry in the pending schedules list that matches a schedule.
 *        The scheduleHandle of the two schedules must match.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] scheduleToMatch The schedule to match with.
 *
 * @return true if a matching entry was found in the pending schedules list, false otherwise.
 */
bool MatchingPendingScheduleExists(Delegate * delegate, const ScheduleStructWithOwnedMembers & scheduleToMatch)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        ScheduleStructWithOwnedMembers schedule;
        CHIP_ERROR err = delegate->GetPendingScheduleAtIndex(i, schedule);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "MatchingPendingScheduleExists: GetPendingScheduleAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return false;
        }

        if (ScheduleHandlesExistAndMatch(schedule, scheduleToMatch))
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

Status ThermostatAttrAccess::SetActiveSchedule(EndpointId endpoint, DataModel::Nullable<ByteSpan> scheduleHandle)
{
    auto delegate = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        return Status::InvalidInState;
    }
    if (!scheduleHandle.IsNull() && !IsScheduleHandlePresentInSchedules(delegate, scheduleHandle.Value()))
    {
        return Status::InvalidCommand;
    }

    CHIP_ERROR err = delegate->SetActiveScheduleHandle(scheduleHandle);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set ActiveScheduleHandle with error %" CHIP_ERROR_FORMAT, err.Format());
        return StatusIB(err).mStatus;
    }

    return Status::Success;
}

CHIP_ERROR ThermostatAttrAccess::AppendPendingSchedule(Thermostat::Delegate * delegate, const ScheduleStruct::Type & newSchedule)
{
    ScheduleStructWithOwnedMembers schedule = newSchedule;

    if (!IsValidScheduleEntry(schedule))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (schedule.GetScheduleHandle().IsNull())
    {
        // 1.a.i Empty schedule handle having builtIn as TRUE
        if (IsBuiltIn(schedule))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        // Force to be false, if passed as null
        schedule.SetBuiltIn(false);
    }
    else
    {
        // Per spec we need to check that:
        // 1.b.i There is an existing non-pending schedule with this handle.
        ScheduleStructWithOwnedMembers matchingSchedule;
        if (!GetMatchingScheduleInSchedules(delegate, schedule.GetScheduleHandle().Value(), matchingSchedule))
        {
            return CHIP_IM_GLOBAL_STATUS(NotFound);
        }

        // 1.b.i There is no existing pending schedule with this handle.
        if (CountSchedulesInPendingListWithScheduleHandle(delegate, schedule.GetScheduleHandle().Value()) > 0)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        const auto & scheduleBuiltIn         = schedule.GetBuiltIn();
        const auto & matchingScheduleBuiltIn = matchingSchedule.GetBuiltIn();
        // 1.b.ii & 1.b.iii The built-in fields do not have a mismatch.
        if (scheduleBuiltIn.IsNull())
        {
            if (matchingScheduleBuiltIn.IsNull())
            {
                // This really shouldn't happen; internal schedules should alway have built-in set
                return CHIP_IM_GLOBAL_STATUS(InvalidInState);
            }
            schedule.SetBuiltIn(matchingScheduleBuiltIn.Value());
        }
        else
        {
            if (matchingScheduleBuiltIn.IsNull())
            {
                // This really shouldn't happen; internal schedules should alway have built-in set
                return CHIP_IM_GLOBAL_STATUS(InvalidInState);
            }
            if (scheduleBuiltIn.Value() != matchingScheduleBuiltIn.Value())
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }
    }

    // 1.c SystemMode does not exist
    if (!SystemModeExistsInScheduleTypes(delegate, schedule.GetSystemMode()))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    // 1.d transition size exceeds the max
    if (schedule.GetTransitions().size() > delegate->GetNumberOfScheduleTransitions())
    {
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    // 1.e transition per day exceeds the max
    if (!delegate->GetNumberOfScheduleTransitionPerDay().IsNull())
    {
        for (const auto dayOfWeek : { ScheduleDayOfWeekBitmap::kSunday, ScheduleDayOfWeekBitmap::kSaturday })
        {
            uint8_t count    = 0;
            auto transitions = schedule.GetTransitions();

            for (uint8_t i = 0; i < transitions.size(); i++)
            {
                if (transitions[i].dayOfWeek.GetField(dayOfWeek))
                {
                    count++;
                }
            }

            if (count > delegate->GetNumberOfScheduleTransitionPerDay().Value())
            {
                return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
            }
        }
    }

    if (schedule.GetPresetHandle().HasValue())
    {
        // 1.f presetHandle exists but presets is not supported
        if (!ScheduleTypeSupportsFeature(delegate, schedule.GetSystemMode(), ScheduleTypeFeaturesBitmap::kSupportsPresets))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        // 1.g presetHandle exists but is not present in the presets list
        if (!IsPresetHandlePresentInPresets(delegate, schedule.GetPresetHandle().Value()))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }

    // 1.h name exists but is not supported
    if (schedule.GetName().HasValue() &&
        !ScheduleTypeSupportsFeature(delegate, schedule.GetSystemMode(), ScheduleTypeFeaturesBitmap::kSupportsNames))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    for (uint8_t i = 0; i < schedule.GetTransitions().size(); i++)
    {
        auto transition = schedule.GetTransitions()[i];

        if (transition.presetHandle.HasValue())
        {
            // 1.i.i preset not supported in transitions
            if (!ScheduleTypeSupportsFeature(delegate, schedule.GetSystemMode(), ScheduleTypeFeaturesBitmap::kSupportsPresets))
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }

            // 1.j presetHandle exists but is not present in the presets list
            if (!IsPresetHandlePresentInPresets(delegate, transition.presetHandle.Value()))
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }

        if (transition.systemMode.HasValue())
        {
            // 1.j.i SystemMode exists but is not supported
            if (!ScheduleTypeSupportsFeature(delegate, schedule.GetSystemMode(), ScheduleTypeFeaturesBitmap::kSupportsSetpoints))
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }

            // 1.j.ii SystemMode is OFF but is not supported
            if ((schedule.GetSystemMode() == SystemModeEnum::kOff) &&
                !ScheduleTypeSupportsFeature(delegate, schedule.GetSystemMode(), ScheduleTypeFeaturesBitmap::kSupportsOff))
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }

        if (!ScheduleTypeSupportsFeature(delegate, schedule.GetSystemMode(), ScheduleTypeFeaturesBitmap::kSupportsSetpoints))
        {
            // 1.k HeatingSetpoint exists but is not supported
            if (transition.heatingSetpoint.HasValue())
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }

            // 1.l CoolingSetpoint exists but is not supported
            if (transition.coolingSetpoint.HasValue())
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }
    }

    // Before adding this schedule to the pending schedules, if the expected length of the pending schedules' list
    // exceeds the total number of schedules supported, return RESOURCE_EXHAUSTED. Note that the schedule has not been appended yet.

    uint8_t numberOfPendingSchedules = CountNumberOfPendingSchedules(delegate);

    // We will be adding one more schedules, so reject if the length is already at max.
    if (numberOfPendingSchedules >= delegate->GetNumberOfSchedules())
    {
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    // TODO #34556 : Check if the number of schedules for each systemMode exceeds the max number of schedules supported for that
    // scenario. We plan to support only one schedule for each systemMode for our use cases so defer this for re-evaluation.

    return delegate->AppendToPendingScheduleList(schedule);
}

Status ThermostatAttrAccess::PrecommitSchedules(EndpointId endpoint)
{
    auto delegate = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        return Status::InvalidInState;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    // For each schedule in the schedules attribute, check that the matching schedule in the pending schedules list does not
    // violate any spec constraints.
    for (uint8_t i = 0; true; i++)
    {
        ScheduleStructWithOwnedMembers schedule;
        err = delegate->GetScheduleAtIndex(i, schedule);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl,
                         "emberAfThermostatClusterCommitSchedulesRequestCallback: GetScheduleAtIndex failed with error "
                         "%" CHIP_ERROR_FORMAT,
                         err.Format());
            return Status::InvalidInState;
        }

        bool found = MatchingPendingScheduleExists(delegate, schedule);

        // If a built in schedule in the Schedules attribute list is removed and not found in the pending schedules list, return
        // CONSTRAINT_ERROR.
        if (IsBuiltIn(schedule) && !found)
        {
            return Status::ConstraintError;
        }
    }

    // If there is an ActiveScheduleHandle set, find the schedule in the pending schedules list that matches the
    // ActiveScheduleHandle attribute. If a schedule is not found with the same scheduleHandle, return INVALID_IN_STATE. If there is
    // no ActiveScheduleHandle attribute set, continue with other checks.
    uint8_t buffer[kScheduleHandleSize];
    MutableByteSpan activeScheduleHandleSpan(buffer);
    auto activeScheduleHandle = DataModel::MakeNullable(activeScheduleHandleSpan);

    err = delegate->GetActiveScheduleHandle(activeScheduleHandle);
    if (err != CHIP_NO_ERROR)
    {
        return Status::InvalidInState;
    }

    if (!activeScheduleHandle.IsNull())
    {
        uint8_t count = CountSchedulesInPendingListWithScheduleHandle(delegate, activeScheduleHandle.Value());
        if (count == 0)
        {
            return Status::InvalidInState;
        }
    }

    return Status::Success;
}

bool emberAfThermostatClusterSetActiveScheduleRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                              const Commands::SetActiveScheduleRequest::DecodableType & commandData)
{
    auto status = gThermostatAttrAccess.SetActiveSchedule(commandPath.mEndpointId, commandData.scheduleHandle);
    commandObj->AddStatus(commandPath, status);
    return true;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfThermostatClusterSetActiveScheduleRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                              const Commands::SetActiveScheduleRequest::DecodableType & commandData)
{
    return Thermostat::emberAfThermostatClusterSetActiveScheduleRequestCallback(commandObj, commandPath, commandData);
}
