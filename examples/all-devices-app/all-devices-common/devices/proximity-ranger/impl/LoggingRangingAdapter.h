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
#pragma once

#include <app/clusters/proximity-ranging-server/BleRssiRangingHelpers.h>
#include <app/clusters/proximity-ranging-server/RangingAdapter.h>

#include <clusters/ProximityRanging/Commands.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TimerDelegate.h>
#include <system/SystemClock.h>

#include <array>
#include <memory>
#include <optional>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/**
 * Hardware-free RangingAdapter that logs every API call and synthesizes
 * measurement events from the StartRangingRequest's timing fields.
 *
 * Read `RangingAdapter.h` first for the full platform-integration contract
 * (registration lifecycle, threading, callback semantics, optional
 * capabilities). This class is a *fake* implementation of a Proximity Ranger
 * device that deliberately avoids calling any platform radio APIs so that
 * the Proximity Ranging cluster server can be exercised on developer
 * machines and in CI without ranging hardware, and so that platform
 * integrators have a worked example of every required override.
 *
 * Purpose
 * -------
 * This file is intended to serve two roles:
 *   1. A drop-in adapter for certification testing of the Proximity Ranging
 *      cluster server without real ranging hardware.
 *   2. A *reference implementation* showing platform integrators what a real
 *      RangingAdapter must do. The class-level and per-method comments
 *      describe the contract surfaced by RangingAdapter.h, and call out where
 *      this stub deliberately substitutes a synthetic behavior for what a
 *      production adapter would do (e.g. driving real radio hardware,
 *      persisting identity keys, deriving session IDs from the radio stack).
 *
 * Look for `// REAL ADAPTER:` notes throughout LoggingRangingAdapter.cpp —
 * they highlight what differs between this stub and a production adapter.
 *
 * One adapter, one technology
 * ---------------------------
 * Each instance is bound to a single RangingTechEnum value via the
 * constructor. ProximityRangingDriver expects one adapter per supported
 * technology, registered before ProximityRangingDriver::Init(). Real
 * platforms typically build one adapter class per backend (e.g. a
 * WiFiRttAdapter, a BltcsAdapter) rather than dispatching internally on the
 * technology enum, but the pattern below shows both styles by switching on
 * mTechnology.
 *
 * Supported technologies:
 *   - kBLEBeaconRSSIRanging       (PROXR feature: BLERBC)
 *   - kBluetoothChannelSounding   (PROXR feature: BLTCS)
 *   - kWiFiRoundTripTimeRanging   (PROXR feature: WFUSDPD)
 *   - kWiFiNextGenerationRanging  (PROXR feature: WFUSDPD)
 *
 * Synthesized measurement timing (per Proximity Ranging spec)
 * ----------------------------------------------------------
 *   - First RangingResult emitted StartTime seconds after StartSession.
 *   - If RangingInstanceInterval is present, subsequent RangingResults are
 *     emitted every interval until EndTime.
 *   - If RangingInstanceInterval is absent, exactly one RangingResult is
 *     emitted (instant ranging).
 *   - At EndTime, a RangingSessionStatus(SessionEndTimeReached) event is
 *     emitted and the session terminates.
 *
 * REAL ADAPTER: a hardware adapter does not synthesize a timer chain. The
 * radio firmware (or its host driver) emits measurement callbacks at the
 * cadence the StartRangingRequest's trigger negotiated with the peer; the
 * adapter forwards each callback into OnMeasurementData and only schedules
 * its own timer for the EndTime cutoff and any safety / liveness checks.
 *
 * ReportingCondition handling
 * ---------------------------
 * Synthesized measurements are evaluated against the captured
 * ReportingCondition before emission. Measurements that do not satisfy the
 * condition are silently dropped — this lets cert tests exercise both the
 * satisfied and unsatisfiable code paths by choosing a condition relative to
 * the fixed default distance baked into BuildMeasurement.
 *
 * REAL ADAPTER: ReportingCondition is a *transport-level* filter that the
 * cluster server already applies before invoking OnMeasurementData consumers.
 * A production adapter does NOT need to evaluate it — it simply delivers
 * every raw measurement from the radio. We only re-evaluate it here because
 * we are synthesizing measurements rather than receiving them, and applying
 * the filter at synthesis time keeps the cert harness's expected event count
 * deterministic.
 *
 * Synthetic defaults are deterministic so cert tests can assert exact values.
 */
class LoggingRangingAdapter : public RangingAdapter
{
public:
    /// Deterministic 16-byte WiFiDevIK (a fixed, distinguishable pattern so
    /// cert tests can assert exact attribute reads).
    ///
    /// REAL ADAPTER: WiFiDevIK is the local Wi-Fi USD Device Identity Key.
    /// The spec requires it to be a CSPRNG-generated 16-byte value persisted
    /// across reboots; the value here is intentionally NOT a secret and MUST
    /// NOT be reused on a real device.
    static constexpr uint8_t kWiFiDevIK[kDeviceIdentityKeyLen] = {
        0x57, 0x49, 0x46, 0x49, 0x44, 0x65, 0x76, 0x49, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };

    /// Deterministic 16-byte BLTDevIK.
    ///
    /// REAL ADAPTER: BLTDevIK is the local Bluetooth Channel Sounding Device
    /// Identity Key. Same generation/persistence requirements as WiFiDevIK —
    /// CSPRNG-derived, persisted, and never hard-coded.
    static constexpr uint8_t kBltDevIK[kDeviceIdentityKeyLen] = {
        0x42, 0x4C, 0x54, 0x44, 0x65, 0x76, 0x49, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    };

    /// Constructs a logging adapter bound to one of the supported technologies.
    /// `technology` MUST be one of: kBLEBeaconRSSIRanging,
    /// kBluetoothChannelSounding, kWiFiRoundTripTimeRanging, OR
    /// kWiFiNextGenerationRanging.
    ///
    /// `timerDelegate` is borrowed and must outlive this adapter; the stub
    /// uses it to simulate measurement cadence. A hardware adapter typically
    /// would not need a TimerDelegate at construction — it instead binds to
    /// its radio driver here.
    ///
    /// `storage` is REQUIRED only when `technology` is kBLEBeaconRSSIRanging
    /// (in which case the BLEDeviceID must be persisted across reboots so
    /// peers can correlate beacons over time) and is otherwise unused. Pass
    /// nullptr for non-BLE-RSSI technologies. The pointer is borrowed and
    /// must outlive this adapter. The constructor VerifyOrDies if storage
    /// is missing for a BLE-RSSI adapter.
    ///
    /// Per-technology identity persistence: the BLE Beacon Ranging
    /// BLEDeviceID is defined at the Matter layer and is therefore
    /// generated and persisted by the cluster/adapter directly via the
    /// supplied PersistentStorageDelegate. Platform adapters that own
    /// similar per-radio identity keys (WiFiDevIK, BLTDevIK, …) should
    /// follow the same pattern: on construction, retrieve the persisted
    /// value from storage and, if absent, CSPRNG-generate it and persist
    /// it before the cluster's Init runs, so the Get* accessors always
    /// have a stable value to return.
    ///
    /// Optional Init() pattern: platforms that need to query the radio for
    /// runtime properties (frequency band support, security capabilities,
    /// identity key material that lives in the radio rather than in
    /// PersistentStorageDelegate) MAY add a public Init() method that the
    /// application invokes before registering the adapter with the driver.
    /// Caching those properties in member fields lets the synchronous
    /// Get* methods return without blocking on the radio. This adapter
    /// does all of its setup in the constructor because it has no radio
    /// to talk to.
    ///
    /// `periodicRangingSupport` controls the RangingCapabilitiesStruct field
    /// of the same name. Real adapters derive this from the radio's
    /// capabilities rather than accepting it as a constructor flag.
    LoggingRangingAdapter(RangingTechEnum technology, TimerDelegate & timerDelegate, PersistentStorageDelegate * storage = nullptr,
                          bool periodicRangingSupport = false);
    /// Destructor MUST end every session it has started — see StopAllSessions
    /// in the .cpp for the cleanup order. RangingAdapter's contract requires
    /// OnRangingSessionStopped to fire for every session the adapter has
    /// accepted via StartSession, so a real adapter's destructor must drain
    /// any in-flight radio sessions and notify the cluster, not just free
    /// memory.
    ~LoggingRangingAdapter() override;

    // ----- RangingAdapter interface ------------------------------------------------
    //
    // The methods below implement the contract documented in
    // src/app/clusters/proximity-ranging-server/RangingAdapter.h. The
    // implementations in LoggingRangingAdapter.cpp annotate each one with
    // `// REAL ADAPTER:` notes describing what a hardware-backed adapter
    // would do at the same point.

    RangingTechEnum GetTechnology() const override { return mTechnology; }
    Structs::RangingCapabilitiesStruct::Type GetCapabilities() const override;
    ResultCodeEnum StartSession(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request) override;
    CHIP_ERROR StopSession(uint8_t sessionId) override;
    void StopAllSessions() override;
    CHIP_ERROR GetActiveSessionIds(Span<uint8_t> & sessionIds) override;

    // The optional Get* accessors below all default to std::nullopt in
    // RangingAdapter's base class. A concrete adapter SHOULD override only
    // the accessor that matches its bound technology (e.g. a BLE-RSSI
    // adapter overrides GetDeviceId; a BLTCS adapter overrides
    // GetBltcsConfig; a Wi-Fi USD adapter overrides GetWiFiUsdConfig) and
    // leave the others unimplemented so the cluster surfaces
    // UnsupportedAttribute. This stub overrides all three only because a
    // single instance is reused across every technology in the example app.
    std::optional<uint64_t> GetDeviceId() override;
    std::optional<WiFiUsdConfig> GetWiFiUsdConfig() override;
    std::optional<BltcsConfig> GetBltcsConfig() override;

private:
    /// Per-session state owned by the adapter.
    ///
    /// REAL ADAPTER: a hardware adapter typically still keeps a Session-like
    /// record here, but the fields look quite different — instead of timer
    /// deadlines and synthesized measurement plumbing, it holds the radio /
    /// firmware handle for the in-flight ranging session, any cached crypto
    /// material, and reservation state for shared radio resources. The
    /// session ID is still the 1-byte value the cluster allocated, since
    /// that is what gets surfaced over the wire and what StopSession is
    /// keyed by.
    struct Session : public TimerContext
    {
        Session(LoggingRangingAdapter & adapter, uint8_t id) : owner(adapter), sessionId(id) {}
        void TimerFired() override;

        LoggingRangingAdapter & owner;
        uint8_t sessionId;
        // Stub-only timing: deadlines and cadence used to synthesize
        // OnMeasurementData callbacks. Real adapters do not need these — the
        // radio reports measurements asynchronously on its own schedule.
        System::Clock::Timestamp startAt = System::Clock::kZero;
        System::Clock::Timestamp endAt   = System::Clock::kZero;
        System::Clock::Milliseconds32 interval{ 0 }; // 0 = single-shot (instant ranging)
        bool hasReporting = false;
        Structs::ReportingConditionStruct::Type reporting;
        bool firstFired = false;

        /// Per-session peer identity captured from the StartRangingRequest,
        /// surfaced in the synthesized measurement so cert tests can verify
        /// the result matches the SessionID's peer.
        ///
        /// REAL ADAPTER: the cluster server already passes the peer's
        /// identity into StartSession via the role-config field that matches
        /// this technology — a real adapter forwards that value directly to
        /// the radio (e.g. as the BLE scan filter target or the Wi-Fi USD
        /// publisher key). It does not need to surface the peer in
        /// RangingMeasurementDataStruct because the radio's measurement
        /// already carries it.
        std::optional<uint64_t> peerBleDeviceId;
        std::optional<std::array<uint8_t, kDeviceIdentityKeyLen>> peerWiFiDevIK;
        std::optional<std::array<uint8_t, kDeviceIdentityKeyLen>> peerBltDevIK;
    };

    /// Removes a session from the active list and notifies the cluster via the
    /// callback. Safe to call from inside Session::TimerFired (vector erase
    /// happens after the synchronous OnRangingSessionStopped notification, so
    /// the driver's bookkeeping observes a consistent state).
    ///
    /// REAL ADAPTER: same callback contract — every session that StartSession
    /// returned kAccepted for MUST be terminated via OnRangingSessionStopped.
    /// `status` carries the reason: SessionEndTimeReached for normal expiry
    /// and stop, HardwareError when the radio reports a failure, etc.
    void TerminateSession(Session & session, RangingSessionStatusEnum status);

    /// Returns true if measurement satisfies the captured ReportingCondition.
    ///
    /// REAL ADAPTER: not needed — the cluster server applies ReportingCondition
    /// filtering before delivering measurements to subscribers. We only
    /// pre-filter here because synthesized measurements are deterministic and
    /// we want the cert harness's expected event count to match the
    /// configured condition.
    static bool SatisfiesReporting(const Structs::ReportingConditionStruct::Type & reporting,
                                   const Structs::RangingMeasurementDataStruct::Type & measurement);

    /// Schedules the next timer fire for `session`. Caller has already
    /// captured (or just refreshed) the session's monotonic deadlines.
    ///
    /// REAL ADAPTER: not needed — the radio reports measurements; the adapter
    /// only needs a single timer for the EndTime cutoff (and even that is
    /// often handled by the radio firmware itself).
    void ScheduleNextFire(Session & session);

    /// Build the deterministic-default measurement, including the
    /// technology-specific fields populated from the session's captured peer
    /// identity (so the result reflects which peer the SessionID belongs to).
    ///
    /// REAL ADAPTER: replace this with a translation from the radio's native
    /// measurement format into RangingMeasurementDataStruct. Distance, error
    /// margin, RSSI, Tx power, and the technology-specific identity fields
    /// (BLEDeviceID / wiFiDevIK / BLTDevIK) all come from the radio's
    /// measurement output — none are static.
    Structs::RangingMeasurementDataStruct::Type BuildMeasurement(const Session & session) const;

    /// Short log tag for the bound technology, e.g. "BLERBC".
    const char * LogTag() const;

    bool mPeriodicRangingSupport = false;
    RangingTechEnum mTechnology;
    TimerDelegate & mTimerDelegate;

    /// Persistent storage backing the BLEDeviceID. Non-null only when
    /// mTechnology == kBLEBeaconRSSIRanging; required in that case so the
    /// BLEDeviceID survives reboots.
    PersistentStorageDelegate * mpStore = nullptr;

    /// Local BLEDeviceID for BLE Beacon RSSI ranging. Loaded from `mpStore`
    /// in the constructor; on first boot the constructor generates a new
    /// one via the BleRssi helper and persists it. Holds kInvalidBleDeviceId
    /// on technologies other than kBLEBeaconRSSIRanging.
    uint64_t mBleDeviceId = BleRssi::kInvalidBleDeviceId;

    /// Active sessions. unique_ptr keeps Session addresses stable across
    /// vector reallocations — important because each Session is registered
    /// with mTimerDelegate as a TimerContext, and the timer system holds the
    /// raw pointer until the timer fires or is cancelled.
    std::vector<std::unique_ptr<Session>> mSessions;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
