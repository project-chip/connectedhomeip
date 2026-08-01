/**
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ThermostatClusterSchedules.h"
#include "ScheduleStructWithOwnedMembers.h"
#include "ThermostatCluster.h"
#include "ThermostatClusterEvents.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::Protocols::InteractionModel;

namespace {

/**
 * @brief Check if a schedule is valid.
 *
 * @param[in] schedule The schedule to check.
 *
 * @return true If the schedule is valid i.e the ScheduleHandle (if not null) fits within size constraints and the systemMode
 *         enum value is valid. Otherwise, return false.
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

/**
 * @brief Finds and returns an entry in the Schedules attribute list that matches
 *        a schedule, if such an entry exists. The scheduleHandle must not be null.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] scheduleHandle The schedule handle to match with.
 * @param[out] matchingSchedule The schedule in the Schedules attribute list that has the same ScheduleHandle as scheduleHandle.
 *
 * @return true if a matching entry was found in the schedules attribute list, false otherwise.
 */
bool GetMatchingScheduleInSchedules(Delegate * delegate, const ByteSpan & scheduleHandle,
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

        if (!matchingSchedule.GetScheduleHandle().IsNull() &&
            scheduleHandle.data_equal(matchingSchedule.GetScheduleHandle().Value()))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Gets the maximum number of schedules allowed for a given system mode.
 *
 * @param[in]  delegate The delegate to use.
 * @param[in]  systemMode The systemMode to match with.
 * @param[out] count The maximum number of schedules for the specified systemMode.
 * @return CHIP_NO_ERROR if the maximum number was determined, or an error if not
 */
CHIP_ERROR MaximumScheduleTypeCount(Delegate * delegate, SystemModeEnum systemMode, size_t & count)
{
    count = 0;
    for (uint8_t i = 0; true; i++)
    {
        ScheduleTypeStruct::Type scheduleType;
        auto err = delegate->GetScheduleTypeAtIndex(i, scheduleType);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            // We exhausted the list trying to find the system mode
            return CHIP_NO_ERROR;
        }
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        if (scheduleType.systemMode == systemMode)
        {
            count = scheduleType.numberOfSchedules;
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_NO_ERROR;
}

/**
 * @brief Returns the count of schedule entries in the pending schedules list that have the matching scheduleHandle.
 * @param[in] delegate The delegate to use.
 * @param[in] scheduleHandleToMatch The schedule handle to match.
 *
 * @return count of the number of schedules found with the matching scheduleHandle. Returns 0 if no matching schedules were
 * found.
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
 * @brief Checks if the scheduleType for the given system mode supports name in the scheduleTypeFeatures bitmap.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] systemMode The systemMode to match with.
 *
 * @return true if the scheduleType for the given system mode supports name, false otherwise.
 */
bool ScheduleTypeSupportsNames(Delegate * delegate, SystemModeEnum systemMode)
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
            return (scheduleType.scheduleTypeFeatures.Has(ScheduleTypeFeaturesBitmap::kSupportsNames));
        }
    }
    return false;
}

/**
 * @brief Checks that a schedule does not have more transitions on any single day than the NumberOfScheduleTransitionPerDay
 *        attribute allows.
 *
 * @param[in] delegate The delegate to use.
 * @param[in] schedule The schedule to check.
 *
 * @return CHIP_NO_ERROR if the schedule does not violate the per-day transition limit, or an IM status error if it does.
 */
CHIP_ERROR ValidateTransitionsPerDay(Delegate * delegate, const ScheduleStructWithOwnedMembers & schedule)
{
    auto perDayLimit = delegate->GetNumberOfScheduleTransitionsPerDay();
    if (perDayLimit.IsNull())
    {
        // No per-day limit is configured.
        return CHIP_NO_ERROR;
    }

    static constexpr ScheduleDayOfWeekBitmap kDays[] = {
        ScheduleDayOfWeekBitmap::kSunday, ScheduleDayOfWeekBitmap::kMonday,   ScheduleDayOfWeekBitmap::kTuesday,
        ScheduleDayOfWeekBitmap::kWednesday, ScheduleDayOfWeekBitmap::kThursday, ScheduleDayOfWeekBitmap::kFriday,
        ScheduleDayOfWeekBitmap::kSaturday, ScheduleDayOfWeekBitmap::kAway,
    };

    for (auto day : kDays)
    {
        size_t count = 0;
        for (const auto & transition : schedule.GetTransitions())
        {
            if (transition.dayOfWeek.Has(day))
            {
                count++;
            }
        }
        if (count > perDayLimit.Value())
        {
            return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
        }
    }
    return CHIP_NO_ERROR;
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

extern ThermostatAttrAccess gThermostatAttrAccess;

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

Status ThermostatAttrAccess::SetActiveSchedule(EndpointId endpoint, DataModel::Nullable<ByteSpan> scheduleHandle)
{
    auto delegate = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        return Status::InvalidInState;
    }

    // If the schedule handle passed in the command is not present in the Schedules attribute, return INVALID_COMMAND.
    if (!scheduleHandle.IsNull() && !IsScheduleHandlePresentInSchedules(delegate, scheduleHandle.Value()))
    {
        return Status::InvalidCommand;
    }

    uint8_t previousBuffer[kScheduleHandleSize];
    MutableByteSpan previousScheduleHandleSpan(previousBuffer);
    auto previousScheduleHandleBuffer = DataModel::MakeNullable(previousScheduleHandleSpan);
    bool havePreviousScheduleHandle   = (delegate->GetActiveScheduleHandle(previousScheduleHandleBuffer) == CHIP_NO_ERROR);

    Optional<DataModel::Nullable<ByteSpan>> previousScheduleHandle;
    if (havePreviousScheduleHandle)
    {
        previousScheduleHandle = previousScheduleHandleBuffer.IsNull()
            ? MakeOptional(DataModel::Nullable<ByteSpan>())
            : MakeOptional(DataModel::MakeNullable(ByteSpan(previousScheduleHandleBuffer.Value())));
    }

    CHIP_ERROR err = delegate->SetActiveScheduleHandle(scheduleHandle);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set ActiveScheduleHandle with error %" CHIP_ERROR_FORMAT, err.Format());
        return StatusIB(err).mStatus;
    }

    GenerateActiveScheduleChangeEvent(endpoint, previousScheduleHandle, scheduleHandle);

    return Status::Success;
}

CHIP_ERROR ThermostatAttrAccess::AppendPendingSchedule(Thermostat::Delegate * delegate,
                                                       const ScheduleStruct::DecodableType & newSchedule)
{
    // Count the incoming transitions before making an owned copy, since our owned storage has a fixed capacity.
    size_t incomingTransitionsCount = 0;
    {
        auto transitionsIter = newSchedule.transitions.begin();
        while (transitionsIter.Next())
        {
            incomingTransitionsCount++;
        }
        ReturnErrorOnFailure(transitionsIter.GetStatus());
        if (incomingTransitionsCount > kScheduleTransitionsMax)
        {
            ChipLogError(Zcl, "Schedule transitions count %u exceeds max supported %u",
                         static_cast<unsigned>(incomingTransitionsCount), static_cast<unsigned>(kScheduleTransitionsMax));
            return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
        }
    }

    ScheduleStructWithOwnedMembers schedule = newSchedule;
    if (!IsValidScheduleEntry(schedule))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (schedule.GetScheduleHandle().IsNull())
    {
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
        // (a) There is an existing non-pending schedule with this handle.
        ScheduleStructWithOwnedMembers matchingSchedule;
        if (!GetMatchingScheduleInSchedules(delegate, schedule.GetScheduleHandle().Value(), matchingSchedule))
        {
            return CHIP_IM_GLOBAL_STATUS(NotFound);
        }

        // (b) There is no existing pending schedule with this handle.
        if (CountSchedulesInPendingListWithScheduleHandle(delegate, schedule.GetScheduleHandle().Value()) > 0)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        const auto & scheduleBuiltIn         = schedule.GetBuiltIn();
        const auto & matchingScheduleBuiltIn = matchingSchedule.GetBuiltIn();
        // (c)/(d) The built-in fields do not have a mismatch.
        if (scheduleBuiltIn.IsNull())
        {
            if (matchingScheduleBuiltIn.IsNull())
            {
                // This really shouldn't happen; internal schedules should always have built-in set
                return CHIP_IM_GLOBAL_STATUS(InvalidInState);
            }
            schedule.SetBuiltIn(matchingScheduleBuiltIn.Value());
        }
        else
        {
            if (matchingScheduleBuiltIn.IsNull())
            {
                // This really shouldn't happen; internal schedules should always have built-in set
                return CHIP_IM_GLOBAL_STATUS(InvalidInState);
            }
            if (scheduleBuiltIn.Value() != matchingScheduleBuiltIn.Value())
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }
    }

    size_t maximumScheduleCount     = delegate->GetNumberOfSchedules();
    size_t maximumScheduleTypeCount = 0;
    if (MaximumScheduleTypeCount(delegate, schedule.GetSystemMode(), maximumScheduleTypeCount) != CHIP_NO_ERROR)
    {
        return CHIP_IM_GLOBAL_STATUS(InvalidInState);
    }

    if (maximumScheduleTypeCount == 0)
    {
        // This is not a supported system mode for schedules
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (schedule.GetName().HasValue() && !ScheduleTypeSupportsNames(delegate, schedule.GetSystemMode()))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    ReturnErrorOnFailure(ValidateTransitionsPerDay(delegate, schedule));

    // Before adding this schedule to the pending schedules, if the expected length of the pending schedules' list
    // exceeds the total number of schedules/transitions supported, return RESOURCE_EXHAUSTED. Note that the schedule has not
    // been appended yet.

    // We're going to append this schedule, so let's assume a count as though it had already been inserted
    size_t scheduleCount         = 1;
    size_t scheduleTypeCount     = 1;
    size_t totalTransitionsCount = schedule.GetTransitions().size();
    for (uint8_t i = 0; true; i++)
    {
        ScheduleStructWithOwnedMembers otherSchedule;
        CHIP_ERROR err = delegate->GetPendingScheduleAtIndex(i, otherSchedule);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            return CHIP_IM_GLOBAL_STATUS(InvalidInState);
        }
        scheduleCount++;
        totalTransitionsCount += otherSchedule.GetTransitions().size();
        if (schedule.GetSystemMode() == otherSchedule.GetSystemMode())
        {
            scheduleTypeCount++;
        }
    }

    if (scheduleCount > maximumScheduleCount)
    {
        ChipLogError(Zcl, "Schedule count exceeded %u: %u ", static_cast<unsigned>(maximumScheduleCount),
                     static_cast<unsigned>(scheduleCount));
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    if (scheduleTypeCount > maximumScheduleTypeCount)
    {
        ChipLogError(Zcl, "Schedule type count exceeded %u: %u ", static_cast<unsigned>(maximumScheduleTypeCount),
                     static_cast<unsigned>(scheduleTypeCount));
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    size_t maximumTransitionsCount = delegate->GetNumberOfScheduleTransitions();
    if (totalTransitionsCount > maximumTransitionsCount)
    {
        ChipLogError(Zcl, "Schedule transitions count exceeded %u: %u ", static_cast<unsigned>(maximumTransitionsCount),
                     static_cast<unsigned>(totalTransitionsCount));
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

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
                         "PrecommitSchedules: GetScheduleAtIndex failed with error "
                         "%" CHIP_ERROR_FORMAT,
                         err.Format());
            return Status::InvalidInState;
        }

        bool found = MatchingPendingScheduleExists(delegate, schedule);

        // If a built in schedule in the Schedules attribute list is removed and not found in the pending schedules list,
        // return CONSTRAINT_ERROR.
        if (IsBuiltIn(schedule) && !found)
        {
            return Status::ConstraintError;
        }
    }

    // If there is an ActiveScheduleHandle set, find the schedule in the pending schedules list that matches the
    // ActiveScheduleHandle attribute. If a schedule is not found with the same scheduleHandle, return INVALID_IN_STATE. If
    // there is no ActiveScheduleHandle attribute set, continue with other checks.
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

bool emberAfThermostatClusterSetActiveScheduleRequestCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::SetActiveScheduleRequest::DecodableType & commandData)
{
    auto status = gThermostatAttrAccess.SetActiveSchedule(commandPath.mEndpointId, DataModel::MakeNullable(commandData.scheduleHandle));
    commandObj->AddStatus(commandPath, status);
    return true;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfThermostatClusterSetActiveScheduleRequestCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::SetActiveScheduleRequest::DecodableType & commandData)
{
    return Thermostat::emberAfThermostatClusterSetActiveScheduleRequestCallback(commandObj, commandPath, commandData);
}
