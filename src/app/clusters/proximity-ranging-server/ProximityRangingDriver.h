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

#include <app/AttributeValueEncoder.h>
#include <clusters/ProximityRanging/Commands.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <lib/support/Span.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/// Length of the Device Identity Key for Wi-Fi USD and BLTCS.
inline constexpr size_t kDeviceIdentityKeyLen = 16;
using DeviceIdentityKey                       = uint8_t[kDeviceIdentityKeyLen];

/// Per-technology configuration exposed via the cluster's optional attributes.
/// Drivers return one of these only when the corresponding feature is in the
/// cluster's feature map; otherwise the matching Get accessor returns nullopt
/// and the cluster surfaces UnsupportedAttribute.
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
 * Driver interface for the Proximity Ranging cluster.
 *
 * Lifecycle: the driver is owned by the application and must outlive the
 * cluster. The cluster calls Init() during Startup() and Shutdown() during
 * cluster shutdown; the driver may be destroyed after Shutdown() returns.
 *
 * Session model: a "session" is a single ranging exchange with a peer.
 * Session IDs are 1-byte values allocated by the cluster (not the driver)
 * when a StartRangingRequest is accepted. The driver tracks which IDs are
 * currently active and exposes them via GetActiveSessionIds() so the cluster
 * can surface SessionIDList and avoid colliding IDs on subsequent starts.
 *
 * Events: synchronous command results flow through HandleStartRanging /
 * HandleStopRanging. Async results — measurement data and unsolicited
 * session terminations — flow back through the Callback registered in Init().
 *
 * Threading: methods on this interface are called from the Matter main
 * thread. The registered Callback is thread-safe; drivers MAY invoke it from
 * any thread.
 *
 * Optional feature configuration: the Get*Config accessors return one
 * std::optional per supported technology and default to std::nullopt, which
 * the cluster maps to UnsupportedAttribute. Override the ones whose features
 * are present in the cluster's feature map. Adding new attributes within an
 * existing technology is a struct-field change rather than a new virtual on
 * every implementor.
 */
class ProximityRangingDriver
{
public:
    class Callback
    {
    public:
        virtual ~Callback() = default;

        virtual void OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement) = 0;
        virtual void OnSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status)                                  = 0;
        virtual void OnAttributeChanged(AttributeId attributeId)                                                           = 0;
    };

    virtual ~ProximityRangingDriver() = default;

    /**
     * The callback pointer remains valid until Shutdown() returns.
     */
    virtual CHIP_ERROR Init(Callback & callback) = 0;

    /**
     * Stops all active sessions and releases resources before the driver
     * may be destroyed.
     */
    virtual void Shutdown() = 0;

    virtual ResultCodeEnum HandleStartRanging(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request) = 0;
    virtual CHIP_ERROR HandleStopRanging(uint8_t sessionId)                                                                    = 0;

    virtual CHIP_ERROR GetRangingCapabilities(AttributeValueEncoder & encoder) = 0;

    virtual size_t GetNumActiveSessionIds()                            = 0;
    virtual CHIP_ERROR GetActiveSessionIds(Span<uint8_t> & sessionIds) = 0;

    /// Override if the BLERBC feature is supported.
    virtual std::optional<BleRbcConfig> GetBleRbcConfig() { return std::nullopt; }

    /// Override if the WFUSDPD feature is supported.
    virtual std::optional<WiFiUsdConfig> GetWiFiUsdConfig() { return std::nullopt; }

    /// Override if the BLTCS feature is supported.
    virtual std::optional<BltcsConfig> GetBltcsConfig() { return std::nullopt; }
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
