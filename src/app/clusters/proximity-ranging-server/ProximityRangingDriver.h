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

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/**
 * Driver interface for the Proximity Ranging cluster.
 *
 * The cluster dispatches decoded commands to the driver synchronously.
 * Async events (measurements, unsolicited session stops) are delivered
 * back to the cluster through the Callback interface.
 *
 * The driver is owned by the application and must outlive the cluster.
 * All methods are called from the Matter main thread.
 */
class ProximityRangingDriver
{
public:
    /**
     * Callback for async events from the driver back to the cluster.
     * The cluster implements this and registers itself during Startup().
     */
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
     * Called during cluster Startup(). The callback pointer remains valid
     * until Shutdown() is called.
     */
    virtual CHIP_ERROR Init(Callback & callback) = 0;

    /**
     * Called when the cluster is shutting down. The driver should stop all
     * active ranging sessions and release resources. The application may safely destroy the
     * driver object after this method returns.
     */
    virtual void Shutdown() = 0;

    /**
     * Start a ranging session synchronously.
     *
     * @return ResultCodeEnum indicating whether the session was accepted.
     */
    virtual ResultCodeEnum HandleStartRanging(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request) = 0;

    /**
     * Stop an active ranging session synchronously.
     */
    virtual CHIP_ERROR HandleStopRanging(uint8_t sessionId) = 0;

    /**
     * Get the ranging capabilities supported by this driver.
     */
    virtual CHIP_ERROR GetRangingCapabilities(AttributeValueEncoder & encoder) = 0;

    /**
     * Returns number of active session IDs across all ranging technologies.
     */
    virtual size_t GetNumActiveSessionIds() = 0;

    /**
     * Collect active session IDs across all ranging technologies.
     */
    virtual CHIP_ERROR GetActiveSessionIds(Span<uint8_t> & sessionIds) = 0;

    /**
     * Get the BLE Device ID. Override if the BLERBC feature is supported.
     */
    virtual CHIP_ERROR GetBleDeviceId(uint64_t & bleDeviceId) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }

    /**
     * Get the WiFi Device Identity Key (16 bytes). Override if WFUSDPD is supported.
     */
    virtual CHIP_ERROR GetWiFiDevIK(MutableByteSpan & wifiDevIK) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }

    /**
     * Get the BLT Device Identity Key (16 bytes). Override if BLTCS is supported.
     */
    virtual CHIP_ERROR GetBLTDevIK(MutableByteSpan & bltDevIK) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }

    /**
     * Get the BLTCS Security Level. Override if BLTCS is supported.
     */
    virtual CHIP_ERROR GetBLTCSSecurityLevel(BLTCSSecurityLevelEnum & securityLevel) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }

    /**
     * Get the BLTCS Mode Capability. Override if BLTCS is supported.
     */
    virtual CHIP_ERROR GetBLTCSModeCapability(BLTCSModeEnum & modeCapability) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
