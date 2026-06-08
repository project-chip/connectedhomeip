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

#include <clusters/ProximityRanging/Commands.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <lib/support/Span.h>
#include <system/SystemClock.h>

#include <cstdint>
#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/// Length of the Device Identity Key for Wi-Fi USD and BLTCS.
inline constexpr size_t kDeviceIdentityKeyLen = 16;
using DeviceIdentityKey                       = uint8_t[kDeviceIdentityKeyLen];

/// Per-technology configuration exposed via the cluster's optional attributes.
/// Adapters return one of these only when they implement the matching technology;
/// otherwise the corresponding Get accessor returns std::nullopt and the cluster
/// surfaces UnsupportedAttribute.
struct BleRbcConfig
{
    uint64_t deviceId;
};

struct WiFiUsdConfig
{
    DeviceIdentityKey deviceIdentityKey;
};

struct BltcsConfig
{
    DeviceIdentityKey deviceIdentityKey;
    BLTCSSecurityLevelEnum securityLevel;
    BLTCSModeEnum modeCapability;
};

/**
 * Narrowed parameter set passed to RangingAdapter::PrepareSession.
 *
 * The driver owns wall-clock scheduling: it gates the StartTime delay before
 * calling StartSession and applies the EndTime cutoff via StopSession. Adapters
 * therefore never see the request's startTime / endTime fields — only the
 * cadence (rangingInstanceInterval, in seconds; std::nullopt means single-shot
 * "instant" ranging) and the role / band selectors the radio needs to bring
 * up the session. The driver also filters OnMeasurementData against
 * ReportingCondition itself, so adapters do not receive that field either.
 */
struct StartSessionParams
{
    RangingTechEnum technology;
    /// Periodic measurement interval in seconds. std::nullopt means single-shot
    /// (instant) ranging — the adapter emits exactly one measurement and then
    /// waits for the driver-owned end-time cutoff to terminate the session.
    std::optional<uint32_t> rangingInstanceInterval;
    std::optional<Structs::BLERangingDeviceRoleConfigStruct::DecodableType> bleRoleConfig;
    std::optional<Structs::WiFiRangingDeviceRoleConfigStruct::DecodableType> wifiRoleConfig;
    std::optional<Structs::BLTChannelSoundingDeviceRoleConfigStruct::DecodableType> bltRoleConfig;
    std::optional<chip::BitMask<RadioBandBitmap>> frequencyBand;
    std::optional<chip::BitMask<RangingBandwidthBitmap>> bandwidth;
};

/**
 * Interface for a single ranging technology (BLE RSSI, Channel Sounding, Wi-Fi USD, etc.).
 *
 * Lifecycle: platform-specific adapters are owned by the application and must
 * be registered with ProximityRangingDriver before the Proximity Ranging
 * cluster is initialised. Adapters must outlive the driver, or be
 * unregistered before destruction.
 *
 * Two-phase session model
 * -----------------------
 * Each ranging session is brought up in two steps so the driver can centralise
 * StartTime / EndTime scheduling that would otherwise be duplicated in every
 * adapter:
 *
 *   1. PrepareSession(sessionId, params) — synchronous. The adapter validates
 *      that the radio can satisfy the request (peer compatibility, security
 *      mode, resource availability, ...) and stages whatever per-session
 *      bookkeeping it needs. Returns a ResultCodeEnum: kAccepted commits the
 *      adapter to the session; any other value rejects the request before the
 *      cluster surfaces a session ID to the client. If the adapter accepted
 *      and stashed resources, those resources MUST be released on a subsequent
 *      StopSession call (see "Stop semantics" below).
 *
 *   2. StartSession(sessionId) — synchronous. Invoked by the driver after the
 *      StartTime delay (immediately when StartTime == 0). The adapter kicks
 *      off the radio activity using the parameters it stashed in
 *      PrepareSession. Returns CHIP_NO_ERROR when ranging has been kicked off;
 *      asynchronous failures are reported via OnRangingSessionStopped with an
 *      appropriate RangingSessionStatusEnum value (typically kHardwareError).
 *      Returning a non-NO_ERROR error is permitted for "could not even
 *      attempt" cases, but adapters SHOULD prefer the asynchronous failure
 *      path so the cluster's event stream is uniform.
 *
 * Session IDs are 1-byte values allocated by the driver and passed verbatim
 * into both calls. The adapter is responsible for tracking which IDs are
 * currently in flight (whether prepared or actively ranging) and reporting
 * termination of every session it has accepted via PrepareSession through
 * OnRangingSessionStopped.
 *
 * Stop semantics
 * --------------
 * StopSession(sessionId) MUST work uniformly on both states — a session that
 * was prepared but for which StartSession has not yet been called, AND a
 * session that is actively ranging. In both cases the adapter releases its
 * per-session resources, performs any radio teardown that applies to the
 * current state, and emits OnRangingSessionStopped exactly once.
 *
 * Adapters MUST NOT deliver OnRangingSessionStopped synchronously from inside
 * PrepareSession; if the start is already known to have failed at preparation
 * time, return a non-Accepted ResultCodeEnum from PrepareSession instead.
 * Termination callbacks are valid only after PrepareSession has returned
 * kAccepted.
 *
 * Async events:
 *   - OnMeasurementData: emitted per the session's rangingInstanceInterval
 *     for the lifetime of the session (driver-owned StartTime / EndTime gates
 *     do not need to be applied by the adapter).
 *   - OnAttributeChanged: called when a runtime configuration value the
 *     adapter exposes as an attribute changes, so the cluster can mark it
 *     dirty.
 *   - OnRangingSessionStopped: emitted when a ranging session has stopped
 *     either by request (StopSession), by hardware failure, or by any other
 *     adapter-internal reason. The driver invokes StopSession at EndTime, so
 *     adapters do not need a self-managed end-time cutoff.
 *
 * Threading: methods on this interface are called from the Matter main
 * thread. The Callback is thread-safe; adapters MAY invoke it from any
 * thread.
 *
 * Optional capabilities: GetDeviceId defaults to std::nullopt; override only
 * when the adapter exposes a stable per-device identifier (e.g. BLE Device ID).
 */
class RangingAdapter
{
public:
    class Callback
    {
    public:
        virtual ~Callback() = default;

        virtual void OnRangingSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status)                           = 0;
        virtual void OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement) = 0;
        virtual void OnAttributeChanged(AttributeId attributeId)                                                           = 0;
    };

    virtual ~RangingAdapter() = default;

    /**
     * The callback must remain valid until the adapter is unregistered or destroyed.
     */
    void SetCallback(Callback * callback) { mCallback = callback; }

    virtual RangingTechEnum GetTechnology() const                            = 0;
    virtual Structs::RangingCapabilitiesStruct::Type GetCapabilities() const = 0;

    /**
     * Validate and stage a ranging session. Returns kAccepted to commit the
     * adapter to the session; any other value rejects it. The driver only
     * surfaces a session ID to the client when this returns kAccepted.
     *
     * After kAccepted, the adapter MUST emit OnRangingSessionStopped exactly
     * once for this session — either in response to a future StopSession
     * call, or asynchronously on hardware-driven termination.
     */
    virtual ResultCodeEnum PrepareSession(uint8_t sessionId, const StartSessionParams & params) = 0;

    /**
     * Kick off ranging for a previously prepared session. Invoked after the
     * driver-owned StartTime delay (immediately when StartTime == 0). Returns
     * CHIP_NO_ERROR when ranging has been started; asynchronous failures
     * MUST be reported via OnRangingSessionStopped.
     */
    virtual CHIP_ERROR StartSession(uint8_t sessionId) = 0;

    /// Stop a session that has been prepared and possibly started. Works on
    /// both not-yet-started and active sessions. The adapter MUST emit
    /// OnRangingSessionStopped for the session as a result of this call.
    virtual CHIP_ERROR StopSession(uint8_t sessionId) = 0;

    virtual void StopAllSessions() = 0;

    /**
     * Append the adapter's currently-active session IDs to the caller-supplied
     * span. The span's size on entry is its capacity; on exit the size reflects
     * how many IDs were written. Returns CHIP_ERROR_BUFFER_TOO_SMALL if the
     * capacity is insufficient.
     */
    virtual CHIP_ERROR GetActiveSessionIds(Span<uint8_t> & sessionIds) = 0;

    /// Override only when the adapter exposes a stable per-device 64-bit
    /// identifier (e.g. BLE Device ID). Default returns std::nullopt.
    virtual std::optional<uint64_t> GetDeviceId() { return std::nullopt; }

    /// Override only on adapters whose technology is one of the Wi-Fi USD
    /// proximity-detection technologies (WiFiRoundTripTimeRanging or
    /// WiFiNextGenerationRanging). Default returns std::nullopt.
    virtual std::optional<WiFiUsdConfig> GetWiFiUsdConfig() { return std::nullopt; }

    /// Override only on adapters whose technology is BluetoothChannelSounding.
    /// Default returns std::nullopt.
    virtual std::optional<BltcsConfig> GetBltcsConfig() { return std::nullopt; }

protected:
    Callback * mCallback = nullptr;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
