/*
 *    Copyright (c) 2022 Project CHIP Authors
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

/// The ScopedNodeId provides an identifier for an operational node on the network that is only valid for use within the current
/// Matter stack instance. It is not to be exchanged or directly used remotely over the network.
class ScopedNodeId
{
public:
    ScopedNodeId() : mNodeId(kUndefinedNodeId), mFabricIndex(kUndefinedFabricIndex) {}
    ScopedNodeId(NodeId nodeId, FabricIndex fabricIndex) : mNodeId(nodeId), mFabricIndex(fabricIndex) {}

    NodeId GetNodeId() const { return mNodeId; }
    FabricIndex GetFabricIndex() const { return mFabricIndex; }

    bool IsOperational() const { return mFabricIndex != kUndefinedFabricIndex && IsOperationalNodeId(mNodeId); }
    bool operator==(const ScopedNodeId & that) const { return (mNodeId == that.mNodeId) && (mFabricIndex == that.mFabricIndex); }
    bool operator!=(const ScopedNodeId & that) const { return !(*this == that); }

private:
    NodeId mNodeId;
    FabricIndex mFabricIndex;
};

} // namespace chip
