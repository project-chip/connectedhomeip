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

/****************************************************************************
 * @file
 * @brief APIs for the  Thermostat cluster.
 *
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "ThermostatClusterAtomic.h"
#include "ThermostatClusterSetpoints.h"
#include "ThermostatDelegate.h"

#include <app-common/zap-generated/callback.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatCluster : public DefaultServerCluster, chip::FabricTable::Delegate, AtomicWriteSession::Delegate
{

public:
    ThermostatCluster(EndpointId aEndpointId, BitFlags<Thermostat::Feature> features);

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    EndpointId Endpoint() { return mPath.mEndpointId; }
    void SetDelegate(Thermostat::Delegate * delegate) { mDelegate = delegate; }

    Protocols::InteractionModel::Status OnAtomicWriteBegin(AttributeId attributeId) override;
    Protocols::InteractionModel::Status OnAtomicWritePrecommit(AttributeId attributeId) override;
    Protocols::InteractionModel::Status OnAtomicWriteCommit(AttributeId attributeId) override;
    Protocols::InteractionModel::Status OnAtomicWriteRollback(AttributeId attributeId) override;

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) override;

    void OnAtomicWriteTimeout();

private:
    const BitFlags<Thermostat::Feature> mFeatures;

    Thermostat::Delegate * mDelegate;

    Setpoints mSetpoints;
    AtomicWriteSession mAtomicWriteSession;

    BitMask<RemoteSensingBitmap> mRemoteSensing;
    BitMask<OccupancyBitmap> mOccupancy;

    SystemModeEnum mSystemMode;
    ControlSequenceOfOperationEnum mControlSequenceOfOperation;

    DataModel::ActionReturnStatus WriteHeatingSetpoint(bool occupied, AttributeId heatingAttributeId,
                                                       AttributeId coolingAttributeId, AttributeValueDecoder & decoder);

    DataModel::ActionReturnStatus WriteCoolingSetpoint(bool occupied, AttributeId heatingAttributeId,
                                                       AttributeId coolingAttributeId, AttributeValueDecoder & decoder);

    DataModel::ActionReturnStatus WriteNonAtomicAttribute(const DataModel::WriteAttributeRequest & request,
                                                          AttributeValueDecoder & decoder);

    /**
     * @brief Set the Active Preset to a given preset handle, or null
     *
     * @param endpoint The endpoint
     * @param presetHandle The handle of the preset to set active, or null to clear the active preset
     * @return Success if the active preset was updated, an error code if not
     */
    Protocols::InteractionModel::Status SetActivePreset(DataModel::Nullable<ByteSpan> presetHandle);

    /**
     * @brief Apply a preset to the pending lists of presets during an atomic write
     *
     * @param delegate The current ThermostatDelegate
     * @param preset The preset to append
     * @return CHIP_NO_ERROR if successful, an error code if not
     */
    CHIP_ERROR AppendPendingPreset(const Structs::PresetStruct::Type & preset);

    chip::Protocols::InteractionModel::Status PrecommitPresets();

    DataModel::ActionReturnStatus WriteSetpointAttribute(const DataModel::WriteAttributeRequest & request,
                                                         AttributeValueDecoder & decoder);

    DataModel::ActionReturnStatus WriteSetpointLimitAttribute(const DataModel::WriteAttributeRequest & request,
                                                              AttributeValueDecoder & decoder);

    std::optional<DataModel::ActionReturnStatus>
    AddThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                            const Commands::AddThermostatSuggestion::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    RemoveThermostatSuggestion(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                               const Commands::RemoveThermostatSuggestion::DecodableType & commandData);
};

ThermostatCluster * ClusterForEndpoint(EndpointId endpointId);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
