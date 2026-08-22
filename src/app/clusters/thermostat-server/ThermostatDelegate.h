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

#pragma once

#include "PresetStructWithOwnedMembers.h"
#include "ScheduleStructWithOwnedMembers.h"
#include "ThermostatSuggestionStructWithOwnedMembers.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/** @brief
 *  Defines methods for implementing application-specific logic for handling Presets in the thermostat cluster.
 *  It defines the interfaces that a thermostat should implement to enable support for reading and writing the
 *  Presets attribute and reading and writing the ActivePresetHandle attribute.
 */
class Delegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     * @brief Get the maximum timeout for atomically writing to an attribute
     *
     * @param[in] attributeId The attribute to write to.
     * @return The maximum allowed timeout; nullopt if the request is invalid.
     */
    virtual std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) = 0;

    /**
     * @brief Get the preset type at a given index in the PresetTypes attribute
     *
     * @param[in] index The index of the preset type in the list.
     * @param[out] presetType The preset type  at the given index in the list.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the preset types list.
     */
    virtual CHIP_ERROR GetPresetTypeAtIndex(size_t index, Structs::PresetTypeStruct::Type & presetType) = 0;

    /**
     * @brief Get the NumberOfPresets attribute value.
     *
     * @return The max number of  presets supported. Return 0 if not set.
     */
    virtual uint8_t GetNumberOfPresets() = 0;

    /**
     * @brief Get the preset at a given index in the Presets attribute.
     *
     * @param[in] index The index of the preset in the list.
     * @param[out] preset The PresetStructWithOwnedMembers struct that has the data from the preset
     *             at the given index in the Presets attribute list.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the presets list.
     */
    virtual CHIP_ERROR GetPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) = 0;

    /**
     * @brief Get the ActivePresetHandle attribute value.
     *
     * @param[out] activePresetHandle The nullable MutableByteSpan to copy the active preset handle into. On success,
     *             the size of the activePresetHandle is updated to the length of the copied data.
     */
    virtual CHIP_ERROR GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & activePresetHandle) = 0;

    /**
     * @brief Set the ActivePresetHandle attribute value.
     *
     * @param[in] newActivePresetHandle The octet string to set the active preset handle to.
     */
    virtual CHIP_ERROR SetActivePresetHandle(const DataModel::Nullable<ByteSpan> & newActivePresetHandle) = 0;

    /**
     * @brief Copies existing presets to the pending preset list
     */
    virtual void InitializePendingPresets() = 0;

    /**
     * @brief Appends a preset to the pending presets list maintained by the delegate.
     *        The delegate must ensure it makes a copy of the provided preset and the data
     *        of its preset handle, if any.  For example, it could create a PresetStructWithOwnedMembers
     *        from the provided preset.
     *
     * @param[in] preset The preset to add to the list.
     *
     * @return CHIP_NO_ERROR if the preset was appended to the list successfully.
     * @return CHIP_ERROR if there was an error adding the preset to the list.
     */
    virtual CHIP_ERROR AppendToPendingPresetList(const PresetStructWithOwnedMembers & preset) = 0;

    /**
     * @brief Get the Preset at a given index in the pending presets list.
     *
     * @param[in] index The index of the preset in the list.
     * @param[out] preset The PresetStructWithOwnedMembers struct that has the data from the pending preset
     *             list at the given index.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the pending presets list.
     */
    virtual CHIP_ERROR GetPendingPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) = 0;

    /**
     * @brief Updates the presets attribute with the content of the pending presets list. If the preset in the pending presets list
     * matches i.e. has the same presetHandle as an existing entry in the Presets attribute, the thermostat will update the entry
     * with the new preset values, otherwise it will add a new preset to the Presets attribute. For new presets that get added,
     * it is the responsibility of this API to allocate unique preset handles to the presets before saving the preset. This will be
     * called when the Thermostat receives a AtomicRequest command of type CommitWrite to commit the pending preset changes.
     *
     * @return CHIP_NO_ERROR if the updates to the presets attribute has been committed successfully.
     * @return CHIP_ERROR if the updates to the presets attribute failed to commit for some reason.
     *
     */
    virtual CHIP_ERROR CommitPendingPresets() = 0;

    /**
     * @brief Clears the pending presets list.
     *
     */
    virtual void ClearPendingPresetList() = 0;

    /**
     * @brief Get the MaxThermostatSuggestions attribute value.
     *
     * @return The max number of thermostat suggestions supported. Return 0 if not set.
     */
    virtual uint8_t GetMaxThermostatSuggestions() = 0;

    /**
     * @brief Get the number of suggestions in the ThermostatSuggestions attribute list.
     *
     * @return The number of entries in the ThermostatSuggestions attribute list. Return 0 if not set.
     */
    virtual uint8_t GetNumberOfThermostatSuggestions() = 0;

    /**
     * @brief Get the ThermostatSuggestion at a given index in the ThermostatSuggestions attribute.
     *
     * @param[in] index The index of the suggestion in the list.
     * @param[out] thermostatSuggestion The ThermostatSuggestionStructWithOwnedMembers struct that has the data from the thermostat
     * suggestion at the given index in the ThermostatSuggestions attribute list.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the ThermostatSuggestions list.
     */
    virtual CHIP_ERROR GetThermostatSuggestionAtIndex(size_t index,
                                                      ThermostatSuggestionStructWithOwnedMembers & thermostatSuggestion) = 0;

    /**
     * @brief Get the CurrentThermostatSuggestion attribute value.
     *
     * @return currentThermostatSuggestion The nullable ThermostatSuggestionStruct to copy the current thermostat suggestion into.
     */
    virtual void GetCurrentThermostatSuggestion(
        DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & currentThermostatSuggestion) = 0;

    /**
     * @brief Get the nullable ThermostatSuggestionNotFollowingReason attribute value.
     *
     */
    virtual DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> GetThermostatSuggestionNotFollowingReason() = 0;

    /**
     * @brief Appends a suggestion to the ThermostatSuggestions attribute list maintained by the delegate.
     *        The delegate must ensure it makes a copy of the provided thermostat suggestion and the data
     *        of its preset handle.  For example, it could create a ThermostatSuggestionStructWithOwnedMembers
     *        from the provided thermostat suggestion.
     *        Note: The caller of this API has the responsibility to mark the relevant attributes dirty.
     *
     * @param[in] thermostatSuggestion The thermostat suggestion to add to the list.
     *
     * @return CHIP_NO_ERROR if the thermostat suggestion was appended to the list successfully.
     * @return CHIP_ERROR if there was an error adding the thermostat suggestion to the list.
     */
    virtual CHIP_ERROR
    AppendToThermostatSuggestionsList(const Structs::ThermostatSuggestionStruct::Type & thermostatSuggestion) = 0;

    /**
     * @brief Removes a suggestion from the ThermostatSuggestions attribute list maintained by the delegate.
     *        If the index being removed is the current thermostat suggestion, the server should set the CurrentThermostatSuggestion
     *        attribute to null. This API must preserve the order of the thermostat suggestion entries that are not removed.
     *        Note: The caller of this API has the responsibility to mark the relevant attributes dirty.
     *
     * @param[in] indexToRemove The index of the thermostat suggestion to remove from the list.
     *
     * @return CHIP_NO_ERROR if the thermostat suggestion was removed from the list successfully.
     * @return CHIP_ERROR if the thermostat suggestion was not found in the list.
     */
    virtual CHIP_ERROR RemoveFromThermostatSuggestionsList(size_t indexToRemove) = 0;

    /**
     * @brief Returns an unused unique ID for a thermostat suggestion.
     *
     * @param[out] an unique ID starting from 0 to UINT8_MAX.
     *
     * @return CHIP_NO_ERROR if a unique ID was found
     *         CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if no uniqueID was found.
     */
    virtual CHIP_ERROR GetUniqueID(uint8_t & uniqueID) = 0;

    /**
     * @brief Evaluates and sets the CurrentThermostatSuggestion attribute based on whether the thermostat has any state changes
     * (like a reboot, etc) or a thermostat suggestion was added or removed. Sets the CurrentThermostatSuggestion attribute to null
     * if the server wasn't able to determine a current suggestion, sets the ThermostatSuggestionNotFollowingReason accordingly.
     * This API should be responsible for keeping track of ExpirationTime for the current thermostat suggestion and re-evaluating
     * the next current suggestion when the current suggestion expires.
     * The caller of this API must ensure that they remove all expired suggestions prior to calling this.
     *
     * @return CHIP_NO_ERROR if a current thermostat suggestion was evaluated successfully.
     * @return CHIP_ERROR if there was an error evaluating the current thermostat suggestion.
     */
    virtual CHIP_ERROR ReEvaluateCurrentSuggestion() = 0;

    /**
     * @brief Get the schedule type at a given index in the ScheduleTypes attribute
     *
     * @param[in] index The index of the schedule type in the list.
     * @param[out] scheduleType The schedule type at the given index in the list.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the schedule types list.
     */
    virtual CHIP_ERROR GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & scheduleType) = 0;

    /**
     * @brief Get the NumberOfSchedules attribute value.
     *
     * @return The max number of schedules supported. Return 0 if not set.
     */
    virtual uint8_t GetNumberOfSchedules() = 0;

    /**
     * @brief Get the NumberOfScheduleTransitions attribute value.
     *
     * @return The max number of schedule transitions supported across all schedules. Return 0 if not set.
     */
    virtual uint8_t GetNumberOfScheduleTransitions() = 0;

    /**
     * @brief Get the nullable NumberOfScheduleTransitionPerDay attribute value.
     *
     * @return The max number of schedule transitions supported per day, or null if there is no such limit.
     */
    virtual DataModel::Nullable<uint8_t> GetNumberOfScheduleTransitionsPerDay() = 0;

    /**
     * @brief Get the schedule at a given index in the Schedules attribute.
     *
     * @param[in] index The index of the schedule in the list.
     * @param[out] schedule The ScheduleStructWithOwnedMembers struct that has the data from the schedule
     *             at the given index in the Schedules attribute list.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the schedules list.
     */
    virtual CHIP_ERROR GetScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) = 0;

    /**
     * @brief Get the ActiveScheduleHandle attribute value.
     *
     * @param[out] activeScheduleHandle The nullable MutableByteSpan to copy the active schedule handle into. On success,
     *             the size of the activeScheduleHandle is updated to the length of the copied data.
     */
    virtual CHIP_ERROR GetActiveScheduleHandle(DataModel::Nullable<MutableByteSpan> & activeScheduleHandle) = 0;

    /**
     * @brief Set the ActiveScheduleHandle attribute value.
     *
     * @param[in] newActiveScheduleHandle The octet string to set the active schedule handle to.
     */
    virtual CHIP_ERROR SetActiveScheduleHandle(const DataModel::Nullable<ByteSpan> & newActiveScheduleHandle) = 0;

    /**
     * @brief Copies existing schedules to the pending schedule list
     */
    virtual void InitializePendingSchedules() = 0;

    /**
     * @brief Appends a schedule to the pending schedules list maintained by the delegate.
     *        The delegate must ensure it makes a copy of the provided schedule and the data
     *        of its schedule handle, if any. For example, it could create a ScheduleStructWithOwnedMembers
     *        from the provided schedule.
     *
     * @param[in] schedule The schedule to add to the list.
     *
     * @return CHIP_NO_ERROR if the schedule was appended to the list successfully.
     * @return CHIP_ERROR if there was an error adding the schedule to the list.
     */
    virtual CHIP_ERROR AppendToPendingScheduleList(const ScheduleStructWithOwnedMembers & schedule) = 0;

    /**
     * @brief Get the Schedule at a given index in the pending schedules list.
     *
     * @param[in] index The index of the schedule in the list.
     * @param[out] schedule The ScheduleStructWithOwnedMembers struct that has the data from the pending schedule
     *             list at the given index.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the pending schedules list.
     */
    virtual CHIP_ERROR GetPendingScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) = 0;

    /**
     * @brief Updates the schedules attribute with the content of the pending schedules list. If the schedule in the pending
     * schedules list matches i.e. has the same scheduleHandle as an existing entry in the Schedules attribute, the thermostat
     * will update the entry with the new schedule values, otherwise it will add a new schedule to the Schedules attribute. For
     * new schedules that get added, it is the responsibility of this API to allocate unique schedule handles to the schedules
     * before saving the schedule. This will be called when the Thermostat receives a AtomicRequest command of type CommitWrite
     * to commit the pending schedule changes.
     *
     * @return CHIP_NO_ERROR if the updates to the schedules attribute has been committed successfully.
     * @return CHIP_ERROR if the updates to the schedules attribute failed to commit for some reason.
     */
    virtual CHIP_ERROR CommitPendingSchedules() = 0;

    /**
     * @brief Clears the pending schedules list.
     */
    virtual void ClearPendingScheduleList() = 0;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    // This should be removed once #39949 is fixed.
protected:
    EndpointId mEndpointId = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
