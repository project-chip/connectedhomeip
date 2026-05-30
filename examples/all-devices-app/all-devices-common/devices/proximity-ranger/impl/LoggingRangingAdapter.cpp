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

#include <app/clusters/proximity-ranging-server/BleRssiRangingHelpers.h>
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
        return "WiFiRoundTripTime";
    case RangingTechEnum::kWiFiNextGenerationRanging:
        return "WiFiNextGeneration";
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
    case RangingTechEnum::kWiFiNextGenerationRanging:
    case RangingTechEnum::kWiFiRoundTripTimeRanging:
        return "WFUSDPD";
    default:
        return "Unknown";
    }
}

// GetCapabilities populates the RangingCapabilitiesStruct surfaced via the
// cluster's RangingCapabilities attribute. The cluster server reads this
// once per Init and again any time OnAttributeChanged(RangingCapabilities)
// is invoked, so the value should reflect the radio's true capabilities.
//
// REAL ADAPTER: technology is fixed at construction (one adapter, one tech).
// frequencyBand and periodicRangingSupport must match what the radio actually
// supports — query the radio driver rather than hard-coding.
Structs::RangingCapabilitiesStruct::Type LoggingRangingAdapter::GetCapabilities() const
{
    Structs::RangingCapabilitiesStruct::Type capabilities = {};
    capabilities.technology                               = mTechnology;
    capabilities.periodicRangingSupport                   = mPeriodicRangingSupport;
    switch (mTechnology)
    {
    case RangingTechEnum::kWiFiNextGenerationRanging:
    case RangingTechEnum::kWiFiRoundTripTimeRanging:
        // Both Wi-Fi USD techs operate across 2.4/5/6 GHz; the actual set is
        // platform-specific.
        capabilities.frequencyBand = BitMask<RadioBandBitmap>(RadioBandBitmap::k2g4, RadioBandBitmap::k5g, RadioBandBitmap::k6g);
        break;
    case RangingTechEnum::kBLEBeaconRSSIRanging:
    case RangingTechEnum::kBluetoothChannelSounding:
    default:
        // BLE-based techs are 2.4 GHz only.
        capabilities.frequencyBand = BitMask<RadioBandBitmap>(RadioBandBitmap::k2g4);
        break;
    }
    return capabilities;
}

// GetDeviceId / GetWiFiUsdConfig / GetBltcsConfig back the cluster's optional
// per-technology attributes (BLEDeviceID, WiFiDevIK, BLTDevIK, etc.).
// Returning std::nullopt makes the cluster surface UnsupportedAttribute.
//
// REAL ADAPTER: each Get* method MUST return Some(...) only on the adapter
// whose technology is the one the attribute belongs to, and the value must
// be the device's persisted, CSPRNG-derived identifier — not a constant.

// Constructor: binds the adapter to a single technology and, for BLE Beacon
// RSSI ranging, loads (or generates and persists) the local BLEDeviceID.
// Non-BLE-RSSI technologies skip the storage path entirely; their `storage`
// argument is unused and may be nullptr.
//
// REAL ADAPTER: equivalent setup is required — load the device's persisted
// local identity (BLEDeviceID, WiFiDevIK, BLTDevIK), and on first boot
// generate via CSPRNG and persist. The BleRssi helper functions in
// app/clusters/proximity-ranging-server/BleRssiRangingHelpers.h provide a
// CSPRNG-based generator for the BLEDeviceID. Storage failures here are
// fatal — without a stable identity, beacons cannot be correlated by peers.
LoggingRangingAdapter::LoggingRangingAdapter(RangingTechEnum technology, TimerDelegate & timerDelegate,
                                             PersistentStorageDelegate * storage, bool periodicRangingSupport) :
    mPeriodicRangingSupport(periodicRangingSupport),
    mTechnology(technology), mTimerDelegate(timerDelegate), mpStore(storage)
{
    if (mTechnology == RangingTechEnum::kBLEBeaconRSSIRanging)
    {
        VerifyOrDie(mpStore != nullptr);
        VerifyOrDie(BleRssi::RetrieveGenerateBleDeviceId(*mpStore, mBleDeviceId) == CHIP_NO_ERROR);
        ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s] Generated and persisted new BLEDeviceID", LogTag());
    }
}

std::optional<uint64_t> LoggingRangingAdapter::GetDeviceId()
{
    // BLE Device ID is only defined for BLE Beacon RSSI ranging; other
    // technologies have no equivalent attribute and should return nullopt.
    // mBleDeviceId is populated by Init() before the adapter is registered.
    if (mTechnology == RangingTechEnum::kBLEBeaconRSSIRanging)
    {
        return mBleDeviceId;
    }
    return std::nullopt;
}

std::optional<WiFiUsdConfig> LoggingRangingAdapter::GetWiFiUsdConfig()
{
    // WiFiDevIK is shared across both Wi-Fi USD ranging technologies
    // (RoundTripTime and NextGeneration), so both branches surface the same
    // local key. A real adapter still keeps a single per-device Wi-Fi USD
    // identity even when both techs are simultaneously supported.
    if (mTechnology != RangingTechEnum::kWiFiRoundTripTimeRanging && mTechnology != RangingTechEnum::kWiFiNextGenerationRanging)
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
    // REAL ADAPTER: securityLevel and modeCapability come from the
    // controller's BLTCS feature set. Hard-coding "level three / both" keeps
    // cert tests deterministic but is not what production reports.
    config.securityLevel  = BLTCSSecurityLevelEnum::kBLTCSSecurityLevelThree;
    config.modeCapability = BLTCSModeEnum::kBoth;
    return config;
}

// Destruction must terminate every active session — see RangingAdapter.h's
// "every session the adapter has accepted MUST be reported via
// OnRangingSessionStopped" contract. StopAllSessions() walks the list and
// notifies the cluster for each entry; without it, the cluster server's
// SessionIDList would retain stale IDs after the adapter disappeared.
LoggingRangingAdapter::~LoggingRangingAdapter()
{
    StopAllSessions();
}

// StartSession is the synchronous entry point invoked by the cluster server
// when it has accepted a StartRangingRequest command. The adapter must:
//   - return kAccepted only if the start succeeded (or is plausibly
//     in-flight, with HardwareError to follow asynchronously);
//   - reject infeasible requests synchronously with the appropriate
//     ResultCodeEnum (e.g. kRejectedInfeasibleRangingTriggers,
//     kRejectedInUse, kRejectedNoMemory);
//   - NOT deliver OnRangingSessionStopped synchronously from inside this
//     function — the cluster's bookkeeping isn't ready for it yet.
//
// REAL ADAPTER: the bulk of this function would translate `request` into a
// radio-driver call (set scan filter, configure publisher, arm CS measurement
// engine, etc.) and arm only the EndTime cutoff timer. Measurement cadence
// itself is driven by the radio.
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

    // Spec: endTime MUST be strictly greater than startTime. Detecting this
    // here keeps the rejection path symmetric with how a real adapter would
    // surface a radio that refuses the request — synchronous, before any
    // async machinery starts.
    if (request.trigger.endTime <= request.trigger.startTime)
    {
        ChipLogError(NotSpecified, "[LoggingRangingAdapter:%s] StartSession rejected: endTime <= startTime", LogTag());
        return ResultCodeEnum::kRejectedInfeasibleRangingTriggers;
    }

    // REAL ADAPTER: a hardware adapter should also reject here on conditions
    // its radio cannot satisfy — e.g. kRejectedInUse if a conflicting session
    // already holds the radio, kRejectedSecurityNotSupported if the requested
    // securityMode isn't available, kRejectedNoMemory on resource exhaustion.
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

// StopSession is invoked by the cluster when a client issues StopRanging.
// Returns CHIP_ERROR_NOT_FOUND if the ID is not active so the cluster can
// reflect that to the client. The adapter is required to deliver
// OnRangingSessionStopped for every session it accepted via StartSession,
// even when the termination is client-initiated.
//
// REAL ADAPTER: tear down the radio session here (cancel scan, release
// publisher, abort CS measurement, etc.). If the radio's stop is async,
// return CHIP_NO_ERROR and emit OnRangingSessionStopped only once the radio
// has confirmed teardown.
CHIP_ERROR LoggingRangingAdapter::StopSession(uint8_t sessionId)
{
    ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s] StopSession id=%u", LogTag(), sessionId);
    auto it = std::find_if(mSessions.begin(), mSessions.end(),
                           [sessionId](const std::unique_ptr<Session> & s) { return s->sessionId == sessionId; });
    VerifyOrReturnError(it != mSessions.end(), CHIP_ERROR_NOT_FOUND);

    mTimerDelegate.CancelTimer(it->get());

    // Erase the session before invoking the callback so any synchronous
    // query of the active session list during the callback observes a
    // consistent state (the stopped session is no longer reported active).
    mSessions.erase(it);

    // Adapters are required to deliver OnRangingSessionStopped for every session
    // termination. Client-initiated stop is reported as SessionEndTimeReached.
    if (mCallback != nullptr)
    {
        mCallback->OnRangingSessionStopped(sessionId, RangingSessionStatusEnum::kSessionEndTimeReached);
    }
    return CHIP_NO_ERROR;
}

// StopAllSessions is invoked on shutdown and during destruction. Order
// matters: cancel the timer first to prevent re-entry from a fired timer,
// then notify the cluster via the callback, then erase the session record.
//
// REAL ADAPTER: do the equivalent radio teardown for each session. If the
// radio supports a single "stop everything" call, prefer it over per-session
// stops to keep shutdown fast.
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

// GetActiveSessionIds appends the adapter's currently-active session IDs into
// the caller-supplied span. The cluster aggregates results across all
// registered adapters to populate the SessionIDList attribute.
//
// REAL ADAPTER: the implementation is the same shape as below — iterate the
// adapter's session bookkeeping and write each active ID into the span.
CHIP_ERROR LoggingRangingAdapter::GetActiveSessionIds(Span<uint8_t> & sessionIds)
{
    VerifyOrReturnError(sessionIds.size() >= mSessions.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    size_t i = 0;
    for (const auto & session : mSessions)
    {
        sessionIds[i++] = session->sessionId;
    }
    sessionIds.reduce_size(i);
    return CHIP_NO_ERROR;
}

// Session::TimerFired is the synthesizer's tick. It plays three distinct
// roles depending on session state:
//   1. EndTime reached → terminate with SessionEndTimeReached.
//   2. Instant ranging post-measurement tick → terminate (the deferred fire
//      from kInstantRangingTerminateDelay).
//   3. Otherwise → emit a measurement, advance to the next tick.
//
// REAL ADAPTER: this entire function is replaced by callbacks from the
// radio. The radio reports each measurement directly into
// OnMeasurementData(sessionId, measurement); the adapter only needs a
// timer-driven path for the EndTime cutoff (case 1) and even that often
// folds into the radio's own session-management layer.
void LoggingRangingAdapter::Session::TimerFired()
{
    auto now = owner.mTimerDelegate.GetCurrentMonotonicTimestamp();

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

// Common termination path used by EndTime expiry, instant-ranging deferred
// terminate, and any internal "stop this session now" reason. Cancels the
// outstanding timer, notifies the cluster, and removes the session record.
//
// REAL ADAPTER: keep the same shape but replace the timer-cancel with the
// radio's per-session teardown call. `status` should reflect the actual
// reason the session ended — SessionEndTimeReached for natural expiry,
// HardwareError for radio failure, etc.
void LoggingRangingAdapter::TerminateSession(Session & session, RangingSessionStatusEnum status)
{
    uint8_t sessionId = session.sessionId;
    mTimerDelegate.CancelTimer(&session);

    auto it = std::find_if(mSessions.begin(), mSessions.end(),
                           [sessionId](const std::unique_ptr<Session> & s) { return s->sessionId == sessionId; });

    // Erase the session before invoking the callback so any synchronous
    // query of the active session list during the callback observes a
    // consistent state. After erase, the `session` reference is dangling
    // and MUST NOT be used; sessionId was captured as a local above.
    if (it != mSessions.end())
    {
        mSessions.erase(it);
    }

    if (mCallback != nullptr)
    {
        mCallback->OnRangingSessionStopped(sessionId, status);
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

    LogErrorOnFailure(mTimerDelegate.StartTimer(&session, delay));
}

bool LoggingRangingAdapter::SatisfiesReporting(const Structs::ReportingConditionStruct::Type & reporting,
                                               const Structs::RangingMeasurementDataStruct::Type & measurement)
{
    if (!measurement.distance.IsNull())
    {
        uint16_t distance = measurement.distance.Value();
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

// BuildMeasurement constructs a synthetic RangingMeasurementDataStruct with
// deterministic defaults plus the technology-specific peer identity captured
// at StartSession time.
//
// REAL ADAPTER: this function is replaced entirely. A hardware adapter
// translates the radio's measurement output into RangingMeasurementDataStruct:
//   - distance / errorMargin: from the radio's range estimate.
//   - rssi / txPower: present for kBLEBeaconRSSIRanging; supplied by the
//     radio.
//   - BLEDeviceID / wiFiDevIK / BLTDevIK: identifies the *peer* the
//     measurement is for, taken from the radio's session metadata. Only the
//     field matching this adapter's technology is set.
//   - Any spec-required null handling (e.g. distance=null when the radio
//     could not converge) is preserved.
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
        // adapter's own (persisted) ID only if the request omitted the role
        // config.
        measurement.BLEDeviceID.SetValue(session.peerBleDeviceId.value_or(mBleDeviceId));
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
    case RangingTechEnum::kWiFiNextGenerationRanging:
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
