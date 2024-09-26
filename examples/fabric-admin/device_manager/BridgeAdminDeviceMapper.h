/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <optional>
#include <memory>
#include <set>
#include <unordered_map>

#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>

struct ScopedNodeIdHasher
{
    std::size_t operator()(const chip::ScopedNodeId& scopedNodeId) const
    {
        std::size_t h1 = std::hash<uint64_t>{}(scopedNodeId.GetFabricIndex());
        std::size_t h2 = std::hash<uint64_t>{}(scopedNodeId.GetNodeId());
        // Bitshifting h2 reduces collisions where fabricIndex == nodeId resulting
        // in hash return of 0.
        return h1 ^ (h2 << 1);
    }
};

// Bi-directional translation between handle for aggregator and information about the
// the device required for fabric admin to communicate with local device.
class BridgeAdminDeviceMapper
{
public:
    std::optional<uint64_t> AddScopedNodeId(const chip::ScopedNodeId & scopedNodeId);
    void RemoveScopedNodeIdByHandleId(uint64_t handleId);

    std::optional<uint64_t> GetHandleId(const chip::ScopedNodeId & scopedNodeId);
    std::optional<chip::ScopedNodeId> GetScopedNodeId(uint64_t handleId);

private:
    uint64_t mNextHandleId = 0;
    // If we ever need more data other than ScopedNodeId we can change
    // mHandleIdToScopedNodeId value from ScopedNodeId to AggregatorDeviceInfo.
    std::unordered_map<uint64_t, chip::ScopedNodeId> mHandleIdToScopedNodeId;
    std::unordered_map<chip::ScopedNodeId, uint64_t, ScopedNodeIdHasher> mScopedNodeIdToHandleId;
};
