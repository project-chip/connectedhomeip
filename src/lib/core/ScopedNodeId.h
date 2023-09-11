/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
