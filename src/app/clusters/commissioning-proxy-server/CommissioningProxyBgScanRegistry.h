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
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemLayer.h>

#include <cstdint>
#include <map>

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
         * Drop this transport's cached scan results
         * (host->ScanCache().ClearTransport(<this transport>)). Called whenever the
         * last fabric is removed, whether or not the hardware scan was running.
         */
        virtual void ClearCachedResults() = 0;
    };

    explicit CommissioningProxyBgScanRegistry(HardwareControl & hardware) : mHardware(hardware) {}
    ~CommissioningProxyBgScanRegistry() { Shutdown(); }

    CommissioningProxyBgScanRegistry(const CommissioningProxyBgScanRegistry &)             = delete;
    CommissioningProxyBgScanRegistry & operator=(const CommissioningProxyBgScanRegistry &) = delete;

    /**
     * ProxyBackGroundScanStartRequest for one fabric. @p timeoutSecs == 0 means no
     * lifetime timer (scan until an explicit Stop). Re-registering an existing
     * (fabricIndex, nodeId) refreshes its transport/bands and restarts its lifetime
     * timer.
     */
    Protocols::InteractionModel::Status Start(FabricIndex fabricIndex, NodeId nodeId, BitMask<CapabilitiesBitmap> transport,
                                              BitMask<WiFiBandBitmap> wiFiBands, uint16_t timeoutSecs);

    /**
     * ProxyBackGroundScanStopRequest. Applies the spec transport/band semantics: a
     * transport bitmap of zero means "stop only the listed bands"; a fabric with no
     * transports or no bands left is removed; SUCCESS is returned even when nothing
     * overlapped, and NOT_FOUND when the fabric has no record.
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

    bool IsEmpty() const { return mFabrics.empty(); }
    bool IsPaused() const { return mPaused; }

    /** Cancel every lifetime timer and stop the hardware scan if the registry owns it. */
    void Shutdown();

private:
    struct FabricKey
    {
        FabricIndex fabricIndex;
        NodeId nodeId;
        bool operator<(const FabricKey & o) const;
    };

    // Heap context handed to the per-fabric lifetime timer so the callback can find
    // its registry and key without a global.
    struct LifetimeCtx
    {
        CommissioningProxyBgScanRegistry * registry;
        FabricKey key;
    };

    struct Record
    {
        BitMask<CapabilitiesBitmap> transport;
        BitMask<WiFiBandBitmap> wiFiBands;
        LifetimeCtx * lifetimeCtx = nullptr;
    };

    static void LifetimeExpiryCallback(System::Layer * layer, void * appState);
    void OnLifetimeExpiry(const FabricKey & key);
    void CancelLifetime(Record & rec);
    void OnBecameEmpty(); // stop hardware if owned, then clear cache

    std::map<FabricKey, Record> mFabrics;
    bool mPaused = false;
    HardwareControl & mHardware;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
