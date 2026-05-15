/*
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

#include "RangingTechnologyController.h"

#include <clusters/ProximityRanging/AttributeIds.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

RangingTechnologyController::~RangingTechnologyController()
{
    // Clear listeners first to prevent callbacks during teardown
    mListenerCount = 0;
    for (size_t i = 0; i < mAdapterCount; i++)
    {
        mAdapters[i]->StopAllSessions();
        mAdapters[i]->SetCallback(nullptr);
    }
}

CHIP_ERROR RangingTechnologyController::AddListener(Listener * listener)
{
    VerifyOrReturnError(listener != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    for (size_t i = 0; i < mListenerCount; i++)
    {
        if (mListeners[i] == listener)
        {
            return CHIP_NO_ERROR;
        }
    }
    VerifyOrReturnError(mListenerCount < kMaxListeners, CHIP_ERROR_NO_MEMORY);
    mListeners[mListenerCount++] = listener;
    return CHIP_NO_ERROR;
}

void RangingTechnologyController::RemoveListener(Listener * listener)
{
    for (size_t i = 0; i < mListenerCount; i++)
    {
        if (mListeners[i] == listener)
        {
            mListeners[i]              = mListeners[--mListenerCount];
            mListeners[mListenerCount] = nullptr;
            return;
        }
    }
}

CHIP_ERROR RangingTechnologyController::RegisterAdapter(RangingAdapter & adapter)
{
    for (size_t i = 0; i < mAdapterCount; i++)
    {
        VerifyOrReturnError(mAdapters[i]->GetTechnology() != adapter.GetTechnology(), CHIP_ERROR_DUPLICATE_KEY_ID);
    }
    VerifyOrReturnError(mAdapterCount < kMaxControllerAdapters, CHIP_ERROR_NO_MEMORY);

    mAdapters[mAdapterCount++] = &adapter;
    adapter.SetCallback(this);
    OnAttributeChanged(Attributes::RangingCapabilities::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR RangingTechnologyController::UnregisterAdapter(RangingAdapter & adapter)
{
    for (size_t i = 0; i < mAdapterCount; i++)
    {
        if (mAdapters[i] != &adapter)
        {
            continue;
        }

        // Stop all sessions owned by this adapter
        adapter.StopAllSessions();
        adapter.SetCallback(nullptr);

        // Remove sessions tracked by this adapter
        for (auto it = mSessions.begin(); it != mSessions.end();)
        {
            if (it->adapter == &adapter)
            {
                it = mSessions.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Compact the adapter array
        mAdapters[i]             = mAdapters[--mAdapterCount];
        mAdapters[mAdapterCount] = nullptr;

        OnAttributeChanged(Attributes::RangingCapabilities::Id);
        OnAttributeChanged(Attributes::SessionIDList::Id);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR RangingTechnologyController::GetRangingCapabilities(AttributeValueEncoder & encoder)
{
    return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
        for (size_t i = 0; i < mAdapterCount; i++)
        {
            ReturnErrorOnFailure(listEncoder.Encode(mAdapters[i]->GetCapabilities()));
        }
        return CHIP_NO_ERROR;
    });
}

ResultCodeEnum RangingTechnologyController::StartSession(uint8_t sessionId,
                                                         const Commands::StartRangingRequest::DecodableType & request)
{
    RangingAdapter * adapter = FindAdapter(request.technology);
    VerifyOrReturnValue(adapter != nullptr, ResultCodeEnum::kRejectedInfeasibleRanging);

    SessionEntry entry;
    entry.sessionId = sessionId;
    entry.adapter   = adapter;

    ResultCodeEnum result = adapter->StartSession(sessionId, request);
    if (result == ResultCodeEnum::kAccepted)
    {
        mSessions.push_back(entry);
        OnAttributeChanged(Attributes::SessionIDList::Id);
    }
    return result;
}

CHIP_ERROR RangingTechnologyController::StopSession(uint8_t sessionId)
{
    // Bookkeeping (mSessions erase + SessionIDList notification) happens in
    // OnRangingSessionStopped, which adapters are required to invoke for any
    // session removal.
    for (auto & entry : mSessions)
    {
        if (entry.sessionId == sessionId)
        {
            return entry.adapter->StopSession(sessionId);
        }
    }
    return CHIP_ERROR_NOT_FOUND;
}

void RangingTechnologyController::StopAllSessions()
{
    // Take a snapshot first: adapter->StopSession will fire OnRangingSessionStopped
    // synchronously, which mutates mSessions.
    std::vector<SessionEntry> snapshot = mSessions;
    for (auto & entry : snapshot)
    {
        LogErrorOnFailure(entry.adapter->StopSession(entry.sessionId));
    }
}

CHIP_ERROR RangingTechnologyController::GetActiveSessionIds(Span<uint8_t> & sessionIds)
{
    if (sessionIds.size() < mSessions.size())
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    for (size_t i = 0; i < mSessions.size(); i++)
    {
        sessionIds.data()[i] = mSessions[i].sessionId;
    }
    sessionIds.reduce_size(mSessions.size());
    return CHIP_NO_ERROR;
}

void RangingTechnologyController::OnRangingSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status)
{
    for (auto it = mSessions.begin(); it != mSessions.end(); ++it)
    {
        if (it->sessionId == sessionId)
        {
            mSessions.erase(it);
            break;
        }
    }

    for (size_t i = 0; i < mListenerCount; i++)
    {
        mListeners[i]->OnAttributeChanged(Attributes::SessionIDList::Id);
        mListeners[i]->OnSessionStopped(sessionId, status);
    }
}

void RangingTechnologyController::OnMeasurementData(uint8_t sessionId,
                                                    const Structs::RangingMeasurementDataStruct::Type & measurement)
{
    for (size_t i = 0; i < mListenerCount; i++)
    {
        mListeners[i]->OnSessionMeasurement(sessionId, measurement);
    }
}

void RangingTechnologyController::OnAttributeChanged(AttributeId attributeId)
{
    for (size_t i = 0; i < mListenerCount; i++)
    {
        mListeners[i]->OnAttributeChanged(attributeId);
    }
}

RangingAdapter * RangingTechnologyController::FindAdapter(RangingTechEnum technology)
{
    for (size_t i = 0; i < mAdapterCount; i++)
    {
        if (mAdapters[i]->GetTechnology() == technology)
        {
            return mAdapters[i];
        }
    }
    return nullptr;
}

uint64_t RangingTechnologyController::GetBleDeviceId()
{
    auto * adapter = FindAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    if (adapter == nullptr)
    {
        return 0;
    }
    std::optional<uint64_t> deviceId = adapter->GetDeviceId();
    // Return 0 (invalid ID) if no ID returned
    return deviceId.value_or(0);
}

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
