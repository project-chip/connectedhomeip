/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include <app/clusters/actions-server/ActionsCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
namespace chip::app::Clusters::Actions {

/**
 * Legacy wrapper around ActionsCluster for backwards compatibility with existing applications.
 *
 * LIMITATIONS:
 *   - Each instance manages a single aggregator endpoint. While the spec permits multiple
 *     aggregator endpoints on a device (each with its own Actions cluster), this implementation
 *     supports one instance per aggregator endpoint — create one ActionsServer per aggregator.
 *   - The Delegate interface has no EndpointId parameters; it is scoped to a single endpoint
 *     instance.
 *
 * NEW CODE should use ActionsCluster directly (see ActionsCluster.h), which integrates cleanly
 * with the code-driven data model and does not carry the Ember/ZAP compatibility overhead.
 */
class ActionsServer
{
public:
    /**
     * Creates an ActionsServer for the given aggregator endpoint. Only one instance should
     * exist per node. The constructor logs an error if this constraint is violated.
     *
     * @param endpointId The aggregator endpoint on which the Actions cluster resides.
     * @param delegate   Application-supplied delegate providing action/endpoint-list data
     *                   and command handling.
     */
    ActionsServer(EndpointId endpointId, Delegate & delegate);
    ~ActionsServer();

    /**
     * Register the actions cluster instance with the codegen data model provider.
     * @return Returns an error if registration fails.
     */
    CHIP_ERROR Init();

    /**
     * Unregister the actions cluster instance from the data model provider.
     */
    void Shutdown();

    // Legacy Notifiers - these proxy directly to the new cluster
    void ActionListModified(EndpointId aEndpoint);
    void EndpointListModified(EndpointId aEndpoint);

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);

    EndpointId GetEndpointId() { return mCluster.Cluster().GetPaths()[0].mEndpointId; }

private:
    CHIP_ERROR ReadActionListAttribute(const ConcreteReadAttributePath & aPath,
                                       const AttributeValueEncoder::ListEncodeHelper & aEncoder);

    CHIP_ERROR ReadEndpointListAttribute(const ConcreteReadAttributePath & aPath,
                                         const AttributeValueEncoder::ListEncodeHelper & aEncoder);

    bool mRegistered = false;
    std::string mSetupURL;
    RegisteredServerCluster<ActionsCluster> mCluster;

    // Counts active instances for diagnostic logging. Multiple instances are valid when
    // the device has multiple aggregator endpoints (e.g. separate Zigbee and Z-Wave bridges).
    static uint8_t sInstanceCount;
};

} // namespace chip::app::Clusters::Actions
