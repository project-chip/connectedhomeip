/*
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/power-topology-server/PowerTopologyDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/PowerTopology/AttributeIds.h>
#include <clusters/PowerTopology/Attributes.h>
#include <clusters/PowerTopology/ClusterId.h>
#include <clusters/PowerTopology/Enums.h>
#include <clusters/PowerTopology/Structs.h>
#include <credentials/FabricTable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

#include <cstddef>

namespace chip::app::Clusters::PowerTopology {

class PowerTopologyCluster : public DefaultServerCluster, private FabricTable::Delegate
{

public:
    // Spec constraints for the ElectricalCircuitNodes attribute (CIRC feature).
    static constexpr size_t kMaxCircuitNodes    = 50;  // list max
    static constexpr size_t kMaxNodeLabelLength = 128; // Label constraint

    struct Config
    {
        EndpointId endpointId;
        // Fully qualified: private inheritance of FabricTable::Delegate injects the name `Delegate`
        // into this class scope, which would otherwise shadow PowerTopology::Delegate here.
        PowerTopology::Delegate & delegate;
        BitMask<Feature> features;
        // Optional: required only for the CIRC feature so the cluster can purge a fabric's
        // ElectricalCircuitNodes entries on fabric removal. When null (e.g. no CIRC, or unit tests
        // that do not exercise fabric removal) the cluster does not register a fabric delegate.
        FabricTable * fabricTable = nullptr;
    };

    PowerTopologyCluster(const Config & config) :
        DefaultServerCluster({ config.endpointId, PowerTopology::Id }), mDelegate(config.delegate), mFeatureFlags(config.features),
        mFabricTable(config.fabricTable)
    {}

    const BitFlags<PowerTopology::Feature> & Features() const { return mFeatureFlags; }

    // Getters - return copies with error checking
    CHIP_ERROR GetAvailableEndpoints(AttributeValueEncoder & aEncoder) const;
    CHIP_ERROR GetActiveEndpoints(AttributeValueEncoder & aEncoder) const;
    CHIP_ERROR GetElectricalCircuitNodes(AttributeValueEncoder & aEncoder) const;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

private:
    // In-cluster storage for a single ElectricalCircuitNodes entry. The generated CircuitNodeStruct
    // carries a CharSpan Label; we own the backing bytes here so the span stays valid across encoding.
    struct StoredCircuitNode
    {
        NodeId node = kUndefinedNodeId;
        Optional<EndpointId> endpoint;
        bool hasLabel = false;
        char label[kMaxNodeLabelLength];
        size_t labelLength      = 0;
        FabricIndex fabricIndex = kUndefinedFabricIndex;

        Structs::CircuitNodeStruct::Type AsStruct() const;
    };

    // ElectricalCircuitNodes (CIRC): fabric-scoped, writable, non-volatile list.
    DataModel::ActionReturnStatus WriteElectricalCircuitNodes(const DataModel::WriteAttributeRequest & request,
                                                              AttributeValueDecoder & decoder);
    bool DecodeCircuitNode(const Structs::CircuitNodeStruct::DecodableType & decoded, FabricIndex fabricIndex,
                           StoredCircuitNode & out) const;
    size_t CountNodesForFabric(FabricIndex fabricIndex) const;
    void RemoveNodesForFabric(FabricIndex fabricIndex);
    CHIP_ERROR LoadCircuitNodes();
    CHIP_ERROR SaveCircuitNodes();

    // FabricTable::Delegate: purge a removed fabric's ElectricalCircuitNodes entries (fabric-scoped data).
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    PowerTopology::Delegate & mDelegate;
    const BitMask<PowerTopology::Feature> mFeatureFlags;
    FabricTable * mFabricTable;

    StoredCircuitNode mCircuitNodes[kMaxCircuitNodes];
    size_t mCircuitNodeCount = 0;
};

} // namespace chip::app::Clusters::PowerTopology
