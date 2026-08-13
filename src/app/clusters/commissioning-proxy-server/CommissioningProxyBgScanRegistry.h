/*
 *
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

#include <clusters/CommissioningProxy/Enums.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <lib/support/TimerDelegate.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cstdint>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

/**
 * @brief Transport-agnostic background-scan fabric registry.
 *
 * Owns everything ProxyBackGroundScanStart/StopRequest needs that does not depend
 * on the physical transport: the per-fabric scan requests (transport + Wi-Fi bands
 * + lifetime timer), the spec transport/band overlap arithmetic on Stop, and the
 * paused/deferred state shared by every transport while the radio is held by a
 * connect or foreground scan.
 *
 * One instance per transport (owned by the transport). The owning transport
 * supplies the only transport-specific parts via HardwareControl. All calls run on
 * the Matter thread with the stack lock held.
 */
class CommissioningProxyBgScanRegistry
{
public:
    /**
     * @brief Hooks the owning transport implements. These are the ONLY
     * transport-specific parts of background scanning.
     */
    class HardwareControl
    {
    public:
        virtual ~HardwareControl() = default;

        /**
         * Start (or resume) the hardware background scan, wiring the transport's own
         * discovery callback.
         *   - CHIP_NO_ERROR:   the scan is running.
         *   - CHIP_ERROR_BUSY: the radio is currently held (BLE: the scanner is owned
         *                      by a connect or foreground scan; PAF: a ProxyConnect
         *                      owns the single NAN subscribe slot). The registry keeps
         *                      the fabrics registered, stays paused, and retries on the
         *                      next ResumeIfNeeded().
         *   - other error:     hard failure; the triggering Start() is rejected.
         */
        virtual CHIP_ERROR StartHardwareScan() = 0;

        /**
         * Stop the hardware background scan. Called only when the registry currently
         * owns the radio (i.e. not while paused).
         */
        virtual void StopHardwareScan() = 0;

        /**
         * Drop cached scan results for this transport
         * (host->ScanCache().ClearTransport(<this transport>, bands)). @p bands == 0
         * means the transport stopped entirely, so all of its results go; otherwise
         * only those bands stopped and only their results go. BLE always receives 0.
         */
        virtual void ClearCachedResults(BitMask<WiFiBandBitmap> bands) = 0;
    };

    CommissioningProxyBgScanRegistry(HardwareControl & hardware, TimerDelegate & timerDelegate) :
        mHardware(hardware), mTimerDelegate(timerDelegate)
    {}
    ~CommissioningProxyBgScanRegistry() { Shutdown(); }

    CommissioningProxyBgScanRegistry(const CommissioningProxyBgScanRegistry &)             = delete;
    CommissioningProxyBgScanRegistry & operator=(const CommissioningProxyBgScanRegistry &) = delete;

    /**
     * ProxyBackGroundScanStartRequest. Several nodes on a fabric may scan at once: the
     * spec identifies each by NodeID + FabricID, so every node keeps its own request and
     * the fabric scans the union of them. A repeat request from the same node replaces
     * that node's own request only.
     *
     * The fabric holds one lifetime timer, set to the latest deadline of its requests;
     * @p timeoutSecs == 0 means that node never expires, which suppresses the timer
     * entirely. A rejected Start changes nothing.
     *
     * @return RESOURCE_EXHAUSTED once the fabric holds
     *         CHIP_CONFIG_COMMISSIONING_PROXY_MAX_BGSCAN_REQUESTS_PER_FABRIC requests.
     *
     * @p transport SHALL carry only the owning transport's own bit; Stop() relies on it.
     */
    Protocols::InteractionModel::Status Start(FabricIndex fabricIndex, NodeId nodeId, BitMask<CapabilitiesBitmap> transport,
                                              BitMask<WiFiBandBitmap> wiFiBands, uint16_t timeoutSecs);

    /**
     * ProxyBackGroundScanStopRequest. Narrows or drops the requesting node's own
     * request: a transport bitmap of zero means "stop only the listed bands". Scanning
     * only really stops for what no remaining request still covers. SUCCESS is returned
     * even when nothing overlapped; NOT_FOUND when this node has no request on this
     * fabric.
     */
    Protocols::InteractionModel::Status Stop(FabricIndex fabricIndex, NodeId nodeId, BitMask<CapabilitiesBitmap> transport,
                                             BitMask<WiFiBandBitmap> wiFiBands);

    /**
     * Suspend the hardware scan because the radio is needed for a connect or
     * foreground scan. Idempotent; keeps the fabrics registered.
     */
    void Pause();

    /**
     * Resume a paused hardware scan if fabrics remain and the radio is free. Safe to
     * call from any "radio freed" path; the transport must wrap this in ScheduleWork
     * if it could otherwise run re-entrantly.
     */
    void ResumeIfNeeded();

    bool IsEmpty() const { return !AnyFabricInUse(); }
    bool IsPaused() const { return mPaused; }

    /**
     * Drop every request @p fabricIndex owns, as though it had stopped them all. Used
     * when the fabric is removed; results for bands nobody scans any more are cleared
     * and the radio stops once no request is left.
     */
    void RemoveFabric(FabricIndex fabricIndex);

    /** Cancel every lifetime timer and stop the hardware scan if the registry owns it. */
    void Shutdown();

private:
    // Context for the per-fabric lifetime timer, so the expiry can find its registry
    // and fabric without a global. Each fabric slot owns one inline, so fabrics with
    // different lifetimes expire independently with no allocation.
    struct LifetimeCtx : public TimerContext
    {
        CommissioningProxyBgScanRegistry * registry = nullptr;
        FabricIndex fabricIndex                     = kUndefinedFabricIndex;
        bool armed                                  = false;

        void TimerFired() override { registry->OnLifetimeExpiry(fabricIndex); }
    };

    // One ProxyBackGroundScanStartRequest, owned by the node that sent it.
    struct Request
    {
        BitMask<CapabilitiesBitmap> transport;
        BitMask<WiFiBandBitmap> wiFiBands;
        System::Clock::Timestamp expiresAt; // only meaningful when hasTimeout
        bool hasTimeout = false;            // Timeout == 0 means "until an explicit Stop"
    };

    // One node's request within a fabric. `inUse` false marks a free slot.
    struct RequestSlot
    {
        bool inUse = false;
        NodeId nodeId;
        Request request;
    };

    // Every request from one fabric, plus that fabric's single lifetime timer.
    struct FabricState
    {
        bool inUse = false;
        FabricIndex fabricIndex;
        RequestSlot requests[CHIP_CONFIG_COMMISSIONING_PROXY_MAX_BGSCAN_REQUESTS_PER_FABRIC];
        LifetimeCtx lifetime;

        uint8_t RequestCount() const;
        RequestSlot * Find(NodeId nodeId);
        RequestSlot * FindFree();
    };

    FabricState * FindFabric(FabricIndex fabricIndex);
    bool AnyFabricInUse() const;

    /// By value on purpose: the caller passes its own LifetimeCtx member, which this
    /// deletes. A reference would dangle for the rest of the body.
    void OnLifetimeExpiry(FabricIndex fabricIndex);
    void CancelLifetime(FabricState & state);
    void OnBecameEmpty(); // stop hardware if owned, then clear cache

    /// Latest deadline among @p state's requests. Returns false when some request has no
    /// timeout, in which case the fabric must not hold a timer at all.
    static bool LatestDeadline(const FabricState & state, System::Clock::Timestamp & out);

    /// Re-arm (or drop) @p fabricIndex's timer after its request set changed — a Stop can
    /// shorten the fabric back to a surviving request's deadline.
    void RecomputeFabricLifetime(FabricIndex fabricIndex, FabricState & state);

    /// Union of the bands every remaining request wants, across all fabrics.
    BitMask<WiFiBandBitmap> BandsInUse() const;

    /// Of @p candidates, drop the cached results for those no request still covers.
    void ClearBandsNoLongerScanned(BitMask<WiFiBandBitmap> candidates);

    FabricState mFabrics[CHIP_CONFIG_MAX_FABRICS];
    bool mPaused = false;
    HardwareControl & mHardware;
    TimerDelegate & mTimerDelegate;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
