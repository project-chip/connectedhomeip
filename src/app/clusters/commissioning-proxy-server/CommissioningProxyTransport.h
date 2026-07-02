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

#include <app/CommandHandler.h>
#include <app/data-model-provider/OperationTypes.h>
#include <clusters/CommissioningProxy/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

class CommissioningProxyCluster;

/**
 * @brief Platform transport driver for the Commissioning Proxy cluster.
 *
 * This is the ONLY piece an application/platform must supply. Everything that is
 * transport-agnostic — command validation, session-id allocation and the session
 * table, per-fabric isolation, ProxyMessageRequest/Response routing, the
 * background-scan result cache, and multi-transport ProxyScanRequest aggregation —
 * lives in the cluster and its subsystems (see CommissioningProxySessionManager,
 * CommissioningProxyScanCache, CommissioningProxyScanAggregator).
 *
 * One concrete implementation exists per physical transport (BLE, Wi-Fi PAF). The
 * cluster owns a small set of registered transports and dispatches to the one that
 * matches a request's Transport bit. This interface is the *union* of what the BLE
 * and Wi-Fi PAF drivers need; a driver that has no concept of a given parameter
 * (e.g. BLE has no Wi-Fi band) simply ignores it.
 *
 * Call direction:
 *   - cluster → transport: the methods below (Connect, Scan, SendMessage, ...).
 *   - transport → cluster: the driver reports asynchronous results back through the
 *     host cluster it was given in SetHost(), by calling the cluster's subsystem
 *     accessors — Sessions() for session/message bookkeeping, ScanCache() for
 *     background-scan results, and ScanAggregator() for foreground-scan results.
 *
 * Threading: every method is invoked with the Matter stack lock held, and every
 * callback into the host must likewise run on the Matter thread.
 */
class CommissioningProxyTransport
{
public:
    virtual ~CommissioningProxyTransport() = default;

    /// The single Transport bit this driver services (kBle or kWiFiPAF).
    virtual CapabilitiesBitmap GetTransportType() const = 0;

    /**
     * @brief Bind the driver to its host cluster.
     *
     * Called once when the transport is registered with the cluster, and again with
     * nullptr when the cluster is torn down so the driver never dereferences a
     * dangling cluster pointer. The driver reports async results back through this
     * pointer's subsystem accessors.
     */
    virtual void SetHost(CommissioningProxyCluster * cluster) = 0;

    // --- ProxyConnectRequest ------------------------------------------------

    /**
     * @brief Open a transport connection to a commissionable device.
     *
     * On success the driver allocates a session via the host's Sessions() manager,
     * registers it, and calls commandObj->AddResponse() with a ProxyConnectResponse
     * carrying the sessionID (which may be asynchronous). The cluster has already
     * validated the request (single supported transport, discriminator range,
     * WiFiBand vs WI feature, MaxSessions gate) before this is called.
     */
    virtual Protocols::InteractionModel::Status Connect(app::CommandHandler * commandObj, const DataModel::InvokeRequest & request,
                                                        uint16_t discriminator, uint16_t timeout) = 0;

    /**
     * @brief Cancel an in-flight Connect for the given fabric (null-SessionID
     *        ProxyDisconnectRequest). Fails the pending IM exchange.
     * @return InvalidInState if no connect is pending.
     */
    virtual Protocols::InteractionModel::Status CancelPendingConnect(FabricIndex fabricIndex) = 0;

    // --- ProxyDisconnectRequest / ProxyMessageRequest -----------------------

    /// Tear down an established proxy session and release its transport resources.
    virtual Protocols::InteractionModel::Status Disconnect(uint16_t sessionId) = 0;

    /**
     * @brief Forward a Matter packet to the commissionee.
     *
     * The commissionee reply arrives asynchronously; the driver routes it back via
     * host->Sessions().DispatchMessageResponse(). A mid-flight session drop is
     * reported via DispatchMessageFailure().
     */
    virtual CHIP_ERROR SendMessage(uint16_t sessionId, System::PacketBufferHandle && buf) = 0;

    // --- ProxyScanRequest (foreground) --------------------------------------

    /**
     * @brief Start a foreground scan bounded by scanMaxTime seconds.
     *
     * When the scan completes the driver reports its results to the host's
     * ScanAggregator() (empty is valid). The aggregator owns the command handle and
     * emits the single combined ProxyScanResponse once every started transport has
     * contributed.
     */
    virtual Protocols::InteractionModel::Status Scan(uint8_t scanMaxTime) = 0;

    // --- ProxyBackgroundScanStart / Stop ------------------------------------

    /**
     * @brief Start/stop a continuous background scan.
     *
     * Discovered devices are reported to the host's ScanCache() (which owns TTL,
     * MaxCachedResults, and CachedResults/NumCachedResults reporting). @p wiFiBands
     * is ignored by drivers with no band concept (BLE).
     */
    virtual Protocols::InteractionModel::Status BgScanStart(uint16_t timeout, BitMask<WiFiBandBitmap> wiFiBands,
                                                            FabricIndex fabricIndex, NodeId nodeId) = 0;

    /**
     * @brief Stop a background scan. The cluster fans the request's full @p transport
     * mask (and @p wiFiBands) to every registered driver; each matches against its
     * own per-fabric record and returns NotFound if nothing matched, so a driver may
     * partially stop (e.g. one band of several). The command reports NotFound only
     * when no driver matched at all. A band-only request (@p transport == 0) is still
     * offered to every driver.
     */
    virtual Protocols::InteractionModel::Status BgScanStop(BitMask<CapabilitiesBitmap> transport, BitMask<WiFiBandBitmap> wiFiBands,
                                                           FabricIndex fabricIndex, NodeId nodeId) = 0;

    // --- Lifecycle ----------------------------------------------------------

    /**
     * @brief Notified by the cluster when the last proxy session across ALL
     *        transports has closed. A driver may resume state it paused for a
     *        connect (PAF resumes a paused background scan; BLE resumes its own
     *        peripheral advertising). Default: no-op.
     */
    virtual void OnAllSessionsClosed() {}

    /**
     * @brief True while a connect is in flight (exchange open, awaiting
     *        success/error/timeout). The cluster sums this across transports and
     *        adds it to Sessions().ActiveCount() for the MaxSessions gate, so two
     *        concurrent connect attempts cannot both pass.
     */
    virtual bool IsConnectPending() const = 0;

    /**
     * @brief Cancel all outstanding driver state (pending connect, per-fabric
     *        background-scan timers) and stop the hardware scan. Called before the
     *        cluster is destroyed so no timer outlives it. SetHost(nullptr) follows.
     */
    virtual void Shutdown() = 0;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
