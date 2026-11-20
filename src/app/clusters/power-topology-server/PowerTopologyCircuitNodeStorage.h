/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/persistence/AttributePersistenceProvider.h>
#include <clusters/PowerTopology/Structs.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

#include <cstddef>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

/// Storage for the ElectricalCircuitNodes attribute (CIRC feature).
///
/// This is deliberately separate from the cluster so that how the entries are held and persisted is
/// the application's choice rather than the cluster's. The cluster keeps only a reference and never
/// allocates, so a platform with strict memory requirements (no heap) can back this with a fixed
/// array and advertise its real limit through Capacity().
///
/// See DefaultCircuitNodeStorage for the implementation most applications will want, which persists
/// entries through a PersistentStorageDelegate.
///
/// Entries are fabric-scoped. Implementations must keep every stored entry's fabricIndex intact,
/// since the cluster relies on it for fabric filtering and for purging a removed fabric.
class CircuitNodeStorage
{
public:
    /// Spec constraint on CircuitNodeStruct::Label.
    static constexpr size_t kMaxNodeLabelLength = 128;

    /// Spec constraint on the ElectricalCircuitNodes list. An implementation may report a smaller
    /// Capacity(), but never a larger one.
    static constexpr size_t kMaxCircuitNodes = 50;

    /// One stored entry.
    ///
    /// The Label bytes are owned here rather than referenced, because the CharSpan in the generated
    /// CircuitNodeStruct must stay valid for as long as the encoder needs it.
    struct Node
    {
        NodeId node = kUndefinedNodeId;
        Optional<EndpointId> endpoint;
        bool hasLabel = false;
        char label[kMaxNodeLabelLength];
        size_t labelLength      = 0;
        FabricIndex fabricIndex = kUndefinedFabricIndex;

        /// View of this entry as the generated struct. The returned Label span points into `label`,
        /// so the Node must outlive the returned value.
        Structs::CircuitNodeStruct::Type AsStruct() const
        {
            Structs::CircuitNodeStruct::Type value;
            value.node        = node;
            value.endpoint    = endpoint;
            value.fabricIndex = fabricIndex;
            if (hasLabel)
            {
                value.label.SetValue(CharSpan(label, labelLength));
            }
            return value;
        }
    };

    virtual ~CircuitNodeStorage() = default;

    /// Lifecycle hook called once from the cluster's Startup(), which is the earliest point at
    /// which the cluster's attribute storage exists.
    ///
    /// It is provided for implementations that persist through the cluster's own attribute storage
    /// (see DefaultCircuitNodeStorage). An implementation with its own persistence, or none, should
    /// leave this as the default no-op. Returning an error fails cluster startup.
    virtual CHIP_ERROR Init(AttributePersistenceProvider & attributeStorage, EndpointId endpointId)
    {
        (void) attributeStorage;
        (void) endpointId;
        return CHIP_NO_ERROR;
    }

    /// Maximum number of entries that can be stored, across all fabrics.
    /// Must not exceed kMaxCircuitNodes.
    virtual size_t Capacity() const = 0;

    /// Number of entries currently stored, across all fabrics.
    virtual size_t Count() const = 0;

    /// Number of stored entries belonging to `fabricIndex`.
    virtual size_t CountForFabric(FabricIndex fabricIndex) const = 0;

    /// Reads the entry at `index`, which must be less than Count().
    /// Returns CHIP_ERROR_INVALID_ARGUMENT if the index is out of range.
    virtual CHIP_ERROR GetNodeAtIndex(size_t index, Node & outNode) const = 0;

    /// Atomically replaces every entry belonging to `fabricIndex` with `nodes`, leaving entries
    /// belonging to other fabrics untouched. Serves the ReplaceAll list write.
    ///
    /// Returns CHIP_ERROR_NO_MEMORY if the result would exceed Capacity(); in that case the stored
    /// contents must be left unchanged.
    virtual CHIP_ERROR ReplaceNodesForFabric(FabricIndex fabricIndex, const Node * nodes, size_t count) = 0;

    /// Appends a single entry, which carries its own fabricIndex. Serves the AppendItem list write.
    ///
    /// Returns CHIP_ERROR_NO_MEMORY if Capacity() is already reached.
    virtual CHIP_ERROR AppendNode(const Node & node) = 0;

    /// Removes every entry belonging to `fabricIndex`. Removing a fabric with no entries succeeds.
    virtual CHIP_ERROR RemoveNodesForFabric(FabricIndex fabricIndex) = 0;
};

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
