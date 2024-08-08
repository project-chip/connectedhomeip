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

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

static constexpr size_t kThermostatEndpointCount =
    MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

/**
 * @brief  Thermostat Attribute Access Interface.
 */
class ThermostatAttrAccess : public chip::app::AttributeAccessInterface
{
public:
    ThermostatAttrAccess() : AttributeAccessInterface(Optional<chip::EndpointId>::Missing(), Thermostat::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

    /**
     * @brief Sets the scoped node id of the originator that sent the last successful
     *        AtomicRequest of type BeginWrite for the given endpoint.
     *
     * @param[in] endpoint The endpoint.
     * @param[in] originatorNodeId The originator scoped node id.
     */
    void SetAtomicWriteScopedNodeId(EndpointId endpoint, ScopedNodeId originatorNodeId);

    /**
     * @brief Gets the scoped node id of the originator that sent the last successful
     *        AtomicRequest of type BeginWrite for the given endpoint.
     *
     * @param[in] endpoint The endpoint.
     *
     * @return the scoped node id for the given endpoint if set. Otherwise returns ScopedNodeId().
     */
    ScopedNodeId GetAtomicWriteScopedNodeId(EndpointId endpoint);

    /**
     * @brief Sets whether an atomic write is in progress for the given endpoint
     *
     * @param[in] endpoint The endpoint.
     * @param[in] inProgress Whether or not an atomic write is in progress.
     */
    void SetAtomicWrite(EndpointId endpoint, bool inProgress);

    /**
     * @brief Gets whether an atomic write is in progress for the given endpoint
     *
     * @param[in] endpoint The endpoint.
     *
     * @return Whether an atomic write is in progress for the given endpoint
     */
    bool InAtomicWrite(EndpointId endpoint);

    /**
     * @brief Gets whether an atomic write is in progress for the given endpoint
     *
     * @param[in] subjectDescriptor The subject descriptor.
     * @param[in] endpoint The endpoint.
     *
     * @return Whether an atomic write is in progress for the given endpoint
     */
    bool InAtomicWrite(const Access::SubjectDescriptor & subjectDescriptor, EndpointId endpoint);

    /**
     * @brief Gets whether an atomic write is in progress for the given endpoint
     *
     * @param[in] commandObj The command handler.
     * @param[in] endpoint The endpoint.
     *
     * @return Whether an atomic write is in progress for the given endpoint
     */
    bool InAtomicWrite(CommandHandler * commandObj, EndpointId endpoint);

private:
    CHIP_ERROR AppendPendingPreset(Delegate * delegate, const Structs::PresetStruct::Type & preset);

    ScopedNodeId mAtomicWriteNodeIds[kThermostatEndpointCount];
    bool mAtomicWriteState[kThermostatEndpointCount];
};

/**
 * @brief Sets the default delegate for the  specific thermostat features.
 *
 * @param[in] endpoint The endpoint to set the default delegate on.
 * @param[in] delegate The default delegate.
 */
void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
