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

#include <lib/core/NodeId.h>

namespace chip {

class OperationalId
{
public:
    OperationalId(NodeId nodeId, FabricIndex fabricIndex) : mNodeId(nodeId), mFabricIndex(fabricIndex)
    {
        VerifyOrDie(mNodeId != kUndefinedNodeId);
        VerifyOrDie(mFabricIndex != kUndefinedFabricIndex);
    }

    NodeId GetNodeId() const { return mNodeId; }
    FabricIndex GetFabricIndex() const { return mFabricIndex; }

    static constexpr OperationalId UndefinedOperationalId() { return OperationalId(); }

    bool operator==(const OperationalId & that) const { return (mNodeId == that.mNodeId) && (mFabricIndex == that.mFabricIndex); }
    bool operator!=(const OperationalId & that) const { return !(*this == that); }

private:
    NodeId mNodeId;
    // TODO: replace FabricIndex with a pointer to FabricInfo structure.
    FabricIndex mFabricIndex;

    constexpr OperationalId() : mNodeId(kUndefinedNodeId), mFabricIndex(kUndefinedFabricIndex) {}
};

} // namespace chip
