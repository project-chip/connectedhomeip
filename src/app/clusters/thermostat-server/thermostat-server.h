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

#include "thermostat-delegate.h"

#include <app-common/zap-generated/callback.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

enum class AtomicWriteState
{
    Closed = 0,
    Open,
};

static constexpr size_t kThermostatEndpointCount =
    MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

/**
 * @brief  Thermostat Attribute Access Interface.
 */
class ThermostatAttrAccess : public chip::app::AttributeAccessInterface, public chip::FabricTable::Delegate
{

public:
    ThermostatAttrAccess() : AttributeAccessInterface(Optional<chip::EndpointId>::Missing(), Thermostat::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    /**
     * @brief Set the Active Preset to a given preset handle, or null
     *
     * @param endpoint The endpoint
     * @param presetHandle The handle of the preset to set active, or null to clear the active preset
     * @return Success if the active preset was updated, an error code if not
     */
    Protocols::InteractionModel::Status SetActivePreset(EndpointId endpoint, DataModel::Nullable<ByteSpan> presetHandle);

    /**
     * @brief Apply a preset to the pending lists of presets during an atomic write
     *
     * @param delegate The current ThermostatDelegate
     * @param preset The preset to append
     * @return CHIP_NO_ERROR if successful, an error code if not
     */
    CHIP_ERROR AppendPendingPreset(Thermostat::Delegate * delegate, const Structs::PresetStruct::Type & preset);

    /**
     * @brief Verifies if the pending presets for a given endpoint are valid
     *
     * @param endpoint The endpoint
     * @return Success if the list of pending presets is valid, an error code if not
     */
    Protocols::InteractionModel::Status PrecommitPresets(EndpointId endpoint);

    /**
     * @brief Callback for when the server is removed from a given fabric; all associated atomic writes are reset
     *
     * @param fabricTable The fabric table
     * @param fabricIndex The fabric index
     */
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    /**
     * @brief Gets the scoped node id of the originator that sent the last successful
     *        AtomicRequest of type BeginWrite for the given endpoint.
     *
     * @param[in] endpoint The endpoint.
     *
     * @return the scoped node id for the given endpoint if set. Otherwise returns ScopedNodeId().
     */
    ScopedNodeId GetAtomicWriteOriginatorScopedNodeId(EndpointId endpoint);

    /**
     * @brief Sets the atomic write state for the given endpoint and originatorNodeId
     *
     * @param[in] endpoint The endpoint.
     * @param[in] originatorNodeId The originator scoped node id.
     * @param[in] state Whether or not an atomic write is open or closed.
     * @param attributeStatuses The set of attribute status structs the atomic write should be associated with
     * @return true if it was able to update the atomic write state
     * @return false if it was unable to update the atomic write state
     */
    bool
    SetAtomicWrite(EndpointId endpoint, ScopedNodeId originatorNodeId, AtomicWriteState state,
                   Platform::ScopedMemoryBufferWithSize<Globals::Structs::AtomicAttributeStatusStruct::Type> & attributeStatuses);

    /**
     * @brief Sets the atomic write state for the given endpoint and originatorNodeId
     *
     */
    /**
     * @brief Resets the atomic write for a given endpoint
     *
     * @param endpoint The endpoint
     */
    void ResetAtomicWrite(EndpointId endpoint);

    /**
     * @brief Checks if a given endpoint has an atomic write open, optionally filtered by an attribute ID
     *
     * @param endpoint The endpoint
     * @param attributeId The optional attribute ID to filter on
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool InAtomicWrite(EndpointId endpoint, Optional<AttributeId> attributeId = NullOptional);

    /**
     * @brief Checks if a given endpoint has an atomic write open for a given subject descriptor, optionally filtered by an
     * attribute ID
     *
     * @param endpoint The endpoint
     * @param subjectDescriptor The subject descriptor for the client making a read or write request
     * @param attributeId The optional attribute ID to filter on
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool InAtomicWrite(EndpointId endpoint, const Access::SubjectDescriptor & subjectDescriptor,
                       Optional<AttributeId> attributeId = NullOptional);

    /**
     * @brief Checks if a given endpoint has an atomic write open for a given command invocation, optionally filtered by an
     * attribute ID
     *
     * @param endpoint The endpoint
     * @param commandObj The CommandHandler for the invoked command
     * @param attributeId The optional attribute ID to filter on
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool InAtomicWrite(EndpointId endpoint, CommandHandler * commandObj, Optional<AttributeId> attributeId = NullOptional);

    /**
     * @brief Checks if a given endpoint has an atomic write open for a given command invocation and a list of attributes
     *
     * @param endpoint The endpoint
     * @param commandObj The CommandHandler for the invoked command
     * @param attributeStatuses The list of attribute statuses whose attributeIds must match the open atomic write
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool
    InAtomicWrite(EndpointId endpoint, CommandHandler * commandObj,
                  Platform::ScopedMemoryBufferWithSize<Globals::Structs::AtomicAttributeStatusStruct::Type> & attributeStatuses);

    /**
     * @brief Handles an AtomicRequest of type BeginWrite
     *
     * @param commandObj The AtomicRequest command handler
     * @param commandPath The path for the Atomic Request command
     * @param commandData The payload data for the Atomic Request
     */
    void BeginAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                          const Commands::AtomicRequest::DecodableType & commandData);

    /**
     * @brief Handles an AtomicRequest of type CommitWrite
     *
     * @param commandObj The AtomicRequest command handler
     * @param commandPath The path for the Atomic Request command
     * @param commandData The payload data for the Atomic Request
     */
    void CommitAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                           const Commands::AtomicRequest::DecodableType & commandData);

    /**
     * @brief Handles an AtomicRequest of type RollbackWrite
     *
     * @param commandObj The AtomicRequest command handler
     * @param commandPath The path for the Atomic Request command
     * @param commandData The payload data for the Atomic Request
     */
    void RollbackAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                             const Commands::AtomicRequest::DecodableType & commandData);

    friend void TimerExpiredCallback(System::Layer * systemLayer, void * callbackContext);

    friend void MatterThermostatClusterServerShutdownCallback(EndpointId endpoint);
    friend void MatterThermostatClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath);

    friend bool emberAfThermostatClusterSetActivePresetRequestCallback(
        CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
        const Clusters::Thermostat::Commands::SetActivePresetRequest::DecodableType & commandData);

    friend bool
    emberAfThermostatClusterAtomicRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                  const Clusters::Thermostat::Commands::AtomicRequest::DecodableType & commandData);

    struct AtomicWriteSession
    {
        AtomicWriteState state = AtomicWriteState::Closed;
        Platform::ScopedMemoryBufferWithSize<AttributeId> attributeIds;
        ScopedNodeId nodeId;
        EndpointId endpointId = kInvalidEndpointId;
    };

    AtomicWriteSession mAtomicWriteSessions[kThermostatEndpointCount];
};

/**
 * @brief Sets the default delegate for the  specific thermostat features.
 *
 * @param[in] endpoint The endpoint to set the default delegate on.
 * @param[in] delegate The default delegate.
 */
void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

Delegate * GetDelegate(EndpointId endpoint);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
