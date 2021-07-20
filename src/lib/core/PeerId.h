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

namespace chip {

/// Convenience types to make it clear what different number types mean
using NodeId   = uint64_t;
using FabricId = uint64_t;

constexpr NodeId kUndefinedNodeId = 0ULL;
constexpr NodeId kAnyNodeId       = 0xFFFFFFFFFFFFFFFFULL;

constexpr FabricId kUndefinedFabricId = 0ULL;
constexpr uint16_t kUndefinedVendorId = 0U;

/// A peer is identified by a node id within a fabric
class PeerId
{
public:
    PeerId() {}

    NodeId GetNodeId() const { return mNodeId; }
    PeerId & SetNodeId(NodeId id)
    {
        mNodeId = id;
        return *this;
    }

    FabricId GetFabricId() const { return mFabricId; }
    PeerId & SetFabricId(FabricId id)
    {
        mFabricId = id;
        return *this;
    }

    bool operator==(const PeerId & other) const { return (mNodeId == other.mNodeId) && (mFabricId == other.mFabricId); }
    bool operator!=(const PeerId & other) const { return (mNodeId != other.mNodeId) || (mFabricId != other.mFabricId); }

private:
    NodeId mNodeId     = kUndefinedNodeId;
    FabricId mFabricId = kUndefinedFabricId;
};

} // namespace chip
