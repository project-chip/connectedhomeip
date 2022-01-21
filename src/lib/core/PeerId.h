/*
 *
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

#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

namespace chip {

using FabricId           = uint64_t;

constexpr FabricId kUndefinedFabricId = 0ULL;
constexpr uint16_t kUndefinedVendorId = 0U;

/// A peer is identified by a node id within a fabric.
class PeerId
{
public:
    PeerId() : mNodeId(kUndefinedNodeId), mFabricIndex(kUndefinedFabricIndex) {}
    PeerId(NodeId nodeId, FabricIndex fabricIndex) : mNodeId(nodeId), mFabricIndex(fabricIndex) {}

    PeerId(const PeerId &) = default;
    PeerId(PeerId &&) = default;
    PeerId & operator=(const PeerId &) = default;
    PeerId & operator=(PeerId &&) = default;

    NodeId GetNodeId() const { return mNodeId; }
    FabricIndex GetFabricIndex() const { return mFabricIndex; }

    bool operator==(const PeerId & other) const {
        if (mFabricIndex == kUndefinedFabricIndex)
        {
            if (mNodeId == kUndefinedNodeId || other.mNodeId == kUndefinedNodeId)
            {
                return false; // (kUndefinedFabricIndex, kUndefinedNodeId) are not equal to each other.
            }
            else
            {
                return mNodeId == other.mNodeId;
            }
        }
        else
        {
            return (mNodeId == other.mNodeId) && (mFabricIndex == other.mFabricIndex);
        }
    }

    bool operator!=(const PeerId & other) const { return !(*this == other); }

private:
    NodeId mNodeId;
    FabricIndex mFabricIndex;
};

} // namespace chip
