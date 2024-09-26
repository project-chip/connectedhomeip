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

#include "BridgeAdminDeviceMapper.h"

#include <limits>

#include <lib/support/CodeUtils.h>

std::optional<uint64_t> BridgeAdminDeviceMapper::AddScopedNodeId(const chip::ScopedNodeId & scopedNodeId)
{
    // We are assuming that we will never run out of HandleIds
    VerifyOrDie(mNextHandleId != std::numeric_limits<uint64_t>::max());
    VerifyOrReturnValue(mScopedNodeIdToHandleId.find(scopedNodeId) == mScopedNodeIdToHandleId.end(), std::nullopt);

    uint64_t handleId                     = mNextHandleId;
    mHandleIdToScopedNodeId[handleId]     = scopedNodeId;
    mScopedNodeIdToHandleId[scopedNodeId] = handleId;
    mNextHandleId++;
    return handleId;
}

void BridgeAdminDeviceMapper::RemoveScopedNodeIdByHandleId(uint64_t handleId)
{
    auto it = mHandleIdToScopedNodeId.find(handleId);
    VerifyOrReturn(it != mHandleIdToScopedNodeId.end());
    mScopedNodeIdToHandleId.erase(it->second);
    mHandleIdToScopedNodeId.erase(handleId);
}

std::optional<uint64_t> BridgeAdminDeviceMapper::GetHandleId(const chip::ScopedNodeId & scopedNodeId)
{
    auto scopedNodeIterator = mScopedNodeIdToHandleId.find(scopedNodeId);
    VerifyOrReturnValue(scopedNodeIterator != mScopedNodeIdToHandleId.end(), std::nullopt);
    return scopedNodeIterator->second;
}

std::optional<chip::ScopedNodeId> BridgeAdminDeviceMapper::GetScopedNodeId(uint64_t handleId)
{
    auto it = mHandleIdToScopedNodeId.find(handleId);
    VerifyOrReturnValue(it != mHandleIdToScopedNodeId.end(), std::nullopt);
    return it->second;
}
