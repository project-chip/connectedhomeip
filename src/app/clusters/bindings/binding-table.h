/**
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

/**
 * @file API declarations for a binding table.
 */

#pragma once

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace Binding {

/**
 * @brief Defines binding entry types.
 */
enum EntryType : uint8_t
{
    /** A binding that is currently not in use. */
    MATTER_UNUSED_BINDING = 0,
    /** A unicast binding whose 64-bit identifier is the destination EUI64. */
    MATTER_UNICAST_BINDING = 1,
    /** A multicast binding whose 64-bit identifier is the group address. This
     * binding can be used to send messages to the group and to receive
     * messages sent to the group. */
    MATTER_MULTICAST_BINDING = 3,
};

/** @brief Defines an entry in the binding table.
 *
 * A binding entry specifies a local endpoint, a remote endpoint, a
 * cluster ID and either the destination EUI64 (for unicast bindings) or the
 * 64-bit group address (for multicast bindings).
 */
struct TableEntry
{
    TableEntry(FabricIndex fabric, NodeId node, EndpointId localEndpoint, EndpointId remoteEndpoint,
               std::optional<ClusterId> cluster) :
        type(MATTER_UNICAST_BINDING),
        fabricIndex(fabric), local(localEndpoint), clusterId(cluster), remote(remoteEndpoint), nodeId(node)
    {}

    TableEntry(FabricIndex fabric, GroupId group, EndpointId localEndpoint, std::optional<ClusterId> cluster) :
        type(MATTER_MULTICAST_BINDING), fabricIndex(fabric), local(localEndpoint), clusterId(cluster), remote(kInvalidEndpointId),
        groupId(group)
    {}

    TableEntry() = default;

    static TableEntry ForNode(FabricIndex fabric, NodeId node, EndpointId localEndpoint, EndpointId remoteEndpoint,
                              std::optional<ClusterId> cluster)
    {
        return TableEntry(fabric, node, localEndpoint, remoteEndpoint, cluster);
    }

    static TableEntry ForGroup(FabricIndex fabric, GroupId group, EndpointId localEndpoint, std::optional<ClusterId> cluster)
    {
        return TableEntry(fabric, group, localEndpoint, cluster);
    }

    /** The type of binding. */
    EntryType type = MATTER_UNUSED_BINDING;

    chip::FabricIndex fabricIndex;
    /** The endpoint on the local node. */
    chip::EndpointId local;
    /** A cluster ID that matches one from the local endpoint's simple descriptor.
     * This cluster ID is set by the provisioning application to indicate which
     * part an endpoint's functionality is bound to this particular remote node
     * and is used to distinguish between unicast and multicast bindings. Note
     * that a binding can be used to to send messages with any cluster ID, not
     * just that listed in the binding.
     */
    std::optional<chip::ClusterId> clusterId;
    /** The endpoint on the remote node (specified by \c identifier). */
    chip::EndpointId remote;
    /** A 64-bit destination identifier.  This is either:
     * - The destination chip::NodeId, for unicasts.
     * - A multicast ChipGroupId, for multicasts.
     * Which one is being used depends on the type of this binding.
     */
    union
    {
        chip::NodeId nodeId;
        chip::GroupId groupId;
    };

    bool operator==(TableEntry const & other) const
    {
        if (type != other.type)
        {
            return false;
        }

        if (type == MATTER_MULTICAST_BINDING && groupId != other.groupId)
        {
            return false;
        }

        if (type == MATTER_UNICAST_BINDING && (nodeId != other.nodeId || remote != other.remote))
        {
            return false;
        }

        return fabricIndex == other.fabricIndex && local == other.local && clusterId == other.clusterId;
    }
};

class Table
{
    friend class Iterator;

public:
    static constexpr size_t kMaxBindingEntries = CHIP_CONFIG_MAX_BINDING_ENTRIES_PER_FABRIC * CHIP_CONFIG_MAX_FABRICS;
    Table();

    class Iterator
    {
        friend class Table;

    public:
        TableEntry & operator*() { return mTable->mBindingTable[mIndex]; }

        const TableEntry & operator*() const { return mTable->mBindingTable[mIndex]; }

        TableEntry * operator->() { return &(mTable->mBindingTable[mIndex]); }

        const TableEntry * operator->() const { return &(mTable->mBindingTable[mIndex]); }

        Iterator operator++();

        bool operator==(const Iterator & rhs) const { return mIndex == rhs.mIndex; }

        bool operator!=(const Iterator & rhs) const { return mIndex != rhs.mIndex; }

        uint8_t GetIndex() const { return mIndex; }

    private:
        Table * mTable;
        uint8_t mPrevIndex;
        uint8_t mIndex;
    };

    CHIP_ERROR Add(const TableEntry & entry);

    const TableEntry & GetAt(uint8_t index);

    // The iter will be moved to the next item in the table after calling RemoveAt.
    CHIP_ERROR RemoveAt(Iterator & iter);

    // Returns the number of active entries in the binding table.
    // *NOTE* The function does not return the capacity of the binding table.
    uint8_t Size() const { return mSize; }

    Iterator begin();

    Iterator end();

    void SetPersistentStorage(PersistentStorageDelegate * storage) { mStorage = storage; }

    CHIP_ERROR LoadFromStorage();

    static Table & GetInstance() { return sInstance; }

private:
    static Table sInstance;

    static constexpr uint32_t kStorageVersion  = 1;
    static constexpr uint8_t kEntryStorageSize = TLV::EstimateStructOverhead(
        sizeof(FabricIndex), sizeof(EndpointId), sizeof(ClusterId), sizeof(EndpointId), sizeof(NodeId), sizeof(uint8_t));
    static constexpr uint8_t kListInfoStorageSize = TLV::EstimateStructOverhead(sizeof(kStorageVersion), sizeof(uint8_t));

    static constexpr uint8_t kTagStorageVersion = 1;
    static constexpr uint8_t kTagHead           = 2;
    static constexpr uint8_t kTagFabricIndex    = 1;
    static constexpr uint8_t kTagLocalEndpoint  = 2;
    static constexpr uint8_t kTagCluster        = 3;
    static constexpr uint8_t kTagRemoteEndpoint = 4;
    static constexpr uint8_t kTagNodeId         = 5;
    static constexpr uint8_t kTagGroupId        = 6;
    static constexpr uint8_t kTagNextEntry      = 7;
    static constexpr uint8_t kNextNullIndex     = 255;

    uint8_t GetNextAvaiableIndex();

    CHIP_ERROR SaveEntryToStorage(uint8_t index, uint8_t nextIndex);
    CHIP_ERROR SaveListInfo(uint8_t head);

    CHIP_ERROR LoadEntryFromStorage(uint8_t index, uint8_t & nextIndex);

    TableEntry mBindingTable[kMaxBindingEntries];
    uint8_t mNextIndex[kMaxBindingEntries];

    uint8_t mHead = kNextNullIndex;
    uint8_t mTail = kNextNullIndex;
    uint8_t mSize = 0;

    PersistentStorageDelegate * mStorage;
};

} // namespace Binding
} // namespace Clusters
} // namespace app
} // namespace chip
