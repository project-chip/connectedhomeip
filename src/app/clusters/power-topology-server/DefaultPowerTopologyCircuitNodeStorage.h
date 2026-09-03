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

#include <app/clusters/power-topology-server/PowerTopologyCircuitNodeStorage.h>

#include <app/persistence/AttributePersistenceProvider.h>
#include <lib/support/ScopedMemoryBuffer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

/// CircuitNodeStorage that persists through the cluster's attribute storage.
///
/// This is the implementation most applications want: entries survive a reboot with no work on the
/// application's part, stored against the endpoint's ElectricalCircuitNodes attribute path, which is
/// the same location and format the cluster used before storage was factored out.
///
/// Entries are held in a single heap buffer allocated once by Init(), and the whole set is
/// re-serialised after every mutation. A platform that cannot allocate, or that wants a different
/// persistence strategy, should implement CircuitNodeStorage directly rather than use this class.
/// That is the reason the interface exists.
///
/// The persisted value is written as a single blob, so its worst case size is bounded by
/// Capacity(): roughly Capacity() * (kMaxNodeLabelLength + 32) bytes, which at the default
/// capacity of 50 is about 8 kB. Some platforms cap the size of one stored value well below that
/// (4 kB and 2 kB are both in use), and a write past the cap fails at runtime rather than at
/// build time. Such a platform should subclass and lower Capacity() to fit its own limit; roughly
/// 25 entries fit in 4 kB and 12 in 2 kB. Lowering it is safe at any point, including on an
/// existing device: Load() stops at Capacity() instead of failing, so a device whose stored list
/// is longer than a reduced capacity still starts, keeping the leading entries.
class DefaultCircuitNodeStorage : public CircuitNodeStorage
{
public:
    DefaultCircuitNodeStorage() = default;

    /// Allocates the node buffer and restores any persisted entries.
    ///
    /// Call from the cluster's Startup(), which is when attribute storage becomes available. A
    /// missing persisted value is normal on first boot and is not reported as an error. Returns
    /// CHIP_ERROR_NO_MEMORY if the buffer cannot be allocated.
    CHIP_ERROR Init(AttributePersistenceProvider & attributeStorage, EndpointId endpointId) override;

    size_t Capacity() const override { return kMaxCircuitNodes; }
    size_t Count() const override { return mCount; }
    size_t CountForFabric(FabricIndex fabricIndex) const override;
    CHIP_ERROR GetNodeAtIndex(size_t index, Node & outNode) const override;
    CHIP_ERROR ReplaceNodesForFabric(FabricIndex fabricIndex, const Node * nodes, size_t count) override;
    CHIP_ERROR AppendNode(const Node & node) override;
    CHIP_ERROR RemoveNodesForFabric(FabricIndex fabricIndex) override;

private:
    /// Drops every entry belonging to `fabricIndex`, compacting in place. Does not persist.
    /// Returns the number of entries removed.
    size_t EraseFabric(FabricIndex fabricIndex);

    CHIP_ERROR Save() const;
    CHIP_ERROR Load();

    AttributePersistenceProvider * mAttributeStorage = nullptr;
    EndpointId mEndpointId                           = kInvalidEndpointId;

    Platform::ScopedMemoryBuffer<Node> mNodes;
    size_t mCount = 0;
};

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
