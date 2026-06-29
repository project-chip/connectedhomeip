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
#include <lib/support/TimerDelegate.h>
#include <system/SystemClock.h>

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
 *      pointers and a TimerDelegate. The adapter set is fixed for the
 *      driver's lifetime - it reflects the device's physical radio
 *      configuration, which does not change at runtime. The TimerDelegate
 *      drives the driver-owned StartTime / EndTime / RangingInstanceInterval
 *      scheduling described below and must outlive the driver.
 *   3. Passes the driver as a reference to ProximityRangingCluster::Config.
 *
 * Wall-clock ownership
 * --------------------
 * The driver owns ALL wall-clock scheduling for a session. Adapters never see
 * StartTime, EndTime, or RangingInstanceInterval; they are stateless w.r.t.
 * cadence and only ever perform a single ranging instance per StartSession
 * invocation.
 *
 * On HandleStartRanging:
 *   - The driver calls adapter->PrepareSession(sid, params). Non-Accepted
 *     return values are surfaced verbatim to the caller and no session
 *     record is committed.
 *   - On kAccepted, the driver allocates a Session pool slot, marks
 *     SessionIDList dirty (so the sid is visible from acceptance), and arms
 *     the EndTime timer at endTime seconds.
 *   - The driver then arms the per-session NextTrigger timer:
 *       * if startTime == 0, the driver calls adapter->StartSession(sid)
 *         synchronously and arms NextTrigger at rangingInstanceInterval (for
 *         periodic ranging) — instant ranging skips arming.
 *       * if startTime  > 0, the driver arms NextTrigger at startTime;
 *         when the timer fires the driver calls adapter->StartSession(sid)
 *         and (for periodic ranging) re-arms NextTrigger at interval.
 *
 * Periodic ranging cadence is anchored at the moment StartSession is issued:
 * after every adapter->StartSession invocation the driver re-arms NextTrigger
 * at "now + interval", so cadence is exactly `interval` seconds tick-to-tick
 * regardless of when OnMeasurementData arrives. If StartSession returns
 * CHIP_ERROR_BUSY (the adapter is still working on the previous tick's
 * measurement) the driver logs and continues with the existing schedule.
 *
 * Instant ranging (rangingInstanceInterval == nullopt) terminates eagerly:
 * the FIRST OnMeasurementData that satisfies ReportingCondition causes the
 * driver to cancel both timers and call adapter->StopSession(sid). If a
 * measurement is filtered out by ReportingCondition, the driver re-invokes
 * adapter->StartSession(sid) immediately to try again until either a passing
 * measurement arrives or the EndTime cutoff fires (which then surfaces as
 * kPeerNotFound via the existing remap).
 *
 * Adapters therefore never see startTime / endTime / rangingInstanceInterval
 * — only the role / band selectors. See RangingAdapter.h for the full
 * single-shot contract.
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
     * Construct a driver bound to a fixed adapter set and a TimerDelegate.
     *
     * The backing array must outlive the driver. Each pointer must be non-null
     * and each adapter's GetTechnology() must be unique within the set;
     * violations terminate via VerifyOrDie because the adapter set is
     * statically composed and any error is a configuration bug. The
     * TimerDelegate must outlive the driver as well; it is used for the
     * per-session NextTrigger / EndTime timers described in the class comment.
     */
    ProximityRangingDriver(Span<RangingAdapter * const> adapters, TimerDelegate & timerDelegate);

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
    struct Session;

    /// Per-Session TimerContext used for both the initial StartTime delay and
    /// every subsequent rangingInstanceInterval tick (the two are never
    /// armed simultaneously, so a single TimerContext is sufficient). On
    /// fire the driver invokes adapter->StartSession(sid) and re-arms for
    /// the next tick when periodic ranging is requested.
    class NextTriggerTimer : public TimerContext
    {
    public:
        explicit NextTriggerTimer(Session & session) : mSession(session) {}
        void TimerFired() override;

    private:
        Session & mSession;
    };

    /// Per-Session TimerContext that fires when the EndTime cutoff elapses
    /// and the driver must call adapter->StopSession(sid).
    class EndTimer : public TimerContext
    {
    public:
        explicit EndTimer(Session & session) : mSession(session) {}
        void TimerFired() override;

    private:
        Session & mSession;
    };

    struct Session
    {
        Session(uint8_t sid, RangingAdapter & rangingAdapter, ProximityRangingDriver & ownerDriver) :
            id(sid), adapter(&rangingAdapter), owner(&ownerDriver), nextTrigger(*this), endTimer(*this)
        {}

        uint8_t id;
        RangingAdapter * adapter;
        ProximityRangingDriver * owner;

        /// ReportingCondition captured at HandleStartRanging time so the
        /// driver can apply min/max distance and errorMargin filtering on
        /// every OnMeasurementData callback. Set only when the request
        /// carried a ReportingCondition.
        std::optional<Structs::ReportingConditionStruct::Type> reporting;

        /// Periodic cadence in milliseconds; std::nullopt means single-shot
        /// (instant) ranging. Captured from request.trigger.rangingInstanceInterval
        /// and never visible to the adapter.
        std::optional<System::Clock::Milliseconds32> interval;

        /// True once a measurement has passed the driver's reporting filter
        /// and been forwarded to the cluster. The driver remaps an
        /// adapter-supplied kSessionEndTimeReached status to kPeerNotFound
        /// when this stays false through the session's lifetime.
        bool peerFound = false;

        /// True when the requested role is one of the passive-responder
        /// roles (BLEBeacon / WiFiPublisher / BLTReflector). Real radios in
        /// these modes do not produce ranging measurements; only the active
        /// initiator does. The driver therefore SHOULD NOT remap the
        /// adapter-reported kSessionEndTimeReached to kPeerNotFound for
        /// these sessions — the absence of measurements is the expected
        /// outcome of a passive-responder session, not a peer-not-found
        /// condition.
        bool isPassiveResponder = false;

        NextTriggerTimer nextTrigger;
        EndTimer endTimer;
    };

    RangingAdapter * FindAdapter(RangingTechEnum technology) const;
    Session * FindSession(uint8_t sessionId);
    /// Cancel any per-session timers that may still be armed. Safe to call
    /// even when no timer is active.
    void CancelSessionTimers(Session & session);
    /// Release the pool slot for @e sessionId and notify the cluster that
    /// SessionIDList has changed. Returns false if no session matched.
    bool RetireSession(uint8_t sessionId);

    /// Returns true when @e measurement satisfies @e reporting. See the spec:
    /// distance bounds drop the measurement when out of range or when distance
    /// is null while a min/max condition is present; errorMargin > condition
    /// drops the measurement.
    static bool SatisfiesReporting(const Structs::ReportingConditionStruct::Type & reporting,
                                   const Structs::RangingMeasurementDataStruct::Type & measurement);

    /// Invoke adapter->StartSession on the session, log on failure, and
    /// (for periodic ranging) re-arm the NextTrigger timer at now + interval.
    void IssueStartSession(Session & session);

    /// Called when a session's NextTrigger timer fires (either the initial
    /// startTime delay or a periodic-interval tick).
    void OnNextTriggerFired(Session & session);
    /// Called when a session's EndTime cutoff has elapsed; forwards
    /// adapter->StopSession to the owning adapter.
    void OnEndTimerFired(Session & session);

    Span<RangingAdapter * const> mAdapters;
    TimerDelegate & mTimerDelegate;

    ObjectPool<Session, kMaxConcurrentSessions> mSessions;

    Callback * mClusterCallback = nullptr;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
