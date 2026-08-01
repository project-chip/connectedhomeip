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
#include <app/data-model-provider/OperationTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatCluster;

class ThermostatSuggestions
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        virtual uint8_t GetMaxThermostatSuggestions()                                                             = 0;
        virtual uint8_t GetNumberOfThermostatSuggestions()                                                        = 0;
        virtual CHIP_ERROR GetThermostatSuggestionAtIndex(size_t index,
                                                           ThermostatSuggestionStructWithOwnedMembers & thermostatSuggestion) = 0;
        virtual void GetCurrentThermostatSuggestion(
            DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> & currentThermostatSuggestion) = 0;
        virtual DataModel::Nullable<ThermostatSuggestionNotFollowingReasonBitmap>
        GetThermostatSuggestionNotFollowingReason()                                                                = 0;
        virtual CHIP_ERROR GetUniqueID(uint8_t & uniqueID)                                                        = 0;
        virtual CHIP_ERROR
        AppendToThermostatSuggestionsList(const Structs::ThermostatSuggestionStruct::Type & thermostatSuggestion) = 0;
        virtual CHIP_ERROR RemoveFromThermostatSuggestionsList(size_t index)                                      = 0;
        virtual CHIP_ERROR ReEvaluateCurrentSuggestion()                                                          = 0;
    };

    ThermostatSuggestions() = default;
    ThermostatSuggestions(ThermostatCluster & cluster) : mCluster(&cluster) {}

    void SetCluster(ThermostatCluster & cluster) { mCluster = &cluster; }

    void SetDelegate(Delegate * delegate) { mDelegate = delegate; }
    Delegate * GetDelegate() const { return mDelegate; }

    void SetPresets(ThermostatPresets * presets) { mPresets = presets; }

    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder);
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler, bool & handled);

    std::optional<DataModel::ActionReturnStatus> AddThermostatSuggestion(
        CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
        const Commands::AddThermostatSuggestion::DecodableType & commandData, bool & handled);

    std::optional<DataModel::ActionReturnStatus> RemoveThermostatSuggestion(
        CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
        const Commands::RemoveThermostatSuggestion::DecodableType & commandData);

    void ReEvaluateCurrentSuggestion();

private:
    ThermostatCluster * mCluster = nullptr;
    Delegate * mDelegate             = nullptr;
    ThermostatPresets * mPresets     = nullptr;
};


} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
