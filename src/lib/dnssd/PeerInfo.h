/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <credentials/FabricTable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/core/PeerId.h>

namespace chip {

/// A peer is identified by a node id within a compressed fabric ID
class PeerInfo
{
public:
    PeerInfo() : mNodeId(kUndefinedNodeId), mCompressedFabricId(kUndefinedCompressedFabricId) {}
    PeerInfo(NodeId nodeId, CompressedFabricId compressedFabricId) : mNodeId(nodeId), mCompressedFabricId(compressedFabricId) {}

    NodeId GetNodeId() const { return mNodeId; }
    CompressedFabricId GetCompressedFabricId() const { return mCompressedFabricId; }

    PeerInfo & SetNodeId(NodeId id)
    {
        mNodeId = id;
        return *this;
    }

    PeerInfo & SetCompressedFabricId(CompressedFabricId id)
    {
        mCompressedFabricId = id;
        return *this;
    }

    bool operator==(const PeerInfo & that) const
    {
        return (mNodeId == that.mNodeId) && (mCompressedFabricId == that.mCompressedFabricId);
    }
    bool operator!=(const PeerInfo & that) const { return !(*this == that); }

    static CHIP_ERROR FromPeerId(PeerInfo & result, const PeerId peerId, FabricTable * fabricTable);
    static CHIP_ERROR ToPeerId(PeerId & result, const PeerInfo peerInfo, FabricTable * fabricTable);

private:
    NodeId mNodeId;
    CompressedFabricId mCompressedFabricId;
};

} // namespace chip
