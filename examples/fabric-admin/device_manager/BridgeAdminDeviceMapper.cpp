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

#include <lib/support/CodeUtils.h>

std::optional<uint64_t> BridgeAdminDeviceMapper::AddAdminScopedNodeId(const chip::ScopedNodeId & scopedNodeId)
{
    VerifyOrReturnValue(mScopedNodeIdToHandle.find(scopedNodeId) == mScopedNodeIdToHandle.end(), std::nullopt, ChipLogError(NotSpecified, "Duplicate ScopedNodeId alread exists in map"));

    uint64_t handle                     = mNextHandle;
    mHandleToScopedNodeId[handle]     = scopedNodeId;
    mScopedNodeIdToHandle[scopedNodeId] = handle;
    // We are assuming that we will never run out of Handles because we are using uint64_t here.
    static_assert(sizeof(mNextHandle) == sizeof(uint64_t));
    mNextHandle++;
    return handle;
}

void BridgeAdminDeviceMapper::RemoveScopedNodeIdByBridgeHandle(uint64_t handle)
{
    auto it = mHandleToScopedNodeId.find(handle);
    VerifyOrReturn(it != mHandleToScopedNodeId.end());
    mScopedNodeIdToHandle.erase(it->second);
    mHandleToScopedNodeId.erase(handle);
}

std::optional<uint64_t> BridgeAdminDeviceMapper::GetHandleForBridge(const chip::ScopedNodeId & scopedNodeId)
{
    auto scopedNodeIterator = mScopedNodeIdToHandle.find(scopedNodeId);
    VerifyOrReturnValue(scopedNodeIterator != mScopedNodeIdToHandle.end(), std::nullopt);
    return scopedNodeIterator->second;
}

std::optional<chip::ScopedNodeId> BridgeAdminDeviceMapper::GetScopedNodeIdForAdmin(uint64_t handle)
{
    auto it = mHandleToScopedNodeId.find(handle);
    VerifyOrReturnValue(it != mHandleToScopedNodeId.end(), std::nullopt);
    return it->second;
}
