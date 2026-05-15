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
 * Lifecycle: platform-specific adapters are owned by the application and must
 * be registered with the RangingTechnologyController before the Proximity
 * Ranging cluster is initialized. Adapters must outlive the controller, or be
 * unregistered before destruction.
 *
 * Session model: the cluster allocates 1-byte session IDs and passes them to
 * StartSession. The adapter tracks which IDs are currently active, exposes
 * them via GetActiveSessionIds, and is responsible for reporting termination
 * of every session it has accepted — whether requested via StopSession or
 * driven by the underlying technology — through OnRangingSessionStopped.
 *
 * StartSession / StopSession semantics: both return synchronously with the
 * status of the start/stop request. An adapter MAY return success early if
 * the underlying start takes too long; in that case it must subsequently
 * deliver OnRangingSessionStopped with HardwareError if the session fails to
 * come up.
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
        virtual void OnAttributeChanged(chip::AttributeId attributeId)                                                     = 0;
    };

    virtual ~RangingAdapter() = default;

    /**
     * The callback must remain valid until the adapter is unregistered or destroyed.
     */
    void SetCallback(Callback * callback) { mCallback = callback; }

    virtual RangingTechEnum GetTechnology() const                            = 0;
    virtual Structs::RangingCapabilitiesStruct::Type GetCapabilities() const = 0;

    virtual ResultCodeEnum StartSession(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request) = 0;
    virtual CHIP_ERROR StopSession(uint8_t sessionId)                                                                    = 0;
    virtual void StopAllSessions()                                                                                       = 0;

    virtual CHIP_ERROR GetActiveSessionIds(std::vector<uint8_t> & sessionIds) = 0;

    virtual std::optional<uint64_t> GetDeviceId() { return std::nullopt; }

protected:
    Callback * mCallback = nullptr;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
