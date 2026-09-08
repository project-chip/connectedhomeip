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

#include <app/clusters/power-topology-server/PowerTopologyCircuitNodeStorage.h>
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
#include <lib/support/ScopedMemoryBuffer.h>

#include <cstddef>

namespace chip::app::Clusters::PowerTopology {

class PowerTopologyCluster : public DefaultServerCluster, private FabricTable::Delegate
{

public:
    // Spec constraints for the ElectricalCircuitNodes attribute (CIRC feature). Defined by the
    // storage interface, since it is the storage that has to honour them.
    static constexpr size_t kMaxCircuitNodes    = CircuitNodeStorage::kMaxCircuitNodes;
    static constexpr size_t kMaxNodeLabelLength = CircuitNodeStorage::kMaxNodeLabelLength;

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
        // Required when the CIRC feature is enabled: holds the ElectricalCircuitNodes entries. The
        // cluster does not own or allocate this; see CircuitNodeStorage for why it is injected.
        CircuitNodeStorage * circuitNodeStorage = nullptr;
    };

    PowerTopologyCluster(const Config & config) :
        DefaultServerCluster({ config.endpointId, PowerTopology::Id }), mDelegate(config.delegate), mFeatureFlags(config.features),
        mFabricTable(config.fabricTable), mCircuitNodeStorage(config.circuitNodeStorage)
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

    /// Brackets a list write so a failed one can be undone. A client writes a list as ReplaceAll
    /// followed by one AppendItem per entry, and each of those arrives as its own WriteAttribute
    /// call; without this the entries already applied when a later one is rejected would stay.
    void ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType,
                                        FabricIndex accessingFabric) override;

private:
    // ElectricalCircuitNodes (CIRC): fabric-scoped, writable, non-volatile list.
    DataModel::ActionReturnStatus WriteElectricalCircuitNodes(const DataModel::WriteAttributeRequest & request,
                                                              AttributeValueDecoder & decoder);
    bool DecodeCircuitNode(const Structs::CircuitNodeStruct::DecodableType & decoded, FabricIndex fabricIndex,
                           CircuitNodeStorage::Node & out) const;

    // Copies the accessing fabric's entries aside for the duration of a list write. On error the
    // copy was not taken and a failed write cannot be rolled back.
    CHIP_ERROR SnapshotNodesForFabric(FabricIndex fabricIndex);
    void ReleaseNodeSnapshot();

    // FabricTable::Delegate: purge a removed fabric's ElectricalCircuitNodes entries (fabric-scoped data).
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    PowerTopology::Delegate & mDelegate;
    const BitMask<PowerTopology::Feature> mFeatureFlags;
    FabricTable * mFabricTable;

    // Not owned. Non-null whenever the CIRC feature is enabled (checked at Startup). The cluster
    // holds no ElectricalCircuitNodes entries of its own; it allocates only for the duration of a
    // single write, to stage the incoming list and to hold the rollback snapshot below.
    CircuitNodeStorage * mCircuitNodeStorage;

    // The accessing fabric's entries as they were when the current list write began, restored if
    // that write fails. Empty outside a list write. mNodeSnapshotValid distinguishes "the fabric
    // had no entries" from "the snapshot could not be taken", since only the former may be restored.
    Platform::ScopedMemoryBuffer<CircuitNodeStorage::Node> mNodeSnapshot;
    size_t mNodeSnapshotCount = 0;
    bool mNodeSnapshotValid   = false;
};

} // namespace chip::app::Clusters::PowerTopology
