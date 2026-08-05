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
 * Hardware-free RangingAdapter that logs every API call and synthesizes a
 * single measurement per StartSession invocation.
 *
 * Read `RangingAdapter.h` first for the full platform-integration contract
 * (registration lifecycle, threading, callback semantics, optional
 * capabilities). This class is a hardware-free simulation implementation of a
 * Proximity Ranger device that avoids calling platform radio APIs, allowing
 * the Proximity Ranging cluster server to be exercised on developer machines
 * and in CI without ranging hardware, and providing platform integrators with
 * a reference example of every required override.
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
 * Single-shot measurement model
 * ----------------------------
 *   - On every StartSession call, the adapter schedules a kSimulatedRangingDuration
 *     timer (3 seconds) to simulate radio measurement latency.
 *   - When the timer fires, the adapter emits exactly ONE OnMeasurementData
 *     callback and returns to the kPrepared state.
 *   - The driver owns the cadence: for periodic ranging, it calls StartSession
 *     again on each rangingInstanceInterval tick; for instant ranging, it
 *     calls StopSession after the first passing measurement.
 *   - If the driver issues StartSession while a measurement is still in
 *     flight (kMeasuring), the adapter returns CHIP_ERROR_BUSY and the driver
 *     skips that tick.
 *
 * REAL ADAPTER: a hardware adapter does not synthesize a measurement timer.
 * Its StartSession kicks off the radio's ranging engine; the radio reports a
 * measurement asynchronously when ready. The adapter does NOT need to handle
 * cadence or end-time — those are entirely driver-owned.
 *
 * ReportingCondition handling
 * ---------------------------
 * Synthesized measurements are NOT evaluated against ReportingCondition by
 * this adapter — the driver applies that filter on every OnMeasurementData
 * callback. The adapter just emits its default measurement and lets the
 * driver suppress it when appropriate.
 *
 * Synthetic defaults are deterministic so cert tests can assert exact values.
 */
class LoggingRangingAdapter : public RangingAdapter
{
public:
    /// Constructs a logging adapter bound to one of the supported technologies.
    /// `technology` MUST be one of: kBLEBeaconRSSIRanging,
    /// kBluetoothChannelSounding, kWiFiRoundTripTimeRanging, OR
    /// kWiFiNextGenerationRanging.
    ///
    /// `timerDelegate` is borrowed and must outlive this adapter; the stub
    /// uses it to simulate radio measurement latency. A hardware adapter
    /// typically would not need a TimerDelegate at construction — it instead
    /// binds to its radio driver here.
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
    /// accepted via PrepareSession, so a real adapter's destructor must drain
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
    ResultCodeEnum PrepareSession(uint8_t sessionId, const StartSessionParams & params) override;
    CHIP_ERROR StartSession(uint8_t sessionId) override;
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
    /// record here, but the fields look quite different — instead of a
    /// simulated-latency timer, it holds the radio / firmware handle for
    /// the in-flight ranging session, any cached crypto material, and
    /// reservation state for shared radio resources. The session ID is
    /// still the 1-byte value the cluster allocated, since that is what
    /// gets surfaced over the wire and what StopSession is keyed by.
    struct Session : public TimerContext
    {
        Session(LoggingRangingAdapter & adapter, uint8_t id) : owner(adapter), sessionId(id) {}
        void TimerFired() override;

        enum class State
        {
            kPrepared,  ///< PrepareSession returned kAccepted (or a measurement just completed); ready for the next StartSession.
            kMeasuring, ///< StartSession invoked; simulated measurement timer is in flight.
        };

        LoggingRangingAdapter & owner;
        uint8_t sessionId;
        State state = State::kPrepared;

        /// Set at PrepareSession when the captured peer identity for this
        /// session's technology matches the "unknown peer" sentinel pattern.
        /// While true, TimerFired never emits OnMeasurementData; the driver
        /// then sees no measurement pass its filter and remaps the eventual
        /// kSessionEndTimeReached to kPeerNotFound — the spec-correct outcome
        /// when a hardware radio cannot range against the requested peer.
        bool isUnknownPeer = false;

        /// True when the StartRangingRequest's role for this session is one
        /// of the passive-responder roles (BLEBeacon / WiFiPublisher /
        /// BLTReflector). Real radios in passive-responder mode do not
        /// produce ranging measurements; only the active initiator does.
        /// While true, TimerFired never emits OnMeasurementData — the
        /// session simply runs until the driver explicitly stops it (either
        /// via end-time or a StopRangingRequest from the controller).
        bool isPassiveResponder = false;

        /// Per-session peer identity captured from the StartRangingRequest,
        /// surfaced in the synthesized measurement so cert tests can verify
        /// the result matches the SessionID's peer.
        ///
        /// REAL ADAPTER: the cluster server already passes the peer's
        /// identity into PrepareSession via the role-config field that
        /// matches this technology — a real adapter forwards that value
        /// directly to the radio (e.g. as the BLE scan filter target or
        /// the Wi-Fi USD publisher key). It does not need to surface the
        /// peer in RangingMeasurementDataStruct because the radio's
        /// measurement already carries it.
        std::optional<uint64_t> peerBleDeviceId;
        std::optional<std::array<uint8_t, kDeviceIdentityKeyLen>> peerWiFiDevIK;
        std::optional<std::array<uint8_t, kDeviceIdentityKeyLen>> peerBltDevIK;
    };

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

    /// Local 16-byte device identity keys for the Wi-Fi USD and Bluetooth
    /// Channel Sounding technologies. Filled with CSPRNG bytes in the
    /// constructor for the matching technology only; the unused buffer for
    /// the other tech is left zeroed and never read. Regenerated each process
    /// start — persistence is intentionally out of scope for this stub.
    ///
    /// REAL ADAPTER: a production Wi-Fi USD or BLTCS adapter MUST persist
    /// these keys across reboots so peers can correlate sessions over time.
    uint8_t mWiFiDevIK[kDeviceIdentityKeyLen] = {};
    uint8_t mBltDevIK[kDeviceIdentityKeyLen]  = {};

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
