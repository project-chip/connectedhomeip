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

#include <app/clusters/proximity-ranging-server/ProximityRangingDriver.h>

#include <clusters/ProximityRanging/AttributeIds.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Iterators.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

ProximityRangingDriver::ProximityRangingDriver(Span<RangingAdapter * const> adapters) : mAdapters(adapters)
{
    // Adapter set is fixed at construction; configuration errors panic here
    // rather than surfacing as runtime registration failures the cluster would
    // have to handle.
    for (size_t i = 0; i < mAdapters.size(); i++)
    {
        VerifyOrDie(mAdapters[i] != nullptr);
        // Disallow multiple adapters of same technology
        for (size_t j = 0; j < i; j++)
        {
            VerifyOrDie(mAdapters[i]->GetTechnology() != mAdapters[j]->GetTechnology());
        }
        mAdapters[i]->SetCallback(this);
    }
}

ProximityRangingDriver::~ProximityRangingDriver()
{
    // Drop cluster callback first to avoid possibly-already-destroyed cluster, then
    // detach adapters so they cannot deliver callbacks.
    mClusterCallback = nullptr;
    for (size_t i = 0; i < mAdapters.size(); i++)
    {
        mAdapters[i]->SetCallback(nullptr);
        mAdapters[i]->StopAllSessions();
    }
    mSessions.ReleaseAll();
}

CHIP_ERROR ProximityRangingDriver::Init(Callback & callback)
{
    VerifyOrReturnError(mClusterCallback == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mClusterCallback = &callback;
    return CHIP_NO_ERROR;
}

void ProximityRangingDriver::Shutdown()
{
    // Take a snapshot list since adapter->StopSession will fire
    // OnRangingSessionStopped synchronously, which mutates mSessions.
    uint8_t snapshot[kMaxConcurrentSessions];
    size_t snapshotCount = 0;
    mSessions.ForEachActiveObject([&](Session * s) {
        if (snapshotCount < kMaxConcurrentSessions)
        {
            snapshot[snapshotCount++] = s->id;
        }
        return Loop::Continue;
    });
    for (size_t i = 0; i < snapshotCount; i++)
    {
        Session * s = FindSession(snapshot[i]);
        if (s != nullptr)
        {
            LogErrorOnFailure(s->adapter->StopSession(snapshot[i]));
        }
    }

    // Release any sessions an adapter failed to retire synchronously so a
    // re-Init starts from a clean pool.
    mSessions.ReleaseAll();
    mClusterCallback = nullptr;
}

ResultCodeEnum ProximityRangingDriver::HandleStartRanging(uint8_t sessionId,
                                                          const Commands::StartRangingRequest::DecodableType & request)
{
    RangingAdapter * adapter = FindAdapter(request.technology);
    VerifyOrReturnValue(adapter != nullptr, ResultCodeEnum::kRejectedInfeasibleRanging);

    // Adapters are required (see RangingAdapter.h) to defer any termination
    // callback until after StartSession has returned, so the session record
    // can safely be committed only after a successful start.
    ResultCodeEnum result = adapter->StartSession(sessionId, request);
    VerifyOrReturnValue(result == ResultCodeEnum::kAccepted, result);

    Session * session = mSessions.CreateObject(Session{ sessionId, adapter });
    if (session == nullptr)
    {
        // Pool exhausted after the adapter already accepted: stop the adapter
        // session so its bookkeeping does not diverge from the driver's.
        LogErrorOnFailure(adapter->StopSession(sessionId));
        return ResultCodeEnum::kBusySessionCapacityReached;
    }

    if (mClusterCallback != nullptr)
    {
        mClusterCallback->OnAttributeChanged(Attributes::SessionIDList::Id);
    }
    return result;
}

CHIP_ERROR ProximityRangingDriver::HandleStopRanging(uint8_t sessionId)
{
    Session * session = FindSession(sessionId);
    VerifyOrReturnError(session != nullptr, CHIP_ERROR_NOT_FOUND);

    // Bookkeeping happens in OnRangingSessionStopped, which adapters MUST
    // invoke for any session removal - including success of this call.
    return session->adapter->StopSession(sessionId);
}

CHIP_ERROR ProximityRangingDriver::GetRangingCapabilities(AttributeValueEncoder & encoder)
{
    return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
        for (size_t i = 0; i < mAdapters.size(); i++)
        {
            ReturnErrorOnFailure(listEncoder.Encode(mAdapters[i]->GetCapabilities()));
        }
        return CHIP_NO_ERROR;
    });
}

size_t ProximityRangingDriver::GetNumActiveSessionIds() const
{
    return mSessions.Allocated();
}

CHIP_ERROR ProximityRangingDriver::GetActiveSessionIds(Span<uint8_t> & out)
{
    const size_t active = GetNumActiveSessionIds();
    VerifyOrReturnError(out.size() >= active, CHIP_ERROR_BUFFER_TOO_SMALL);

    const size_t capacity = out.size();
    size_t i              = 0;
    mSessions.ForEachActiveObject([&](Session * s) {
        if (i >= capacity)
        {
            return Loop::Break;
        }
        out[i++] = s->id;
        return Loop::Continue;
    });
    out.reduce_size(i);
    return CHIP_NO_ERROR;
}

std::optional<BleRbcConfig> ProximityRangingDriver::GetBleRbcConfig()
{
    auto * adapter = FindAdapter(RangingTechEnum::kBLEBeaconRSSIRanging);
    VerifyOrReturnValue(adapter != nullptr, std::nullopt);
    auto deviceId = adapter->GetDeviceId();
    return BleRbcConfig{ deviceId.value_or(0) };
}

std::optional<WiFiUsdConfig> ProximityRangingDriver::GetWiFiUsdConfig()
{
    // The WFUSDPD feature covers two ranging-tech enums; either adapter, if
    // bound, supplies the cluster-level WiFiDevIK.
    RangingAdapter * adapter = FindAdapter(RangingTechEnum::kWiFiRoundTripTimeRanging);
    if (adapter != nullptr)
    {
        return adapter->GetWiFiUsdConfig();
    }
    adapter = FindAdapter(RangingTechEnum::kWiFiNextGenerationRanging);
    if (adapter != nullptr)
    {
        return adapter->GetWiFiUsdConfig();
    }
    return std::nullopt;
}

std::optional<BltcsConfig> ProximityRangingDriver::GetBltcsConfig()
{
    auto * adapter = FindAdapter(RangingTechEnum::kBluetoothChannelSounding);
    VerifyOrReturnValue(adapter != nullptr, std::nullopt);
    return adapter->GetBltcsConfig();
}

void ProximityRangingDriver::OnRangingSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status)
{
    // Drop spurious or stale adapter notifications for sessions the driver no
    // longer tracks (e.g. duplicate stop, post-Shutdown delivery).
    VerifyOrReturn(FindSession(sessionId) != nullptr);
    // RetireSession dirties SessionIDList as part of releasing the pool slot.
    RetireSession(sessionId);
    VerifyOrReturn(mClusterCallback != nullptr);
    mClusterCallback->OnSessionStopped(sessionId, status);
}

void ProximityRangingDriver::OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement)
{
    VerifyOrReturn(FindSession(sessionId) != nullptr);
    VerifyOrReturn(mClusterCallback != nullptr);
    mClusterCallback->OnMeasurementData(sessionId, measurement);
}

void ProximityRangingDriver::OnAttributeChanged(AttributeId attributeId)
{
    VerifyOrReturn(mClusterCallback != nullptr);
    mClusterCallback->OnAttributeChanged(attributeId);
}

RangingAdapter * ProximityRangingDriver::FindAdapter(RangingTechEnum technology) const
{
    for (size_t i = 0; i < mAdapters.size(); i++)
    {
        if (mAdapters[i]->GetTechnology() == technology)
        {
            return mAdapters[i];
        }
    }
    return nullptr;
}

ProximityRangingDriver::Session * ProximityRangingDriver::FindSession(uint8_t sessionId)
{
    Session * found = nullptr;
    mSessions.ForEachActiveObject([&](Session * s) {
        if (s->id == sessionId)
        {
            found = s;
            return Loop::Break;
        }
        return Loop::Continue;
    });
    return found;
}

void ProximityRangingDriver::RetireSession(uint8_t sessionId)
{
    Session * s = FindSession(sessionId);
    if (s == nullptr)
    {
        return;
    }
    mSessions.ReleaseObject(s);
    if (mClusterCallback != nullptr)
    {
        mClusterCallback->OnAttributeChanged(Attributes::SessionIDList::Id);
    }
}

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
