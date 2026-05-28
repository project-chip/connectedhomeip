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

#include <devices/proximity-ranger/RangingAdapter.h>

#include <clusters/ProximityRanging/Commands.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Structs.h>
#include <lib/core/CHIPError.h>
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
 * Intended for certification testing of the Proximity Ranging cluster server
 * without real hardware. The technology to bind this adapter to
 * is selected via the constructor — one instance handles a single technology
 * (BLERBC, WFUSDPD, or BLTCS), exposes its capabilities, and populates the
 * technology-specific fields of the synthesized RangingMeasurementDataStruct.
 *
 * Supported technologies:
 *   - kBLEBeaconRSSIRanging (BLERBC)
 *   - kBluetoothChannelSounding (BLTCS)
 *   - kWiFiRoundTripTimeRanging (WFUSDPD) — the controller's WFUSDPD lookup
 *     falls back to kWiFiNextGenerationRanging if needed, and either tech
 *     advertises the same WiFiDevIK attribute.
 *
 * Measurement timing (per spec):
 *   - First RangingResult emitted StartTime seconds after StartSession.
 *   - If RangingInstanceInterval is present, subsequent RangingResults are
 *     emitted every interval until EndTime.
 *   - If RangingInstanceInterval is absent, exactly one RangingResult is
 *     emitted (instant ranging).
 *   - At EndTime, a RangingSessionStatus(SessionEndTimeReached) event is
 *     emitted and the session terminates.
 *
 * ReportingCondition handling: synthesized measurements are evaluated against
 * the captured ReportingCondition before emission. Measurements that do not
 * satisfy the condition are silently dropped — this lets cert tests exercise
 * both the satisfied and unsatisfiable code paths by choosing a condition
 * relative to the fixed default distance (kDefaultDistanceCm).
 *
 * Synthetic defaults are deterministic so cert tests can assert exact values.
 * Edge-case fields (Distance=null, DetectedAttackLevel, hardware errors) are
 * driven by a TestEventTrigger handler, added in a follow-up change.
 */
class LoggingRangingAdapter : public RangingAdapter
{
public:
    /// Synthetic fixed measurement defaults — chosen to satisfy a permissive
    /// ReportingCondition, and deterministic so cert tests can assert them.
    static constexpr uint16_t kDefaultDistanceCm    = 100;
    static constexpr uint16_t kDefaultErrorMarginCm = 10;
    static constexpr int8_t kDefaultRssiDbm         = -50;
    static constexpr int8_t kDefaultTxPowerDbm      = 0;

    /// Gap between the measurement fire and the terminate fire for instant
    /// ranging. Simulates a realistic measurement duration (BLE scan, WiFi RTT
    /// round trip, etc.) and gives the subscription engine time to flush a
    /// SessionIDList "session added" report before the "session removed"
    /// report — without it the two MarkDirty calls collapse into a single
    /// report carrying the final empty state, and one-shot reads from
    /// chip-tool race past the active window.
    static constexpr System::Clock::Milliseconds32 kInstantRangingTerminateDelay{ 3000 };

    /// Deterministic BLE Device ID (kept stable across runs to ease cert
    /// scripting). A real adapter would generate this from a CSPRNG and
    /// persist it across reboots.
    static constexpr uint64_t kBleDeviceId = 0xA110CA7E1234ABCDull;

    /// Deterministic 16-byte WiFiDevIK (a fixed, distinguishable pattern so
    /// cert tests can assert exact attribute reads).
    static constexpr uint8_t kWiFiDevIK[kDeviceIdentityKeyLen] = {
        0x57, 0x49, 0x46, 0x49, 0x44, 0x65, 0x76, 0x49, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };

    /// Deterministic 16-byte BLTDevIK.
    static constexpr uint8_t kBltDevIK[kDeviceIdentityKeyLen] = {
        0x42, 0x4C, 0x54, 0x44, 0x65, 0x76, 0x49, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    };

    /// Constructs a logging adapter bound to one of the supported technologies.
    /// `technology` MUST be one of: kBLEBeaconRSSIRanging,
    /// kBluetoothChannelSounding, or kWiFiRoundTripTimeRanging.
    LoggingRangingAdapter(RangingTechEnum technology, TimerDelegate & timerDelegate) :
        mTechnology(technology), mTimerDelegate(timerDelegate)
    {}
    ~LoggingRangingAdapter() override;

    // RangingAdapter interface
    RangingTechEnum GetTechnology() const override { return mTechnology; }
    Structs::RangingCapabilitiesStruct::Type GetCapabilities() const override;
    ResultCodeEnum StartSession(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request) override;
    CHIP_ERROR StopSession(uint8_t sessionId) override;
    void StopAllSessions() override;
    CHIP_ERROR GetActiveSessionIds(std::vector<uint8_t> & sessionIds) override;
    std::optional<uint64_t> GetDeviceId() override;
    std::optional<WiFiUsdConfig> GetWiFiUsdConfig() override;
    std::optional<BltcsConfig> GetBltcsConfig() override;

private:
    struct Session : public TimerContext
    {
        Session(LoggingRangingAdapter & adapter, uint8_t id) : owner(adapter), sessionId(id) {}
        void TimerFired() override;

        LoggingRangingAdapter & owner;
        uint8_t sessionId;
        System::Clock::Timestamp startAt = System::Clock::kZero;
        System::Clock::Timestamp endAt   = System::Clock::kZero;
        System::Clock::Milliseconds32 interval{ 0 }; // 0 = single-shot (instant ranging)
        bool hasReporting = false;
        Structs::ReportingConditionStruct::Type reporting;
        bool firstFired = false;

        /// Per-session peer identity captured from the StartRangingRequest,
        /// surfaced in the synthesized measurement so cert tests can verify
        /// the result matches the SessionID's peer.
        std::optional<uint64_t> peerBleDeviceId;
        std::optional<std::array<uint8_t, kDeviceIdentityKeyLen>> peerWiFiDevIK;
        std::optional<std::array<uint8_t, kDeviceIdentityKeyLen>> peerBltDevIK;
    };

    /// Removes a session from the active list and notifies the cluster via the
    /// callback. Safe to call from inside Session::TimerFired (vector erase
    /// happens after the synchronous OnRangingSessionStopped notification, so
    /// the controller's bookkeeping observes a consistent state).
    void TerminateSession(Session & session, RangingSessionStatusEnum status);

    /// Returns true if measurement satisfies the captured ReportingCondition.
    static bool SatisfiesReporting(const Structs::ReportingConditionStruct::Type & reporting,
                                   const Structs::RangingMeasurementDataStruct::Type & measurement);

    /// Schedules the next timer fire for `session`. Caller has already
    /// captured (or just refreshed) the session's monotonic deadlines.
    void ScheduleNextFire(Session & session);

    /// Build the deterministic-default measurement, including the
    /// technology-specific fields populated from the session's captured peer
    /// identity (so the result reflects which peer the SessionID belongs to).
    Structs::RangingMeasurementDataStruct::Type BuildMeasurement(const Session & session) const;

    /// Short log tag for the bound technology, e.g. "BLERBC".
    const char * LogTag() const;

    RangingTechEnum mTechnology;
    TimerDelegate & mTimerDelegate;
    std::vector<std::unique_ptr<Session>> mSessions;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
