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

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <system/SystemClock.h>

#include <cstdint>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

/**
 * @brief The Wi-Fi PAF operations a commissioning proxy needs that have no portable form.
 *
 * Opening, sending over and closing a PAF session is already portable: it goes through
 * chip::WiFiPAF::WiFiPAFLayer and the generic ConnectivityManager facade
 * (WiFiPAFSubscribe / WiFiPAFCancelSubscribe / WiFiPAFCancelIncompleteSubscribe).
 * Discovery is not — scanning for commissionable devices, and recovering the subscribe
 * id the platform assigned, both need the platform implementation. Those live here so
 * CommissioningProxyPafTransport stays free of any platform header, and so a test can
 * drive it against a fake.
 *
 * One implementation per platform. The application owns the instance and passes it to
 * the transport, which keeps a reference and never takes ownership.
 */
class CommissioningProxyPafAdapter
{
public:
    /**
     * Fired once per discovered commissionable device, on the Matter event loop, with
     * @p context passed back unchanged. Used by both the foreground and the background
     * scan, so a device looks the same whichever scan found it.
     *
     * Reported as scalars rather than as a platform type: a NAN peer descriptor is
     * platform-specific and typically owns heap storage for its extended data, neither
     * of which belongs in the cluster.
     *
     * @param address       peer NAN MAC.
     * @param extendedData  empty when the advertisement carried none.
     * @param wiFiBand      WiFiBandBitmap value the peer was found on; 0 when unknown.
     *
     * Both spans are only valid for the duration of the call.
     */
    using DiscoveryCallback = void (*)(void * context, ByteSpan address, uint16_t discriminator, uint16_t vendorId,
                                       uint16_t productId, ByteSpan extendedData, uint16_t wiFiBand);

    /// Fired once when a foreground scan started by StartForegroundScan has finished
    /// reporting devices, so the transport knows the result set is complete.
    using ScanCompleteCallback = void (*)(void * context);

    virtual ~CommissioningProxyPafAdapter() = default;

    /**
     * Run a one-shot discovery scan lasting @p window, reporting each peer through
     * @p onDevice and then calling @p onDone exactly once.
     *
     * Unlike BLE, the platform owns the scan window here, so the transport arms no timer
     * of its own and relies on @p onDone to close the scan out.
     *
     * A NAN radio has a single subscribe slot shared with the background scan and with
     * connect, so this reports CHIP_ERROR_BUSY when that slot is already held.
     */
    virtual CHIP_ERROR StartForegroundScan(System::Clock::Seconds16 window, DiscoveryCallback onDevice, ScanCompleteCallback onDone,
                                           void * context) = 0;

    /**
     * Guarantee no further callbacks from a scan started by StartForegroundScan.
     *
     * The platform owns the scan window and may have no way to abort it early, so this is
     * not required to stop the radio — it is required to detach the callbacks, so a scan
     * that completes after the caller has gone cannot reach it. Called from the
     * transport's Shutdown(). A no-op when no foreground scan is outstanding.
     */
    virtual void StopForegroundScan() = 0;

    /**
     * Start a continuous discovery scan, reporting each peer as it is seen — including
     * re-discoveries, so the cluster's cache can refresh a TTL rather than treat the
     * peer as new.
     *
     * Reports CHIP_ERROR_BUSY while the single subscribe slot is held; the
     * background-scan registry treats that as "defer and retry later".
     */
    virtual CHIP_ERROR StartBackgroundScan(DiscoveryCallback cb, void * context) = 0;

    /**
     * Stop a scan started by StartBackgroundScan. A no-op when none is running, so
     * teardown paths can call it unconditionally.
     */
    virtual void StopBackgroundScan() = 0;

    /**
     * The subscribe id the platform assigned to the most recent subscribe request, which
     * the portable WiFiPAFSubscribe() call does not return. The transport needs it to
     * cancel that specific subscribe if the connect fails or is abandoned.
     *
     * Valid only immediately after a successful subscribe; 0 when there is none.
     */
    virtual uint32_t PendingConnectSubscribeId() const = 0;

    /**
     * Disconnect the publish receive handler the platform registered when the proxy
     * began publishing over NAN.
     *
     * A proxy publishes so it can be commissioned onto a fabric itself. Once it is
     * commissioned that handler has to go, or a later subscribe leaves the platform with
     * two handlers for the same traffic. A no-op when none is registered, so the
     * commissioning-complete path can call it unconditionally.
     */
    virtual void DisconnectPublishReceiveHandler() = 0;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
