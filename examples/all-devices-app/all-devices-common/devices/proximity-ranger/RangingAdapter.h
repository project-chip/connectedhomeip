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

#include <optional>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/**
 * Interface for a single ranging technology (BLE RSSI, Channel Sounding, Wi-Fi USD, etc.).
 *
 * Platform-specific adapters implement this and are registered with a
 * RangingTechnologyController. Adapters must outlive the controller.
 * All methods are called from the Matter main thread.
 */
class RangingAdapter
{
public:
    /** Receives async results from ranging sessions. */
    class Callback
    {
    public:
        virtual ~Callback() = default;

        virtual void OnRangingSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status)                           = 0;
        virtual void OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement) = 0;
        virtual void OnAttributeChanged(chip::AttributeId attributeId)                                                     = 0;
    };

    virtual ~RangingAdapter() = default;

    /**
     * Set the callback for async event delivery (measurements, session stops).
     * Called by RangingTechnologyController during RegisterAdapter.
     * The callback must remain valid until the adapter is unregistered or destroyed.
     */
    void SetCallback(Callback * callback) { mCallback = callback; }

    /**
     * Returns the ranging technology this adapter implements.
     */
    virtual RangingTechEnum GetTechnology() const = 0;

    /**
     * Returns the ranging capabilities this adapter supports.
     */
    virtual Structs::RangingCapabilitiesStruct::Type GetCapabilities() const = 0;

    /**
     * Start a ranging session synchronously. Measurement data and unsolicited
     * session stops are still delivered asynchronously via the Callback
     * registered with SetCallback.
     */
    virtual ResultCodeEnum StartSession(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request) = 0;

    /**
     * Stop an active ranging session synchronously.
     */
    virtual CHIP_ERROR StopSession(uint8_t sessionId) = 0;

    /**
     * Stop all active ranging sessions.
     */
    virtual void StopAllSessions() = 0;

    /**
     * Collect active session IDs managed by this adapter.
     */
    virtual CHIP_ERROR GetActiveSessionIds(std::vector<uint8_t> & sessionIds) = 0;

    /**
     * Get the unique Device ID for this adapter if supported.
     */
    virtual std::optional<uint64_t> GetDeviceId() { return std::nullopt; }

protected:
    Callback * mCallback = nullptr;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
