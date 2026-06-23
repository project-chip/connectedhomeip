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
#include <system/SystemClock.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

static_assert(kMaxConcurrentSessions >= 1, "ProximityRangingDriver must support at least 1 session");

namespace {

/// Translate a decoded StartRangingRequest into the narrowed parameter set
/// that adapters consume. ReportingCondition, trigger.startTime,
/// trigger.endTime, and trigger.rangingInstanceInterval are intentionally
/// excluded — the driver applies ReportingCondition itself and owns all
/// wall-clock scheduling (StartTime delay, EndTime cutoff, periodic ticks).
StartSessionParams BuildStartParams(const Commands::StartRangingRequest::DecodableType & request)
{
    StartSessionParams params;
    params.technology = request.technology;
    if (request.BLERangingDeviceRoleConfig.HasValue())
    {
        params.bleRoleConfig = request.BLERangingDeviceRoleConfig.Value();
    }
    if (request.wiFiRangingDeviceRoleConfig.HasValue())
    {
        params.wifiRoleConfig = request.wiFiRangingDeviceRoleConfig.Value();
    }
    if (request.BLTChannelSoundingDeviceRoleConfig.HasValue())
    {
        params.bltRoleConfig = request.BLTChannelSoundingDeviceRoleConfig.Value();
    }
    if (request.frequencyBand.HasValue())
    {
        params.frequencyBand = request.frequencyBand.Value();
    }
    if (request.bandwidth.HasValue())
    {
        params.bandwidth = request.bandwidth.Value();
    }
    return params;
}

System::Clock::Milliseconds32 SecondsToMilliseconds32(uint32_t seconds)
{
    return std::chrono::duration_cast<System::Clock::Milliseconds32>(System::Clock::Seconds32(seconds));
}

/// Returns true for the responder side of a ranging pair. Real radios in
/// these roles never produce ranging measurements; only the active
/// initiator does. The driver uses this to suppress its kPeerNotFound remap
/// for responder sessions, where the absence of measurements is the
/// expected outcome rather than a peer-not-found condition.
bool IsPassiveResponderRole(RangingRoleEnum role)
{
    switch (role)
    {
    case RangingRoleEnum::kBLEBeaconRole:
    case RangingRoleEnum::kWiFiPublisherRole:
    case RangingRoleEnum::kBLTReflectorRole:
        return true;
    default:
        return false;
    }
}

bool RequestIsPassiveResponder(const Commands::StartRangingRequest::DecodableType & request)
{
    if (request.BLERangingDeviceRoleConfig.HasValue())
    {
        return IsPassiveResponderRole(request.BLERangingDeviceRoleConfig.Value().role);
    }
    if (request.wiFiRangingDeviceRoleConfig.HasValue())
    {
        return IsPassiveResponderRole(request.wiFiRangingDeviceRoleConfig.Value().role);
    }
    if (request.BLTChannelSoundingDeviceRoleConfig.HasValue())
    {
        return IsPassiveResponderRole(request.BLTChannelSoundingDeviceRoleConfig.Value().role);
    }
    return false;
}

} // namespace

void ProximityRangingDriver::NextTriggerTimer::TimerFired()
{
    mSession.owner->OnNextTriggerFired(mSession);
}

void ProximityRangingDriver::EndTimer::TimerFired()
{
    mSession.owner->OnEndTimerFired(mSession);
}

ProximityRangingDriver::ProximityRangingDriver(Span<RangingAdapter * const> adapters, TimerDelegate & timerDelegate) :
    mAdapters(adapters), mTimerDelegate(timerDelegate)
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
    mSessions.ForEachActiveObject([this](Session * s) {
        CancelSessionTimers(*s);
        return Loop::Continue;
    });
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
    // ObjectPool::ForEachActiveObject permits ReleaseObject (via the adapter's
    // synchronous OnRangingSessionStopped → RetireSession path) on the current
    // element during iteration.
    mSessions.ForEachActiveObject([this](Session * s) {
        CancelSessionTimers(*s);
        LogErrorOnFailure(s->adapter->StopSession(s->id));
        return Loop::Continue;
    });

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

    // Phase 1: ask the adapter to validate and stage the session. Adapters are
    // required (see RangingAdapter.h) to defer any termination callback until
    // after a kAccepted return, so the session record can safely be committed
    // only after PrepareSession reports kAccepted.
    ResultCodeEnum result = adapter->PrepareSession(sessionId, BuildStartParams(request));
    VerifyOrReturnValue(result == ResultCodeEnum::kAccepted, result);

    Session * session = mSessions.CreateObject(sessionId, *adapter, *this);
    if (session == nullptr)
    {
        // Pool exhausted after the adapter already accepted: roll back the
        // adapter-side reservation so its bookkeeping does not diverge from
        // the driver's.
        LogErrorOnFailure(adapter->StopSession(sessionId));
        return ResultCodeEnum::kBusySessionCapacityReached;
    }

    if (request.reportingCondition.HasValue())
    {
        session->reporting = request.reportingCondition.Value();
    }
    if (request.trigger.rangingInstanceInterval.HasValue())
    {
        session->interval = SecondsToMilliseconds32(request.trigger.rangingInstanceInterval.Value());
    }
    session->isPassiveResponder = RequestIsPassiveResponder(request);

    // The sessionId becomes visible in SessionIDList from acceptance even if
    // its StartTime delay has not yet elapsed — the client received kAccepted
    // and is entitled to see the ID.
    if (mClusterCallback != nullptr)
    {
        mClusterCallback->OnAttributeChanged(Attributes::SessionIDList::Id);
    }

    // Phase 2: arm EndTime cutoff. trigger.endTime > trigger.startTime is
    // enforced by the cluster's preflight (see ProximityRangingCluster::
    // ValidateStartRangingRequest), so endTime is always strictly positive.
    LogErrorOnFailure(mTimerDelegate.StartTimer(&session->endTimer, SecondsToMilliseconds32(request.trigger.endTime)));

    // Phase 3: kick off the first measurement. With startTime == 0 we issue
    // it synchronously on the calling thread; otherwise we arm NextTrigger
    // for the StartTime delay and the timer's fire path will issue it.
    if (request.trigger.startTime == 0)
    {
        IssueStartSession(*session);
    }
    else
    {
        LogErrorOnFailure(mTimerDelegate.StartTimer(&session->nextTrigger, SecondsToMilliseconds32(request.trigger.startTime)));
    }

    return result;
}

CHIP_ERROR ProximityRangingDriver::HandleStopRanging(uint8_t sessionId)
{
    Session * session = FindSession(sessionId);
    VerifyOrReturnError(session != nullptr, CHIP_ERROR_NOT_FOUND);

    // Cancel pending start/periodic/end timers so they cannot fire after the
    // adapter has already torn the session down. Bookkeeping otherwise
    // happens in OnRangingSessionStopped, which adapters MUST invoke for any
    // session removal — including success of this call.
    CancelSessionTimers(*session);
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
    // For "normal end" terminations (kSessionEndTimeReached), only the driver
    // knows whether a measurement actually passed the ReportingCondition
    // filter. If none did, remap to kPeerNotFound. Other terminal statuses
    // (kHardwareError, kSecurityFailure, etc.) carry adapter-only context
    // and pass through verbatim.
    //
    // The remap is suppressed for passive-responder sessions (BLE beacon,
    // Wi-Fi publisher, BLT-CS reflector): real radios in these roles never
    // produce measurements, so the absence of a measurement is the expected
    // outcome rather than a peer-not-found condition.
    Session * s = FindSession(sessionId);
    if (s != nullptr)
    {
        if (status == RangingSessionStatusEnum::kSessionEndTimeReached && !s->peerFound && !s->isPassiveResponder)
        {
            status = RangingSessionStatusEnum::kPeerNotFound;
        }
        // Cancel both timers; the session is going away and we must not let
        // a pending start/end timer fire against a freed pool slot.
        CancelSessionTimers(*s);
    }

    // Drop spurious or stale adapter notifications for sessions the driver no
    // longer tracks (e.g. duplicate stop, post-Shutdown delivery).
    // RetireSession dirties SessionIDList as part of releasing the pool slot,
    // and is a no-op if the session is already gone.
    if (!RetireSession(sessionId))
    {
        return;
    }
    VerifyOrReturn(mClusterCallback != nullptr);
    mClusterCallback->OnSessionStopped(sessionId, status);
}

void ProximityRangingDriver::OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement)
{
    Session * s = FindSession(sessionId);
    VerifyOrReturn(s != nullptr);

    const bool isInstantRanging = !s->interval.has_value();

    if (s->reporting.has_value() && !SatisfiesReporting(*s->reporting, measurement))
    {
        ChipLogProgress(Zcl, "[ProximityRangingDriver] sid=%u suppressing measurement (ReportingCondition not satisfied)",
                        sessionId);
        // Per the spec, instant ranging performs "a single ranging attempt
        // ... after which the ranging session is terminated." A filtered
        // measurement is the radio's one shot — the driver does NOT re-issue
        // StartSession. The session simply lingers until EndTime fires; the
        // existing peerFound=false -> kPeerNotFound remap surfaces the
        // spec-correct outcome to the client.
        return;
    }

    s->peerFound = true;
    if (mClusterCallback != nullptr)
    {
        mClusterCallback->OnMeasurementData(sessionId, measurement);
    }

    // Instant ranging terminates after the first measurement passes the
    // reporting filter. Cancel timers eagerly and ask the adapter to tear
    // down — the OnRangingSessionStopped callback will retire the session
    // and surface the RangingSessionStatus(SessionEndTimeReached) event.
    if (isInstantRanging)
    {
        CancelSessionTimers(*s);
        LogErrorOnFailure(s->adapter->StopSession(sessionId));
    }
}

bool ProximityRangingDriver::SatisfiesReporting(const Structs::ReportingConditionStruct::Type & reporting,
                                                const Structs::RangingMeasurementDataStruct::Type & measurement)
{
    if (!measurement.distance.IsNull())
    {
        const uint16_t distance = measurement.distance.Value();
        if (reporting.minDistanceCondition.HasValue() && distance < reporting.minDistanceCondition.Value())
        {
            return false;
        }
        if (reporting.maxDistanceCondition.HasValue() && distance > reporting.maxDistanceCondition.Value())
        {
            return false;
        }
    }
    else if (reporting.minDistanceCondition.HasValue() || reporting.maxDistanceCondition.HasValue())
    {
        // Spec: "If Distance is null, the Server SHOULD still emit the
        // RangingResult event unless prohibited by a ReportingCondition."
        // A min/max distance condition cannot be evaluated against a null
        // distance, so suppress when any distance condition is present.
        return false;
    }
    if (reporting.errorMarginCondition.HasValue() && measurement.errorMargin.HasValue() &&
        measurement.errorMargin.Value() > reporting.errorMarginCondition.Value())
    {
        return false;
    }
    return true;
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

void ProximityRangingDriver::CancelSessionTimers(Session & session)
{
    mTimerDelegate.CancelTimer(&session.nextTrigger);
    mTimerDelegate.CancelTimer(&session.endTimer);
}

bool ProximityRangingDriver::RetireSession(uint8_t sessionId)
{
    Session * s = FindSession(sessionId);
    if (s == nullptr)
    {
        return false;
    }
    mSessions.ReleaseObject(s);
    if (mClusterCallback != nullptr)
    {
        mClusterCallback->OnAttributeChanged(Attributes::SessionIDList::Id);
    }
    return true;
}

void ProximityRangingDriver::IssueStartSession(Session & session)
{
    // CHIP_ERROR_BUSY signals the previous tick's measurement is still in
    // flight; the next periodic tick (or instant-ranging filter retry) will
    // call again. Other errors are logged but do not stop the session — the
    // adapter is contractually required to surface terminal failures via
    // OnRangingSessionStopped(kHardwareError).
    CHIP_ERROR err = session.adapter->StartSession(session.id);
    if (err != CHIP_NO_ERROR && err != CHIP_ERROR_BUSY)
    {
        ChipLogError(Zcl, "[ProximityRangingDriver] sid=%u StartSession returned %" CHIP_ERROR_FORMAT, session.id, err.Format());
    }
    if (err == CHIP_ERROR_BUSY)
    {
        ChipLogProgress(Zcl, "[ProximityRangingDriver] sid=%u StartSession busy; skipping this tick", session.id);
    }

    // Re-arm NextTrigger for the next periodic tick. Cadence is anchored
    // from the moment we issued StartSession so the spacing between ticks
    // is exactly `interval` regardless of measurement latency.
    if (session.interval.has_value())
    {
        LogErrorOnFailure(mTimerDelegate.StartTimer(&session.nextTrigger, *session.interval));
    }
}

void ProximityRangingDriver::OnNextTriggerFired(Session & session)
{
    // The TimerDelegate guarantees the timer cannot fire after CancelTimer
    // returns, so reaching here means the session is still live.
    IssueStartSession(session);
}

void ProximityRangingDriver::OnEndTimerFired(Session & session)
{
    // Capture the id before invoking StopSession, since the adapter's
    // OnRangingSessionStopped path may release the session pool slot before
    // StopSession returns.
    const uint8_t sessionId = session.id;
    // Cancel NextTrigger first so a pending periodic tick cannot fire after
    // the adapter has acknowledged the stop.
    mTimerDelegate.CancelTimer(&session.nextTrigger);
    CHIP_ERROR err = session.adapter->StopSession(sessionId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "[ProximityRangingDriver] sid=%u end-time StopSession returned %" CHIP_ERROR_FORMAT, sessionId,
                     err.Format());
    }
}

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
