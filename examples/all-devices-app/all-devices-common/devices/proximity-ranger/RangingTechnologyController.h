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
#include <lib/core/CHIPError.h>

#include <vector>

#include "RangingAdapter.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/**
 * Maximum number of adapters which the controller allows to register.
 * Implementers should ensure this is set to the number of ranging technologies the
 * device plans to support.
 */
static constexpr uint8_t kMaxControllerAdapters = 2;

/**
 * Routes ranging sessions to the appropriate technology adapter based on the
 * requested technology. Maintains a registry of adapters (one per technology)
 * and tracks which adapter owns each active session.
 *
 * The application creates the controller and adapters. The controller holds
 * non-owning references to adapters, which must outlive the controller.
 * All methods are called from the Matter main thread.
 */
class RangingTechnologyController : public RangingAdapter::Callback
{
public:
    /** Receives async results from adapters, forwarded to the driver. */
    class Listener
    {
    public:
        virtual ~Listener() = default;

        virtual void OnSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status)                                     = 0;
        virtual void OnSessionMeasurement(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement) = 0;
        virtual void OnAttributeChanged(chip::AttributeId attributeId)                                                        = 0;
    };

    RangingTechnologyController() = default;
    ~RangingTechnologyController();

    /**
     * Add a listener that receives async results from adapters.
     * Multiple listeners are supported for multi-endpoint configurations.
     *
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_ARGUMENT if null,
     *         CHIP_ERROR_NO_MEMORY if max listeners reached.
     */
    CHIP_ERROR AddListener(Listener * listener);

    /**
     * Remove a previously added listener.
     */
    void RemoveListener(Listener * listener);

    /**
     * Register a technology adapter. Only one adapter per technology is allowed.
     * The controller holds a non-owning reference; the adapter must outlive the
     * controller or be explicitly unregistered before destruction.
     *
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_NO_MEMORY if full,
     *         CHIP_ERROR_DUPLICATE_KEY_ID if technology already registered.
     */
    CHIP_ERROR RegisterAdapter(RangingAdapter & adapter);

    /**
     * Unregister a previously registered adapter. Stops all sessions owned by
     * the adapter and removes it from the registry. Must be called before the
     * adapter is destroyed if the controller is still alive.
     *
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_NOT_FOUND if not registered.
     */
    CHIP_ERROR UnregisterAdapter(RangingAdapter & adapter);

    /**
     * Encode the capabilities from all registered adapters.
     */
    CHIP_ERROR GetRangingCapabilities(AttributeValueEncoder & encoder);

    /**
     * Route a StartRangingRequest to the appropriate adapter based on the
     * Technology field. Tracks the session→adapter mapping.
     */
    ResultCodeEnum StartSession(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request);

    /**
     * Route a StopRangingRequest to the adapter that owns the session.
     */
    CHIP_ERROR StopSession(uint8_t sessionId);

    /**
     * Stop all active sessions and clear state.
     */
    void StopAllSessions();

    /**
     * Returns the number of active sessions across all adapters
     */
    size_t GetNumActiveSessionIds() const { return mSessions.size(); }

    CHIP_ERROR GetActiveSessionIds(chip::Span<uint8_t> & sessionIds);

    // RangingAdapter::Callback implementation
    void OnRangingSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status) override;
    void OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement) override;
    void OnAttributeChanged(chip::AttributeId attributeId) override;

    uint64_t GetBleDeviceId();

private:
    RangingAdapter * FindAdapter(RangingTechEnum technology);

    struct SessionEntry
    {
        uint8_t sessionId        = 0;
        RangingAdapter * adapter = nullptr;
    };

    RangingAdapter * mAdapters[kMaxControllerAdapters] = {};
    size_t mAdapterCount                               = 0;
    std::vector<SessionEntry> mSessions;

    static constexpr size_t kMaxListeners = 4;
    Listener * mListeners[kMaxListeners]  = {};
    size_t mListenerCount                 = 0;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
