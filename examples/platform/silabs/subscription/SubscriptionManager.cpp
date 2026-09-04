/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "SubscriptionManager.h"

#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/server/Server.h>
#include <credentials/FabricTable.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <utility>

namespace chip {
namespace Silabs {

using chip::app::AttributePathParams;
using chip::app::ConcreteDataAttributePath;
using chip::app::InteractionModelEngine;
using chip::app::ReadClient;
using chip::app::ReadPrepareParams;
using chip::app::StatusIB;

// ---------------------------------------------------------------------------
// Instance
// ---------------------------------------------------------------------------

SubscriptionManager & SubscriptionManager::Instance()
{
    static SubscriptionManager sInstance;
    return sInstance;
}

SubscriptionManager::SubscriptionManager()
{
    for (Handle i = 0; i < kMaxSubscriptions; ++i)
    {
        mSlots[i].callback.Init(this, i);
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

CHIP_ERROR SubscriptionManager::Subscribe(const Info & info, Handle * outHandle, uint16_t minIntervalSeconds,
                                          uint16_t maxIntervalSeconds)
{
    if (outHandle != nullptr)
    {
        *outHandle = kInvalidHandle;
    }

    const FabricTable & fabricTable = Server::GetInstance().GetFabricTable();
    VerifyOrReturnError(fabricTable.FindFabricWithIndex(info.fabricIndex) != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);

    Handle slot = FindFreeSlot();
    VerifyOrReturnError(slot != kInvalidHandle, CHIP_ERROR_NO_MEMORY);

    auto * pathParams = Platform::New<AttributePathParams>(info.endpointId, info.clusterId, info.attributeId);
    VerifyOrReturnError(pathParams != nullptr, CHIP_ERROR_NO_MEMORY);

    ReadPrepareParams readParams;
    readParams.mpAttributePathParamsList    = pathParams;
    readParams.mAttributePathParamsListSize = 1;
    readParams.mMinIntervalFloorSeconds     = minIntervalSeconds;
    readParams.mMaxIntervalCeilingSeconds   = maxIntervalSeconds;
    readParams.mKeepSubscriptions           = true;
    readParams.mIsFabricFiltered            = true;

    auto * readClient = Platform::New<ReadClient>(InteractionModelEngine::GetInstance(), nullptr, mSlots[slot].callback,
                                                  ReadClient::InteractionType::Subscribe);
    if (readClient == nullptr)
    {
        Platform::Delete(pathParams);
        return CHIP_ERROR_NO_MEMORY;
    }

    ScopedNodeId peer(info.nodeId, info.fabricIndex);

    CHIP_ERROR err = readClient->SendAutoResubscribeRequest(peer, std::move(readParams));
    if (err != CHIP_NO_ERROR)
    {
        // On failure, ReadClient has already invoked OnDeallocatePaths, which freed pathParams.
        Platform::Delete(readClient);
        return err;
    }

    mSlots[slot].client = readClient;
    mSlots[slot].info   = info;

    if (outHandle != nullptr)
    {
        *outHandle = slot;
    }
    return CHIP_NO_ERROR;
}

uint8_t SubscriptionManager::ActiveCount() const
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < kMaxSubscriptions; ++i)
    {
        if (mSlots[i].client != nullptr)
        {
            ++count;
        }
    }
    return count;
}

bool SubscriptionManager::IsActive(Handle handle) const
{
    return handle < kMaxSubscriptions && mSlots[handle].client != nullptr;
}

const SubscriptionManager::Info * SubscriptionManager::GetInfo(Handle handle) const
{
    if (!IsActive(handle))
    {
        return nullptr;
    }
    return &mSlots[handle].info;
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

SubscriptionManager::Handle SubscriptionManager::FindFreeSlot()
{
    for (Handle i = 0; i < kMaxSubscriptions; ++i)
    {
        if (mSlots[i].client == nullptr)
        {
            return i;
        }
    }
    return kInvalidHandle;
}

void SubscriptionManager::ReleaseSlot(Handle handle)
{
    if (handle >= kMaxSubscriptions)
    {
        return;
    }
    mSlots[handle].client = nullptr;
    mSlots[handle].info   = Info{};
}

// ---------------------------------------------------------------------------
// SlotCallback
// ---------------------------------------------------------------------------

void SubscriptionManager::SlotCallback::Init(SubscriptionManager * owner, Handle handle)
{
    mOwner  = owner;
    mHandle = handle;
}

void SubscriptionManager::SlotCallback::OnAttributeData(const ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                                                        const StatusIB & status)
{
    if (mOwner == nullptr || mOwner->mDelegate == nullptr)
    {
        return;
    }
    mOwner->mDelegate->OnAttributeData(mHandle, mOwner->mSlots[mHandle].info, path, data, status);
}

void SubscriptionManager::SlotCallback::OnSubscriptionEstablished(chip::SubscriptionId subscriptionId)
{
    if (mOwner == nullptr || mOwner->mDelegate == nullptr)
    {
        return;
    }
    mOwner->mDelegate->OnSubscriptionEstablished(mHandle, mOwner->mSlots[mHandle].info, subscriptionId);
}

void SubscriptionManager::SlotCallback::OnError(CHIP_ERROR error)
{
    if (mOwner == nullptr || mOwner->mDelegate == nullptr)
    {
        return;
    }
    mOwner->mDelegate->OnError(mHandle, mOwner->mSlots[mHandle].info, error);
}

void SubscriptionManager::SlotCallback::OnDone(ReadClient * client)
{
    if (mOwner == nullptr)
    {
        return;
    }

    Info info = mOwner->mSlots[mHandle].info;
    mOwner->ReleaseSlot(mHandle);

    if (mOwner->mDelegate != nullptr)
    {
        mOwner->mDelegate->OnSubscriptionTerminated(mHandle, info);
    }

    Platform::Delete(client);
}

void SubscriptionManager::SlotCallback::OnDeallocatePaths(ReadPrepareParams && readPrepareParams)
{
    if (readPrepareParams.mpAttributePathParamsList != nullptr)
    {
        Platform::Delete(readPrepareParams.mpAttributePathParamsList);
        readPrepareParams.mpAttributePathParamsList    = nullptr;
        readPrepareParams.mAttributePathParamsListSize = 0;
    }
}

} // namespace Silabs
} // namespace chip
