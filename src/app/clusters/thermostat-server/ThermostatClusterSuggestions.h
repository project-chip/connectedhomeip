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

#pragma once

#include "ThermostatClusterPresets.h"
#include "ThermostatSuggestionStructWithOwnedMembers.h"
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

class ThermostatClusterCore;

class ThermostatSuggestions
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

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
         * @param[out] thermostatSuggestion The ThermostatSuggestionStructWithOwnedMembers struct that has the data from the
         * thermostat suggestion at the given index in the ThermostatSuggestions attribute list.
         * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the ThermostatSuggestions list.
         */
        virtual CHIP_ERROR GetThermostatSuggestionAtIndex(size_t index,
                                                          ThermostatSuggestionStructWithOwnedMembers & thermostatSuggestion) = 0;
        /**
         * @brief Get the CurrentThermostatSuggestion attribute value.
         *
         * @return currentThermostatSuggestion The nullable ThermostatSuggestionStruct to copy the current thermostat suggestion
         * into.
         */
        virtual void GetCurrentThermostatSuggestion(
            DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & currentThermostatSuggestion) = 0;

        /**
         * @brief Get the nullable ThermostatSuggestionNotFollowingReason attribute value.
         */
        virtual DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap> GetThermostatSuggestionNotFollowingReason() = 0;

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
         *        If the index being removed is the current thermostat suggestion, the server should set the
         * CurrentThermostatSuggestion attribute to null. This API must preserve the order of the thermostat suggestion entries that
         * are not removed. Note: The caller of this API has the responsibility to mark the relevant attributes dirty.
         *
         * @param[in] index The index of the thermostat suggestion to remove from the list.
         *
         * @return CHIP_NO_ERROR if the thermostat suggestion was removed from the list successfully.
         * @return CHIP_ERROR if the thermostat suggestion was not found in the list.
         */
        virtual CHIP_ERROR RemoveFromThermostatSuggestionsList(size_t index) = 0;

        /**
         * @brief Evaluates and sets the CurrentThermostatSuggestion attribute based on whether the thermostat has any state changes
         * (like a reboot, etc) or a thermostat suggestion was added or removed. Sets the CurrentThermostatSuggestion attribute to
         * null if the server wasn't able to determine a current suggestion, sets the ThermostatSuggestionNotFollowingReason
         * accordingly. This API should be responsible for keeping track of ExpirationTime for the current thermostat suggestion and
         * re-evaluating the next current suggestion when the current suggestion expires. The caller of this API must ensure that
         * they remove all expired suggestions prior to calling this.
         *
         * @return CHIP_NO_ERROR if a current thermostat suggestion was evaluated successfully.
         * @return CHIP_ERROR if there was an error evaluating the current thermostat suggestion.
         */
        virtual CHIP_ERROR ReEvaluateCurrentSuggestion() = 0;
    };

    ThermostatSuggestions() = delete;
    ThermostatSuggestions(ThermostatClusterCore & cluster, ThermostatPresets & presets, Delegate & delegate) :
        mCluster(cluster), mDelegate(delegate), mPresets(presets)
    {}

    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder);
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler,
                                                               bool & handled);

    std::optional<DataModel::ActionReturnStatus>
    AddThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                            const Commands::AddThermostatSuggestion::DecodableType & commandData, bool & handled);

    std::optional<DataModel::ActionReturnStatus>
    RemoveThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                               const Commands::RemoveThermostatSuggestion::DecodableType & commandData);

    void ReEvaluateCurrentSuggestion();

private:
    ThermostatClusterCore & mCluster;
    Delegate & mDelegate;
    ThermostatPresets & mPresets;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
