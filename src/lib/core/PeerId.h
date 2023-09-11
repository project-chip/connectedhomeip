/*
 *
 * SPDX-FileCopyrightText: 2021-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

namespace chip {

/* NOTE: PeerId should be only used by mDNS, because it contains a compressed fabric id which is not unique, and the compressed
 * fabric id is only used for mDNS announcement. ScopedNodeId which contains a node id and fabirc index, should be used in prefer of
 * PeerId. ScopedNodeId is locally unique.
 */
// TODO: remove PeerId usage outside lib/dns, move PeerId into lib/dns
/// A peer is identified by a node id within a compressed fabric ID
class PeerId
{
public:
    PeerId() {}
    PeerId(CompressedFabricId compressedFabricId, NodeId nodeId) : mNodeId(nodeId), mCompressedFabricId(compressedFabricId) {}

    NodeId GetNodeId() const { return mNodeId; }
    PeerId & SetNodeId(NodeId id)
    {
        mNodeId = id;
        return *this;
    }

    CompressedFabricId GetCompressedFabricId() const { return mCompressedFabricId; }
    PeerId & SetCompressedFabricId(CompressedFabricId id)
    {
        mCompressedFabricId = id;
        return *this;
    }

    bool operator==(const PeerId & other) const
    {
        return (mNodeId == other.mNodeId) && (mCompressedFabricId == other.mCompressedFabricId);
    }
    bool operator!=(const PeerId & other) const
    {
        return (mNodeId != other.mNodeId) || (mCompressedFabricId != other.mCompressedFabricId);
    }

private:
    NodeId mNodeId = kUndefinedNodeId;

    CompressedFabricId mCompressedFabricId = kUndefinedCompressedFabricId;
};

} // namespace chip
