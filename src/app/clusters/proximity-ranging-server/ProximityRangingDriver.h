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
#include <app/clusters/proximity-ranging-server/RangingAdapter.h>
#include <clusters/ProximityRanging/Commands.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>
#include <lib/support/Span.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/**
 * Maximum number of ranging sessions that may be active concurrently across
 * all adapters owned by a single ProximityRangingDriver instance. Sets the
 * capacity of the driver's internal session pool: a StartRanging request
 * accepted by its adapter but unable to claim a pool slot is immediately
 * stopped on the adapter and reported back to the cluster as
 * ResultCodeEnum::kBusySessionCapacityReached.
 *
 * The default of 16 is chosen to comfortably cover
 * typical multi-peer deployments while keeping the static pool footprint
 * small. Platforms with stricter memory budgets or higher session demand
 * may override this by defining the macro in their build configuration
 * before including this header (e.g. via a `-D` flag or CHIPProjectConfig).
 */
#ifndef CHIP_CLUSTER_PROXIMITY_RANGING_MAX_CONCURRENT_SESSIONS
#define CHIP_CLUSTER_PROXIMITY_RANGING_MAX_CONCURRENT_SESSIONS 16
#endif

static constexpr size_t kMaxConcurrentSessions = CHIP_CLUSTER_PROXIMITY_RANGING_MAX_CONCURRENT_SESSIONS;

/**
 * Proximity Ranging driver - routes ranging operations between the cluster and
 * a fixed set of per-technology RangingAdapters.
 *
 * The application:
 *   1. Implements one RangingAdapter per supported technology.
 *   2. Constructs a ProximityRangingDriver with a Span over its adapter
 *      pointers. The adapter set is fixed for the driver's lifetime - it
 *      reflects the device's physical radio configuration, which does not
 *      change at runtime.
 *   3. Passes the driver as a reference to ProximityRangingCluster::Config.
 *
 * The driver owns the session→adapter table and forwards async results
 * (measurements, terminations, attribute changes) from adapters to the cluster
 * via the Callback supplied at Init() time. The cluster invokes Init/Shutdown
 * automatically as part of its Startup/Shutdown lifecycle.
 *
 * Threading: cluster-invoked methods run on the Matter main thread. Adapter
 * callbacks (OnMeasurementData / OnRangingSessionStopped / OnAttributeChanged)
 * may arrive from any thread; the driver forwards them under the same
 * threading contract.
 */
class ProximityRangingDriver : public RangingAdapter::Callback
{
public:
    /// Cluster-facing async result sink. Implemented by ProximityRangingCluster.
    class Callback
    {
    public:
        virtual ~Callback() = default;
        virtual void OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement) = 0;
        virtual void OnSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status)                                  = 0;
        virtual void OnAttributeChanged(AttributeId attributeId)                                                           = 0;
    };

    /**
     * Construct a driver bound to a fixed adapter set.
     *
     * The backing array must outlive the driver. Each pointer must be non-null
     * and each adapter's GetTechnology() must be unique within the set;
     * violations terminate via VerifyOrDie because the adapter set is
     * statically composed and any error is a configuration bug.
     */
    ProximityRangingDriver(Span<RangingAdapter * const> adapters);

    ~ProximityRangingDriver() override;

    // Not copyable or movable
    ProximityRangingDriver(const ProximityRangingDriver &)             = delete;
    ProximityRangingDriver & operator=(const ProximityRangingDriver &) = delete;

    /// Called by the cluster from Startup(). The callback must remain valid
    /// until Shutdown() returns.
    CHIP_ERROR Init(Callback & callback);

    /// Called by the cluster from Shutdown(). Stops all active sessions and
    /// clears the cluster callback.
    void Shutdown();

    /**
     * Route a StartRangingRequest to the adapter matching the requested
     * technology. The driver tags the new session with the provided ID; the
     * caller (cluster) is responsible for ID allocation.
     */
    ResultCodeEnum HandleStartRanging(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request);

    /**
     * Route a StopRangingRequest to the adapter that owns the session.
     *
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_NOT_FOUND if no active
     *         session has the requested ID.
     */
    CHIP_ERROR HandleStopRanging(uint8_t sessionId);

    /// Encodes the capabilities of every bound adapter.
    CHIP_ERROR GetRangingCapabilities(AttributeValueEncoder & encoder);

    /// Number of currently-active sessions across all adapters.
    size_t GetNumActiveSessionIds() const;

    /// Fills the caller-supplied span with active session IDs. The span's
    /// capacity must be at least GetNumActiveSessionIds(); on return the
    /// size reflects how many IDs were written.
    CHIP_ERROR GetActiveSessionIds(Span<uint8_t> & out);

    std::optional<BleRbcConfig> GetBleRbcConfig();
    std::optional<WiFiUsdConfig> GetWiFiUsdConfig();
    std::optional<BltcsConfig> GetBltcsConfig();

    // RangingAdapter::Callback
    void OnRangingSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status) override;
    void OnMeasurementData(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement) override;
    void OnAttributeChanged(AttributeId attributeId) override;

private:
    struct Session
    {
        uint8_t id               = 0;
        RangingAdapter * adapter = nullptr;

        /// ReportingCondition captured at HandleStartRanging time so the
        /// driver can apply min/max distance and errorMargin filtering on
        /// every OnMeasurementData callback. Set only when the request
        /// carried a ReportingCondition.
        std::optional<Structs::ReportingConditionStruct::Type> reporting;

        /// True once a measurement has passed the driver's reporting filter
        /// and been forwarded to the cluster. The driver remaps an
        /// adapter-supplied kSessionEndTimeReached status to kPeerNotFound
        /// when this stays false through the session's lifetime.
        bool peerFound = false;
    };

    RangingAdapter * FindAdapter(RangingTechEnum technology) const;
    Session * FindSession(uint8_t sessionId);
    /// Release the pool slot for @e sessionId and notify the cluster that
    /// SessionIDList has changed. Returns false if no session matched.
    bool RetireSession(uint8_t sessionId);

    /// Returns true when @e measurement satisfies @e reporting. See the spec:
    /// distance bounds drop the measurement when out of range or when distance
    /// is null while a min/max condition is present; errorMargin > condition
    /// drops the measurement.
    static bool SatisfiesReporting(const Structs::ReportingConditionStruct::Type & reporting,
                                   const Structs::RangingMeasurementDataStruct::Type & measurement);

    Span<RangingAdapter * const> mAdapters;

    ObjectPool<Session, kMaxConcurrentSessions> mSessions;

    Callback * mClusterCallback = nullptr;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
