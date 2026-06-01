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
 * Narrowed parameter set passed to RangingAdapter::StartSession.
 *
 * The driver filters OnMeasurementData against ReportingCondition itself, so
 * adapters no longer receive the reportingCondition field. Everything else
 * the radio needs to bring up a ranging session is here: technology, the
 * trigger (startTime / endTime / rangingInstanceInterval) for cadence, the
 * matching role config (peer identity), and the optional radio-tunable
 * frequency / bandwidth selectors.
 */
struct StartSessionParams
{
    RangingTechEnum technology;
    Structs::RangingTriggerConditionStruct::Type trigger;
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
 * Session model: the driver allocates 1-byte session IDs and passes them to
 * StartSession. The adapter tracks which IDs are currently active and is
 * responsible for reporting termination of every session it has accepted -
 * whether requested via StopSession or driven by the underlying technology -
 * through OnRangingSessionStopped.
 *
 * StartSession / StopSession semantics: both return synchronously with the
 * status of the start/stop request. An adapter MAY return success early if
 * the underlying start takes too long; in that case it must subsequently
 * deliver OnRangingSessionStopped with HardwareError if the session fails to
 * come up.
 *
 * Adapters MUST NOT deliver OnRangingSessionStopped synchronously from inside
 * StartSession. If the start is already known to have failed, return a
 * non-Accepted ResultCodeEnum instead. Termination callbacks are valid only
 * after StartSession has returned kAccepted.
 *
 * Async events:
 *   - OnMeasurementData: emitted per the session's measurement parameters
 *     (interval, peer config) for the lifetime of the session.
 *   - OnAttributeChanged: called when a runtime configuration value the
 *     adapter exposes as an attribute changes, so the cluster can mark it
 *     dirty.
 *   - OnRangingSessionStopped: emitted when a ranging session has stopped
 *     either by timeout, request, or from a hardware failure.
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

    virtual ResultCodeEnum StartSession(uint8_t sessionId, const StartSessionParams & params) = 0;
    virtual CHIP_ERROR StopSession(uint8_t sessionId)                                         = 0;
    virtual void StopAllSessions()                                                            = 0;

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
