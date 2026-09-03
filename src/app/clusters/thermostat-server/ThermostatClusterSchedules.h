/**
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

#pragma once

#include "ScheduleStructWithOwnedMembers.h"
#include "ThermostatClusterAtomic.h"
#include "ThermostatClusterBase.h"
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatSchedules
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

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
         * @brief Get the Schedule at a given index in the pending schedules list.
         *
         * @param[in] index The index of the schedule in the list.
         * @param[out] schedule The ScheduleStructWithOwnedMembers struct that has the data from the pending schedule
         *             list at the given index.
         * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the pending schedules list.
         */
        virtual CHIP_ERROR GetPendingScheduleAtIndex(size_t index, ScheduleStructWithOwnedMembers & schedule) = 0;

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
         * @brief Clears the pending schedule list
         */
        virtual void ClearPendingScheduleList() = 0;

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
         * @brief Updates the schedules attribute with the content of the pending schedules list. If the schedule in the pending
         * schedules list matches i.e. has the same scheduleHandle as an existing entry in the Schedules attribute, the
         * thermostat will update the entry with the new schedule values, otherwise it will add a new schedule to the Schedules
         * attribute. For new schedules that get added, it is the responsibility of this API to allocate unique schedule
         * handles to the schedules before saving the schedule.
         * @note This will be called when the Thermostat receives a AtomicRequest command of type CommitWrite to commit the
         * pending schedule changes.
         *
         * @return CHIP_NO_ERROR if the updates to the schedules attribute has been committed successfully.
         * @return CHIP_ERROR if the updates to the schedules attribute failed to commit for some reason.
         */
        virtual CHIP_ERROR CommitPendingSchedules() = 0;

        /**
         * @brief Get the maximum timeout for atomically writing to an attribute
         *
         * @param[in] attributeId The attribute to write to.
         * @return The maximum allowed timeout; nullopt if the request is invalid.
         */
        virtual std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId) { return std::nullopt; }
    };

    ThermostatSchedules() = delete;
    ThermostatSchedules(ThermostatClusterBase & cluster, AtomicWriteSession & atomicWriteSession, Delegate & delegate) :
        mCluster(cluster), mAtomicWriteSession(atomicWriteSession), mDelegate(delegate)
    {}

    Delegate & GetDelegate() { return mDelegate; }

    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder);
    std::optional<DataModel::ActionReturnStatus> WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder);
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler);
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);

    std::optional<Protocols::InteractionModel::Status> OnAtomicWriteBegin(AttributeId attributeId);
    std::optional<Protocols::InteractionModel::Status> OnAtomicWritePrecommit(AttributeId attributeId);
    std::optional<Protocols::InteractionModel::Status> OnAtomicWriteCommit(AttributeId attributeId);
    std::optional<Protocols::InteractionModel::Status> OnAtomicWriteRollback(AttributeId attributeId);

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(AttributeId attributeId);

    /**
     * @brief Set the Active Schedule to a given schedule handle, or null
     *
     * @param scheduleHandle The handle of the schedule to set active, or null to clear the active schedule
     * @return Success if the active schedule was updated, an error code if not
     */
    Protocols::InteractionModel::Status SetActiveSchedule(DataModel::Nullable<ByteSpan> scheduleHandle);

    /**
     * @brief Apply a schedule to the pending lists of schedules during an atomic write
     *
     * @param schedule The schedule to append
     * @return CHIP_NO_ERROR if successful, an error code if not
     */
    CHIP_ERROR AppendPendingSchedule(const Structs::ScheduleStruct::DecodableType & newSchedule);

    Protocols::InteractionModel::Status PrecommitSchedules();

    bool IsScheduleHandlePresentInSchedules(const ByteSpan & scheduleHandleToMatch);

private:
    ThermostatClusterBase & mCluster;
    AtomicWriteSession & mAtomicWriteSession;
    Delegate & mDelegate;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
