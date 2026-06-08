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
#include <crypto/CHIPCryptoPAL.h>
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

/// Synthetic fixed measurement defaults — chosen to satisfy a permissive
/// ReportingCondition, and deterministic so cert tests can assert them.
///
/// REAL ADAPTER: these constants do not exist in production. Distance,
/// error margin, RSSI and Tx power are reported from the radio's own
/// measurement output; values vary with the channel, peer, and environment.
constexpr uint16_t kDefaultDistanceCm    = 100;
constexpr uint16_t kDefaultErrorMarginCm = 10;
constexpr int8_t kDefaultRssiDbm         = -50;
constexpr int8_t kDefaultTxPowerDbm      = 0;

/// Sentinel "unknown peer" identity values used by cert tests to exercise
/// the kPeerNotFound termination path. When a StartRangingRequest arrives
/// carrying one of these as the peer identity for the matching technology,
/// the adapter accepts the session but suppresses every OnMeasurementData
/// emission; the session's peerFound flag therefore stays false, and the
/// EndTime cutoff terminates with kPeerNotFound instead of
/// kSessionEndTimeReached.
///
/// REAL ADAPTER: these sentinels do not exist in production. A hardware
/// adapter naturally produces no measurements when the requested peer is
/// not in range, so the cluster server's existing kPeerNotFound semantics
/// are reached without any peer-recognition shim. We need this only because
/// the stub adapter would otherwise emit a default measurement for every
/// session, regardless of the peer key carried in the request.
constexpr uint64_t kUnknownPeerBleDeviceId                     = 0xDEADBEEFCAFEBABEULL;
constexpr uint8_t kUnknownPeerWiFiDevIK[kDeviceIdentityKeyLen] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE, 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
};
constexpr uint8_t kUnknownPeerBltDevIK[kDeviceIdentityKeyLen] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE, 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
};

/// Returns true if the captured peer identity for `tech` matches the
/// "unknown peer" sentinel — i.e. the cluster client deliberately asked us
/// to range against a peer the (stub) radio cannot find.
bool IsUnknownPeerIdentity(RangingTechEnum tech, const std::optional<uint64_t> & peerBleDeviceId,
                           const std::optional<std::array<uint8_t, kDeviceIdentityKeyLen>> & peerWiFiDevIK,
                           const std::optional<std::array<uint8_t, kDeviceIdentityKeyLen>> & peerBltDevIK)
{
    switch (tech)
    {
    case RangingTechEnum::kBLEBeaconRSSIRanging:
        return peerBleDeviceId.has_value() && peerBleDeviceId.value() == kUnknownPeerBleDeviceId;
    case RangingTechEnum::kWiFiRoundTripTimeRanging:
    case RangingTechEnum::kWiFiNextGenerationRanging:
        return peerWiFiDevIK.has_value() && memcmp(peerWiFiDevIK->data(), kUnknownPeerWiFiDevIK, kDeviceIdentityKeyLen) == 0;
    case RangingTechEnum::kBluetoothChannelSounding:
        return peerBltDevIK.has_value() && memcmp(peerBltDevIK->data(), kUnknownPeerBltDevIK, kDeviceIdentityKeyLen) == 0;
    default:
        return false;
    }
}

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
// once during cluster initialisation and again any time
// OnAttributeChanged(RangingCapabilities) is invoked, so the value should
// reflect the radio's true capabilities.
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

// Constructor: binds the adapter to a single technology and, for BLE Beacon
// RSSI ranging, loads (or generates and persists) the local BLEDeviceID.
// Non-BLE-RSSI technologies skip the storage path entirely; their `storage`
// argument is unused and may be nullptr.
//
// REAL ADAPTER: equivalent setup is required — load the device's persisted
// local identity (BLEDeviceID, WiFiDevIK, BLTDevIK), and on first boot
// generate via CSPRNG and persist. The BleRssi helper functions in
// app/clusters/proximity-ranging-server/BleRssiRangingHelpers.h provide a
// CSPRNG-based generator for the BLEDeviceID.
LoggingRangingAdapter::LoggingRangingAdapter(RangingTechEnum technology, TimerDelegate & timerDelegate,
                                             PersistentStorageDelegate * storage, bool periodicRangingSupport) :
    mPeriodicRangingSupport(periodicRangingSupport),
    mTechnology(technology), mTimerDelegate(timerDelegate), mpStore(storage)
{
    if (mTechnology == RangingTechEnum::kBLEBeaconRSSIRanging)
    {
        // Storage failures are fatal here, without stable identity ranging sessions
        // cannot be correlated to beacons
        VerifyOrDie(mpStore != nullptr);
        VerifyOrDie(BleRssi::RetrieveGenerateBleDeviceId(*mpStore, mBleDeviceId) == CHIP_NO_ERROR);
        ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s] Generated and persisted new BLEDeviceID", LogTag());
    }

    // CSPRNG-fill the device identity key for the matching technology. These
    // keys are NOT persisted; they regenerate on every process start. A real
    // adapter must persist them across reboots — see header comment.
    switch (mTechnology)
    {
    case RangingTechEnum::kWiFiRoundTripTimeRanging:
    case RangingTechEnum::kWiFiNextGenerationRanging:
        VerifyOrDie(Crypto::DRBG_get_bytes(mWiFiDevIK, kDeviceIdentityKeyLen) == CHIP_NO_ERROR);
        ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s] Generated random WiFiDevIK", LogTag());
        break;
    case RangingTechEnum::kBluetoothChannelSounding:
        VerifyOrDie(Crypto::DRBG_get_bytes(mBltDevIK, kDeviceIdentityKeyLen) == CHIP_NO_ERROR);
        ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s] Generated random BLTDevIK", LogTag());
        break;
    default:
        break;
    }
}

// GetDeviceId / GetWiFiUsdConfig / GetBltcsConfig back the cluster's optional
// per-technology attributes (BLEDeviceID, WiFiDevIK, BLTDevIK, etc.).
// Returning std::nullopt makes the cluster surface UnsupportedAttribute.
//
// REAL ADAPTER: each Get* method MUST return Some(...) only on the adapter
// whose technology is the one the attribute belongs to, and the value must
// be the device's persisted, CSPRNG-derived identifier — not a constant.
std::optional<uint64_t> LoggingRangingAdapter::GetDeviceId()
{
    // BLE Device ID is only defined for BLE Beacon RSSI ranging; other
    // technologies have no equivalent attribute and should return nullopt.
    // mBleDeviceId is populated by the constructor before the adapter is
    // registered with the driver.
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
    memcpy(config.deviceIdentityKey, mWiFiDevIK, kDeviceIdentityKeyLen);
    return config;
}

std::optional<BltcsConfig> LoggingRangingAdapter::GetBltcsConfig()
{
    if (mTechnology != RangingTechEnum::kBluetoothChannelSounding)
    {
        return std::nullopt;
    }
    BltcsConfig config{};
    memcpy(config.deviceIdentityKey, mBltDevIK, kDeviceIdentityKeyLen);
    // REAL ADAPTER: securityLevel and modeCapability come from the
    // controller's BLTCS feature set. Hard-coding "level three / both" keeps
    // cert tests deterministic but is not what production reports.
    config.securityLevel  = BLTCSSecurityLevelEnum::kBLTCSSecurityLevelThree;
    config.modeCapability = BLTCSModeEnum::kBoth;
    return config;
}

// Destruction must terminate every active session — see RangingAdapter.h's
// "every session the adapter has accepted via PrepareSession MUST be reported
// via OnRangingSessionStopped" contract. StopAllSessions() walks the list and
// notifies the cluster for each entry; without it, the cluster server's
// SessionIDList would retain stale IDs after the adapter disappeared.
LoggingRangingAdapter::~LoggingRangingAdapter()
{
    StopAllSessions();
}

// PrepareSession is the synchronous entry point invoked by the driver when
// the cluster server has accepted a StartRangingRequest command. The adapter
// must:
//   - Determine whether the request is compatible with what the radio can do
//     and synchronously return kRejectedInfeasibleRanging (or an
//     adapter-specific ResultCodeEnum) when it is not. Capability checks
//     belong to the adapter, not the cluster, because only the adapter knows
//     the radio's true capabilities.
//   - Stage all per-session bookkeeping needed for the eventual StartSession
//     call. PrepareSession MUST NOT yet drive the radio — measurement
//     activity is gated on the driver-owned StartTime delay.
//   - Track every accepted session by its 1-byte sessionId so the matching
//     StartSession / StopSession calls (and any radio-driven termination)
//     can map back to the correct in-flight session. The adapter is
//     responsible for delivering OnRangingSessionStopped exactly once per
//     accepted session — whether termination is client-initiated, EndTime
//     expiry, or hardware failure — so the cluster's SessionIDList stays
//     consistent.
//   - NOT deliver OnRangingSessionStopped synchronously from inside this
//     function — the cluster's bookkeeping isn't ready for it yet.
//
// REAL ADAPTER: most of this function would translate `params` into the
// radio driver's session-staging API (allocate a session handle, configure
// the scan filter / publisher / CS engine without yet starting it, ...).
// Asynchronous validation that requires talking to the radio MAY return
// kAccepted here and reject in StartSession or via OnRangingSessionStopped,
// but doing the work synchronously is preferred where the radio supports it.
ResultCodeEnum LoggingRangingAdapter::PrepareSession(uint8_t sessionId, const StartSessionParams & params)
{
    if (params.rangingInstanceInterval.has_value())
    {
        ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s] PrepareSession id=%u tech=%s interval=%" PRIu32 "s", LogTag(),
                        sessionId, TechName(params.technology), params.rangingInstanceInterval.value());
    }
    else
    {
        ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s] PrepareSession id=%u tech=%s interval=absent", LogTag(),
                        sessionId, TechName(params.technology));
    }

    if (params.wifiRoleConfig.has_value())
    {
        const auto & cfg = *params.wifiRoleConfig;
        ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s]   WiFiRoleConfig role=%s peerWiFiDevIK.size=%u pmk=%s", LogTag(),
                        RoleName(cfg.role), static_cast<unsigned>(cfg.peerWiFiDevIK.size()),
                        cfg.pmk.HasValue() ? "present" : "absent");
    }
    if (params.bleRoleConfig.has_value())
    {
        const auto & cfg = *params.bleRoleConfig;
        ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s]   BLERoleConfig role=%s peerBLEDeviceID=0x%016" PRIx64, LogTag(),
                        RoleName(cfg.role), cfg.peerBLEDeviceID);
    }
    if (params.bltRoleConfig.has_value())
    {
        const auto & cfg = *params.bltRoleConfig;
        ChipLogProgress(NotSpecified,
                        "[LoggingRangingAdapter:%s]   BLTRoleConfig role=%s peerBLTDevIK.size=%u "
                        "BLTCSMode=%s BLTCSSecurityLevel=%s ltk=%s",
                        LogTag(), RoleName(cfg.role), static_cast<unsigned>(cfg.peerBLTDevIK.size()),
                        cfg.BLTCSMode.HasValue() ? "present" : "absent", cfg.BLTCSSecurityLevel.HasValue() ? "present" : "absent",
                        cfg.ltk.HasValue() ? "present" : "absent");
    }

    // REAL ADAPTER: a hardware adapter should reject here on conditions its
    // radio cannot satisfy — e.g. kRejectedInUse if a conflicting session
    // already holds the radio, kRejectedSecurityNotSupported if the
    // requested securityMode isn't available, kRejectedNoMemory on resource
    // exhaustion.
    auto session      = std::make_unique<Session>(*this, sessionId);
    session->interval = params.rangingInstanceInterval.has_value()
        ? std::chrono::duration_cast<System::Clock::Milliseconds32>(
              System::Clock::Seconds32(params.rangingInstanceInterval.value()))
        : System::Clock::Milliseconds32(0);

    // Capture peer identity from the role config matching this adapter's
    // technology; surfaced later in BuildMeasurement so the synthesized
    // RangingResult reflects the peer this SessionID is bound to.
    if (params.bleRoleConfig.has_value())
    {
        session->peerBleDeviceId = params.bleRoleConfig->peerBLEDeviceID;
    }
    if (params.wifiRoleConfig.has_value())
    {
        const auto & ik = params.wifiRoleConfig->peerWiFiDevIK;
        if (ik.size() == kDeviceIdentityKeyLen)
        {
            std::array<uint8_t, kDeviceIdentityKeyLen> key{};
            memcpy(key.data(), ik.data(), kDeviceIdentityKeyLen);
            session->peerWiFiDevIK = key;
        }
    }
    if (params.bltRoleConfig.has_value())
    {
        const auto & ik = params.bltRoleConfig->peerBLTDevIK;
        if (ik.size() == kDeviceIdentityKeyLen)
        {
            std::array<uint8_t, kDeviceIdentityKeyLen> key{};
            memcpy(key.data(), ik.data(), kDeviceIdentityKeyLen);
            session->peerBltDevIK = key;
        }
    }

    // One-time peer-identity sentinel check: if the request targets the
    // technology-specific "unknown peer" pattern, mark the session so
    // TimerFired never emits OnMeasurementData. peerFound stays false on the
    // driver side; the driver's EndTime cutoff then maps the session to
    // kPeerNotFound.
    session->isUnknownPeer =
        IsUnknownPeerIdentity(mTechnology, session->peerBleDeviceId, session->peerWiFiDevIK, session->peerBltDevIK);
    if (session->isUnknownPeer)
    {
        ChipLogProgress(NotSpecified,
                        "[LoggingRangingAdapter:%s] sid=%u peer identity matches kUnknownPeer sentinel; will not emit measurements",
                        LogTag(), sessionId);
    }

    mSessions.push_back(std::move(session));
    return ResultCodeEnum::kAccepted;
}

// StartSession is the second-phase entry point invoked by the driver after
// the StartTime delay has elapsed (or immediately when StartTime == 0). The
// adapter MUST:
//   - Begin radio activity using whatever it stashed in PrepareSession.
//   - Forward every measurement received from the radio into the callback's
//     OnMeasurementData with a fully populated RangingMeasurementDataStruct.
//   - Report any asynchronous failure via OnRangingSessionStopped — typically
//     with kHardwareError.
//
// REAL ADAPTER: this function calls the radio driver's "go" entry point —
// arm the scan, fire the publisher, kick the CS measurement engine, etc.
// The radio reports measurements asynchronously from this point on.
CHIP_ERROR LoggingRangingAdapter::StartSession(uint8_t sessionId)
{
    ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s] StartSession id=%u", LogTag(), sessionId);
    auto it = std::find_if(mSessions.begin(), mSessions.end(),
                           [sessionId](const std::unique_ptr<Session> & s) { return s->sessionId == sessionId; });
    VerifyOrReturnError(it != mSessions.end(), CHIP_ERROR_NOT_FOUND);
    Session & session = **it;
    VerifyOrReturnError(session.state == Session::State::kPrepared, CHIP_ERROR_INCORRECT_STATE);

    // Schedule the first measurement tick at 0ms so that periodic ranging
    // emits a measurement right at start-time (matching what real radios do
    // — the first sample arrives as soon as the radio's ranging engine has
    // converged, not one cadence-interval later). Subsequent ticks fire on
    // `interval` cadence via TimerFired → ScheduleNextFire.
    session.state = Session::State::kActive;
    LogErrorOnFailure(mTimerDelegate.StartTimer(&session, System::Clock::Milliseconds32(0)));
    return CHIP_NO_ERROR;
}

// StopSession is invoked by the driver when a client issues StopRanging or
// when the EndTime cutoff fires. It MUST work uniformly on both kPrepared
// (StartSession not yet called) and kActive sessions. Returns
// CHIP_ERROR_NOT_FOUND if the ID is not active so the cluster can reflect
// that to the client.
//
// REAL ADAPTER: tear down whatever was staged or armed for this session
// (cancel scan, release publisher, abort CS measurement, etc.). If the
// radio's stop is async, return CHIP_NO_ERROR and emit
// OnRangingSessionStopped only once the radio has confirmed teardown.
CHIP_ERROR LoggingRangingAdapter::StopSession(uint8_t sessionId)
{
    ChipLogProgress(NotSpecified, "[LoggingRangingAdapter:%s] StopSession id=%u", LogTag(), sessionId);
    auto it = std::find_if(mSessions.begin(), mSessions.end(),
                           [sessionId](const std::unique_ptr<Session> & s) { return s->sessionId == sessionId; });
    VerifyOrReturnError(it != mSessions.end(), CHIP_ERROR_NOT_FOUND);

    // Cancel the measurement timer regardless of state — it was only ever
    // armed in kActive, but CancelTimer is safe when nothing is pending.
    mTimerDelegate.CancelTimer(it->get());

    // Erase the session before invoking the callback so any synchronous
    // query of the active session list during the callback observes a
    // consistent state (the stopped session is no longer reported active).
    mSessions.erase(it);

    // Adapters are required to deliver OnRangingSessionStopped for every
    // session termination. Client-initiated stop is reported as
    // SessionEndTimeReached (the driver remaps to kPeerNotFound when no
    // measurement passed its filter).
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
        // Capture the sessionId locally because mSessions.pop_back()
        // destroys the back element; we then erase before invoking the
        // callback so any synchronous query of the active session list
        // observes a consistent state. Same ordering as StopSession and
        // TerminateSession.
        uint8_t sessionId = mSessions.back()->sessionId;
        mTimerDelegate.CancelTimer(mSessions.back().get());
        mSessions.pop_back();

        if (mCallback != nullptr)
        {
            mCallback->OnRangingSessionStopped(sessionId, RangingSessionStatusEnum::kSessionEndTimeReached);
        }
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

// Session::TimerFired is the synthesizer's measurement tick. The session
// must be kActive to reach here (StartSession is what arms the first fire).
// Fires a measurement (skipped when the unknown-peer sentinel is set), then
// reschedules if the cadence is periodic; for instant ranging (interval == 0)
// the session sits idle after the single measurement until the driver
// invokes StopSession at EndTime.
//
// REAL ADAPTER: this entire function is replaced by callbacks from the radio.
// The radio reports each measurement directly into OnMeasurementData; the
// adapter does not need a measurement-cadence timer. The driver-owned
// EndTime cutoff likewise removes any need for an adapter-side end timer.
void LoggingRangingAdapter::Session::TimerFired()
{
    if (state != State::kActive)
    {
        return;
    }

    if (!isUnknownPeer && owner.mCallback != nullptr)
    {
        auto measurement = owner.BuildMeasurement(*this);
        owner.mCallback->OnMeasurementData(sessionId, measurement);
    }

    if (interval != System::Clock::Milliseconds32(0))
    {
        owner.ScheduleNextFire(*this);
    }
}

// Common termination path used by any internal "stop this session now"
// reason that does not arrive via StopSession (e.g. simulated hardware
// failure). Cancels the outstanding timer, notifies the cluster, and
// removes the session record.
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
    // For instant ranging (interval == 0), schedule a 0ms tick so the single
    // measurement still goes through the timer path. For periodic ranging,
    // schedule the next interval.
    System::Clock::Milliseconds32 delay =
        (session.interval != System::Clock::Milliseconds32(0)) ? session.interval : System::Clock::Milliseconds32(0);
    LogErrorOnFailure(mTimerDelegate.StartTimer(&session, delay));
}

// BuildMeasurement constructs a synthetic RangingMeasurementDataStruct with
// deterministic defaults plus the technology-specific peer identity captured
// at PrepareSession time.
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
            measurement.BLTDevIK.SetValue(ByteSpan(mBltDevIK, kDeviceIdentityKeyLen));
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
            measurement.wiFiDevIK.SetValue(ByteSpan(mWiFiDevIK, kDeviceIdentityKeyLen));
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
