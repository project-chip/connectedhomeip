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

#include "LoggingRangingAdapter.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>
#include <cstring>
#include <inttypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

namespace {

constexpr const char * RoleName(RangingRoleEnum role)
{
    switch (role)
    {
    case RangingRoleEnum::kWiFiSubscriberRole:
        return "WiFiSubscriber";
    case RangingRoleEnum::kWiFiPublisherRole:
        return "WiFiPublisher";
    case RangingRoleEnum::kBLEScanningRole:
        return "BLEScanning";
    case RangingRoleEnum::kBLEBeaconRole:
        return "BLEBeacon";
    case RangingRoleEnum::kBLTInitiatorRole:
        return "BLTInitiator";
    case RangingRoleEnum::kBLTReflectorRole:
        return "BLTReflector";
    default:
        return "Unknown";
    }
}

constexpr const char * SecurityName(RangingSecurityEnum mode)
{
    switch (mode)
    {
    case RangingSecurityEnum::kSecureRanging:
        return "SecureRanging";
    case RangingSecurityEnum::kOpenRanging:
        return "OpenRanging";
    default:
        return "Unknown";
    }
}

constexpr const char * TechName(RangingTechEnum tech)
{
    switch (tech)
    {
    case RangingTechEnum::kBluetoothChannelSounding:
        return "BluetoothChannelSounding";
    case RangingTechEnum::kWiFiRoundTripTimeRanging:
        return "WiFiRTT";
    case RangingTechEnum::kWiFiNextGenerationRanging:
        return "WiFiNGR";
    case RangingTechEnum::kBLEBeaconRSSIRanging:
        return "BLEBeaconRSSI";
    default:
        return "Unknown";
    }
}

} // namespace

const char * LoggingRangingAdapter::LogTag() const
{
    switch (mTechnology)
    {
    case RangingTechEnum::kBLEBeaconRSSIRanging:
        return "BLERBC";
    case RangingTechEnum::kBluetoothChannelSounding:
        return "BLTCS";
    case RangingTechEnum::kWiFiRoundTripTimeRanging:
        return "WFUSDPD";
    default:
        return "Unknown";
    }
}

Structs::RangingCapabilitiesStruct::Type LoggingRangingAdapter::GetCapabilities() const
{
    Structs::RangingCapabilitiesStruct::Type capabilities = {};
    capabilities.technology                               = mTechnology;
    capabilities.periodicRangingSupport                   = true;
    switch (mTechnology)
    {
    case RangingTechEnum::kWiFiRoundTripTimeRanging:
        capabilities.frequencyBand = BitMask<RadioBandBitmap>(RadioBandBitmap::k2g4, RadioBandBitmap::k5g, RadioBandBitmap::k6g);
        break;
    case RangingTechEnum::kBLEBeaconRSSIRanging:
    case RangingTechEnum::kBluetoothChannelSounding:
    default:
        capabilities.frequencyBand = BitMask<RadioBandBitmap>(RadioBandBitmap::k2g4);
        break;
    }
    return capabilities;
}

std::optional<uint64_t> LoggingRangingAdapter::GetDeviceId()
{
    if (mTechnology == RangingTechEnum::kBLEBeaconRSSIRanging)
    {
        return kBleDeviceId;
    }
    return std::nullopt;
}

std::optional<WiFiUsdConfig> LoggingRangingAdapter::GetWiFiUsdConfig()
{
    if (mTechnology != RangingTechEnum::kWiFiRoundTripTimeRanging)
    {
        return std::nullopt;
    }
    WiFiUsdConfig config{};
    memcpy(config.deviceIdentityKey, kWiFiDevIK, kDeviceIdentityKeyLen);
    return config;
}

std::optional<BltcsConfig> LoggingRangingAdapter::GetBltcsConfig()
{
    if (mTechnology != RangingTechEnum::kBluetoothChannelSounding)
    {
        return std::nullopt;
    }
    BltcsConfig config{};
    memcpy(config.deviceIdentityKey, kBltDevIK, kDeviceIdentityKeyLen);
    config.securityLevel  = BLTCSSecurityLevelEnum::kBLTCSSecurityLevelThree;
    config.modeCapability = BLTCSModeEnum::kBoth;
    return config;
}

LoggingRangingAdapter::~LoggingRangingAdapter()
{
    StopAllSessions();
}

ResultCodeEnum LoggingRangingAdapter::StartSession(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request)
{
    if (request.trigger.rangingInstanceInterval.HasValue())
    {
        ChipLogProgress(NotSpecified,
                        "[LoggingRangingAdapter:%s] StartSession id=%u tech=%s securityMode=%s "
                        "startTime=%us endTime=%us interval=%us reporting=%s",
                        LogTag(), sessionId, TechName(request.technology), SecurityName(request.securityMode),
                        request.trigger.startTime, request.trigger.endTime, request.trigger.rangingInstanceInterval.Value(),
                        request.reportingCondition.HasValue() ? "present" : "absent");
    }
    else
    {
        ChipLogProgress(NotSpecified,
                        "[LoggingRangingAdapter:%s] StartSession id=%u tech=%s securityMode=%s "
                        "startTime=%us endTime=%us interval=absent reporting=%s",
                        LogTag(), sessionId, TechName(request.technology), SecurityName(request.securityMode),
                        request.trigger.startTime, request.trigger.endTime,
                        request.reportingCondition.HasValue() ? "present" : "absent");
    }

    if (request.wiFiRangingDeviceRoleConfig.HasValue())
    {
        const auto & cfg = request.wiFiRangingDeviceRoleConfig.Value();
        ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s]   WiFiRoleConfig role=%s peerWiFiDevIK.size=%u pmk=%s", LogTag(),
                        RoleName(cfg.role), static_cast<unsigned>(cfg.peerWiFiDevIK.size()),
                        cfg.pmk.HasValue() ? "present" : "absent");
    }
    if (request.BLERangingDeviceRoleConfig.HasValue())
    {
        const auto & cfg = request.BLERangingDeviceRoleConfig.Value();
        ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s]   BLERoleConfig role=%s peerBLEDeviceID=0x%016" PRIx64, LogTag(),
                        RoleName(cfg.role), cfg.peerBLEDeviceID);
    }
    if (request.BLTChannelSoundingDeviceRoleConfig.HasValue())
    {
        const auto & cfg = request.BLTChannelSoundingDeviceRoleConfig.Value();
        ChipLogProgress(NotSpecified,
                        "[LoggingRangingAdapter:%s]   BLTRoleConfig role=%s peerBLTDevIK.size=%u "
                        "BLTCSMode=%s BLTCSSecurityLevel=%s ltk=%s",
                        LogTag(), RoleName(cfg.role), static_cast<unsigned>(cfg.peerBLTDevIK.size()),
                        cfg.BLTCSMode.HasValue() ? "present" : "absent", cfg.BLTCSSecurityLevel.HasValue() ? "present" : "absent",
                        cfg.ltk.HasValue() ? "present" : "absent");
    }

    if (request.trigger.endTime <= request.trigger.startTime)
    {
        ChipLogError(NotSpecified, "[LoggingRangingAdapter:%s] StartSession rejected: endTime <= startTime", LogTag());
        return ResultCodeEnum::kRejectedInfeasibleRangingTriggers;
    }

    auto session = std::make_unique<Session>(*this, sessionId);
    auto now     = mTimerDelegate.GetCurrentMonotonicTimestamp();
    session->startAt =
        now + std::chrono::duration_cast<System::Clock::Milliseconds64>(System::Clock::Seconds32(request.trigger.startTime));
    session->endAt =
        now + std::chrono::duration_cast<System::Clock::Milliseconds64>(System::Clock::Seconds32(request.trigger.endTime));
    session->interval     = request.trigger.rangingInstanceInterval.HasValue()
            ? std::chrono::duration_cast<System::Clock::Milliseconds32>(
              System::Clock::Seconds32(request.trigger.rangingInstanceInterval.Value()))
            : System::Clock::Milliseconds32(0);
    session->hasReporting = request.reportingCondition.HasValue();
    if (session->hasReporting)
    {
        session->reporting = request.reportingCondition.Value();
    }

    // Capture peer identity from the role config matching this adapter's
    // technology; surfaced later in BuildMeasurement so the synthesized
    // RangingResult reflects the peer this SessionID is bound to.
    if (request.BLERangingDeviceRoleConfig.HasValue())
    {
        session->peerBleDeviceId = request.BLERangingDeviceRoleConfig.Value().peerBLEDeviceID;
    }
    if (request.wiFiRangingDeviceRoleConfig.HasValue())
    {
        const auto & ik = request.wiFiRangingDeviceRoleConfig.Value().peerWiFiDevIK;
        if (ik.size() == kDeviceIdentityKeyLen)
        {
            std::array<uint8_t, kDeviceIdentityKeyLen> key{};
            memcpy(key.data(), ik.data(), kDeviceIdentityKeyLen);
            session->peerWiFiDevIK = key;
        }
    }
    if (request.BLTChannelSoundingDeviceRoleConfig.HasValue())
    {
        const auto & ik = request.BLTChannelSoundingDeviceRoleConfig.Value().peerBLTDevIK;
        if (ik.size() == kDeviceIdentityKeyLen)
        {
            std::array<uint8_t, kDeviceIdentityKeyLen> key{};
            memcpy(key.data(), ik.data(), kDeviceIdentityKeyLen);
            session->peerBltDevIK = key;
        }
    }

    Session * sessionPtr = session.get();
    mSessions.push_back(std::move(session));
    ScheduleNextFire(*sessionPtr);
    return ResultCodeEnum::kAccepted;
}

CHIP_ERROR LoggingRangingAdapter::StopSession(uint8_t sessionId)
{
    ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s] StopSession id=%u", LogTag(), sessionId);
    auto it = std::find_if(mSessions.begin(), mSessions.end(),
                           [sessionId](const std::unique_ptr<Session> & s) { return s->sessionId == sessionId; });
    VerifyOrReturnError(it != mSessions.end(), CHIP_ERROR_NOT_FOUND);

    // Cancel any pending fire before unwinding the session.
    mTimerDelegate.CancelTimer(it->get());

    // Adapters are required to deliver OnRangingSessionStopped for every session
    // termination. The controller treats client-initiated stop as a
    // SessionEndTimeReached completion.
    Session & session = **it;
    if (mCallback != nullptr)
    {
        mCallback->OnRangingSessionStopped(session.sessionId, RangingSessionStatusEnum::kSessionEndTimeReached);
    }
    mSessions.erase(it);
    return CHIP_NO_ERROR;
}

void LoggingRangingAdapter::StopAllSessions()
{
    while (!mSessions.empty())
    {
        Session & session = *mSessions.back();
        mTimerDelegate.CancelTimer(&session);
        if (mCallback != nullptr)
        {
            mCallback->OnRangingSessionStopped(session.sessionId, RangingSessionStatusEnum::kSessionEndTimeReached);
        }
        mSessions.pop_back();
    }
}

CHIP_ERROR LoggingRangingAdapter::GetActiveSessionIds(std::vector<uint8_t> & sessionIds)
{
    sessionIds.clear();
    sessionIds.reserve(mSessions.size());
    for (const auto & session : mSessions)
    {
        sessionIds.push_back(session->sessionId);
    }
    return CHIP_NO_ERROR;
}

void LoggingRangingAdapter::Session::TimerFired()
{
    auto now = owner.mTimerDelegate.GetCurrentMonotonicTimestamp();

    // EndTime first: if reached, terminate without emitting another result.
    if (now >= endAt)
    {
        owner.TerminateSession(*this, RangingSessionStatusEnum::kSessionEndTimeReached);
        return;
    }

    // For instant ranging, the post-measurement fire is the deferred terminate.
    // Splitting termination into a separate timer event lets the SessionIDList
    // "session added" notification reach subscribers before the "session
    // removed" notification — otherwise the report engine coalesces both
    // MarkDirty calls into a single report carrying the final empty state.
    if (firstFired && interval == System::Clock::Milliseconds32(0))
    {
        owner.TerminateSession(*this, RangingSessionStatusEnum::kSessionEndTimeReached);
        return;
    }

    // Build and (conditionally) emit a measurement.
    auto measurement = owner.BuildMeasurement(*this);
    bool emit        = !hasReporting || SatisfiesReporting(reporting, measurement);
    if (emit && owner.mCallback != nullptr)
    {
        owner.mCallback->OnMeasurementData(sessionId, measurement);
    }
    else if (!emit)
    {
        ChipLogProgress(NotSpecified,
                        "[LoggingRangingAdapter:%s] sid=%u suppressing measurement (ReportingCondition not satisfied)",
                        owner.LogTag(), sessionId);
    }

    firstFired = true;
    owner.ScheduleNextFire(*this);
}

void LoggingRangingAdapter::TerminateSession(Session & session, RangingSessionStatusEnum status)
{
    uint8_t sessionId = session.sessionId;
    mTimerDelegate.CancelTimer(&session);

    auto it = std::find_if(mSessions.begin(), mSessions.end(),
                           [sessionId](const std::unique_ptr<Session> & s) { return s->sessionId == sessionId; });

    if (mCallback != nullptr)
    {
        mCallback->OnRangingSessionStopped(sessionId, status);
    }
    if (it != mSessions.end())
    {
        mSessions.erase(it);
    }
}

void LoggingRangingAdapter::ScheduleNextFire(Session & session)
{
    auto now = mTimerDelegate.GetCurrentMonotonicTimestamp();

    System::Clock::Timestamp nextAt;
    if (!session.firstFired)
    {
        nextAt = session.startAt;
    }
    else if (session.interval == System::Clock::Milliseconds32(0))
    {
        // Instant ranging: defer the post-measurement terminate by a small but
        // observable gap so subscriptions to SessionIDList see the session
        // appear and disappear as two reports rather than coalescing them
        // into one report carrying the final empty state.
        nextAt = now + kInstantRangingTerminateDelay;
    }
    else
    {
        nextAt = now + session.interval;
    }
    if (nextAt > session.endAt)
    {
        nextAt = session.endAt;
    }

    System::Clock::Milliseconds32 delay =
        (nextAt > now) ? std::chrono::duration_cast<System::Clock::Milliseconds32>(nextAt - now) : System::Clock::Milliseconds32(0);

    auto failure      = mTimerDelegate.StartTimer(&session, delay);
    bool startedTimer = failure.Handle([this, &session](CHIP_ERROR err) {
        ChipLogError(NotSpecified, "[LoggingRangingAdapter:%s] StartTimer failed for sid=%u: %" CHIP_ERROR_FORMAT, LogTag(),
                     session.sessionId, err.Format());
    });
    (void) startedTimer;
}

bool LoggingRangingAdapter::SatisfiesReporting(const Structs::ReportingConditionStruct::Type & reporting,
                                               const Structs::RangingMeasurementDataStruct::Type & measurement)
{
    if (measurement.distance.IsNull())
    {
        // Spec: "If Distance is null, the Server SHOULD still emit the
        // RangingResult event unless prohibited by a ReportingCondition."
        // A min/max distance condition cannot be evaluated against a null
        // distance, so suppress when any distance condition is present.
        return !reporting.minDistanceCondition.HasValue() && !reporting.maxDistanceCondition.HasValue();
    }
    uint16_t distance = measurement.distance.Value();
    if (reporting.minDistanceCondition.HasValue() && distance < reporting.minDistanceCondition.Value())
    {
        return false;
    }
    if (reporting.maxDistanceCondition.HasValue() && distance > reporting.maxDistanceCondition.Value())
    {
        return false;
    }
    if (reporting.errorMarginCondition.HasValue() && measurement.errorMargin.HasValue() &&
        measurement.errorMargin.Value() > reporting.errorMarginCondition.Value())
    {
        return false;
    }
    return true;
}

Structs::RangingMeasurementDataStruct::Type LoggingRangingAdapter::BuildMeasurement(const Session & session) const
{
    Structs::RangingMeasurementDataStruct::Type measurement{};
    measurement.distance.SetNonNull(kDefaultDistanceCm);
    measurement.errorMargin.SetValue(kDefaultErrorMarginCm);
    switch (mTechnology)
    {
    case RangingTechEnum::kBLEBeaconRSSIRanging:
        // Reflect the peer the SessionID is bound to (StartRangingRequest's
        // BLERangingDeviceRoleConfig.peerBLEDeviceID); fall back to the
        // adapter's own ID only if the request omitted the role config.
        measurement.BLEDeviceID.SetValue(session.peerBleDeviceId.value_or(kBleDeviceId));
        // Spec: RSSI and TxPower SHALL be present when Technology == BLEBeaconRSSI.
        measurement.rssi.Emplace().SetNonNull(kDefaultRssiDbm);
        measurement.txPower.Emplace().SetNonNull(kDefaultTxPowerDbm);
        break;
    case RangingTechEnum::kBluetoothChannelSounding:
        if (session.peerBltDevIK.has_value())
        {
            measurement.BLTDevIK.SetValue(ByteSpan(session.peerBltDevIK->data(), kDeviceIdentityKeyLen));
        }
        else
        {
            measurement.BLTDevIK.SetValue(ByteSpan(kBltDevIK, kDeviceIdentityKeyLen));
        }
        break;
    case RangingTechEnum::kWiFiRoundTripTimeRanging:
        if (session.peerWiFiDevIK.has_value())
        {
            measurement.wiFiDevIK.SetValue(ByteSpan(session.peerWiFiDevIK->data(), kDeviceIdentityKeyLen));
        }
        else
        {
            measurement.wiFiDevIK.SetValue(ByteSpan(kWiFiDevIK, kDeviceIdentityKeyLen));
        }
        break;
    default:
        break;
    }
    return measurement;
}

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
