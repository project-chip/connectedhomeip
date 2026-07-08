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
#include "CommissioningProxyPafTransport.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBgScanRegistry.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/Linux/ConnectivityManagerImpl.h>
#include <platform/PlatformManager.h>
#include <wifipaf/WiFiPAFLayer.h>
#include <wifipaf/WiFiPAFLayerDelegate.h>
#include <wifipaf/WiFiPAFRole.h>

#include <map>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {
namespace Paf {

// Defined below; called during the receive-delegate/timeout teardown paths, which
// appear before the definition.
void OnAllSessionsClosed();

namespace {

using ScanResultT = chip::app::Clusters::CommissioningProxy::Structs::ScanResultStruct::Type;

// ------------------------------------------------------------------
// Per-sessionId PAF state.  Owned by this module; the dispatcher's
// sProxySessions only tracks (transport, fabric).
// ------------------------------------------------------------------
static std::map<uint16_t, chip::WiFiPAF::WiFiPAFSession> sSessions;

// Context for the async WiFiPAF connection initiated by ProxyConnectRequest.
struct ConnectCtx
{
    chip::app::CommandHandler::Handle handle;
    chip::app::ConcreteCommandPath path;
    uint16_t discriminator;
    uint32_t subscribeId; // NAN subscribe_id from wpa_supplicant, stored after WiFiPAFSubscribe
    chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * cluster = nullptr;
    chip::FabricIndex fabricIndex                                                = chip::kUndefinedFabricIndex;
};

// Owning pointer to the in-flight PAF ProxyConnect.
// Nulled by whichever callback fires first (success / error / timeout) so any
// later callback for the same subscribe can detect it has already been handled
// and avoid use-after-free.
static ConnectCtx * sPendingConnect = nullptr;

// Host cluster (set via the transport's SetHost). All transport-agnostic
// bookkeeping (sessions, message routing, scan cache/aggregation) is reached
// through its subsystem accessors.
static chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * sHost = nullptr;

// True while a ProxyScanRequest is in progress; used to return Busy for concurrent requests.
static bool sScanInProgress = false;

// ------------------------------------------------------------------
// Background scan (PAF).
//
// The transport-agnostic per-fabric bookkeeping (records, lifetime timers,
// Start/Stop transport/band arithmetic, paused/deferred state) lives in the
// shared CommissioningProxyBgScanRegistry; this transport supplies only the PAF
// hardware start/stop/clear hooks (PafBgScanHardware below).  The single NAN
// subscribe slot is shared with the connect subscribe, so StartHardwareScan
// reports BUSY while a ProxyConnect is in flight and the registry defers/resumes.
// ------------------------------------------------------------------

static ScanResultT MakeScanResult(const chip::DeviceLayer::NanPeerInfo & p)
{
    ScanResultT r{};
    r.address.SetNonNull(chip::ByteSpan(p.mac, sizeof(p.mac)));
    r.transport = chip::BitMask<chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap>(
        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap::kWiFiPAF);
    r.discriminator = p.discriminator;
    r.vendorID      = static_cast<chip::VendorId>(p.vid);
    r.productID     = p.pid;
    if (p.hasExtendedData && !p.storage.empty())
    {
        r.extendedData.SetNonNull(chip::ByteSpan(p.storage.data(), p.storage.size()));
    }
    else
    {
        r.extendedData.SetNull();
    }
    if (p.band != 0)
    {
        r.wiFiBand.SetValue(static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(p.band));
    }
    else
    {
        r.wiFiBand.ClearValue();
    }
    return r;
}

static void OnBgScanDiscovery(void * /*ctx*/, const chip::DeviceLayer::NanPeerInfo & peer)
{
    // Runs on the Matter event loop (wpa_supplicant NAN callbacks are dispatched
    // there), so the shared cache may be touched directly.
    if (sHost != nullptr)
    {
        sHost->ScanCache().Report(MakeScanResult(peer));
    }
}

// ------------------------------------------------------------------
// Background-scan hardware hooks + shared registry.
//
// The registry owns the per-fabric records, lifetime timers, and paused state;
// this HardwareControl supplies only the PAF-specific start/stop/clear.  The
// single NAN subscribe slot is shared with the connect subscribe, so
// StartHardwareScan reports BUSY while a ProxyConnect is in flight — the registry
// treats that as "defer and resume later" (its paused state).
// ------------------------------------------------------------------
class PafBgScanHardware : public CommissioningProxyBgScanRegistry::HardwareControl
{
public:
    CHIP_ERROR StartHardwareScan() override
    {
        if (sPendingConnect != nullptr)
        {
            // A ProxyConnect owns the single NAN subscribe slot; defer.
            return CHIP_ERROR_BUSY;
        }
        return chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStartBackgroundScan(OnBgScanDiscovery, nullptr);
    }
    void StopHardwareScan() override { chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStopBackgroundScan(); }
    void ClearCachedResults() override
    {
        if (sHost != nullptr)
        {
            sHost->ScanCache().ClearTransport(CapabilitiesBitmap::kWiFiPAF);
        }
    }
};

// Declared before the registry so it outlives it (the registry's destructor may
// call back into these hooks).
static PafBgScanHardware sPafBgScanHardware;
static CommissioningProxyBgScanRegistry sBgScan(sPafBgScanHardware);

// ------------------------------------------------------------------
// WiFiPAFLayerDelegate wrapper.
//
// Wraps the original mWiFiPAFTransport so that incoming PAFTP messages for
// proxy sessions are routed back to the commissioner as ProxyMessageResponse
// (via the dispatcher) rather than being injected into the proxy app's own
// Matter transport stack.
// ------------------------------------------------------------------
class ProxyWiFiPAFDelegate : public chip::WiFiPAF::WiFiPAFLayerDelegate
{
public:
    void Install()
    {
        auto & layer = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
        if (layer.mWiFiPAFTransport == this)
        {
            return;
        }
        mOriginalTransport      = layer.mWiFiPAFTransport;
        layer.mWiFiPAFTransport = this;
        ChipLogProgress(AppServer, "ProxyWiFiPAFDelegate: installed (original=%p)", (void *) mOriginalTransport);
    }

    void Uninstall()
    {
        auto & layer = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
        if (layer.mWiFiPAFTransport == this)
        {
            layer.mWiFiPAFTransport = mOriginalTransport;
            ChipLogProgress(AppServer, "ProxyWiFiPAFDelegate: uninstalled");
        }
        mOriginalTransport = nullptr;
    }

    CHIP_ERROR WiFiPAFMessageReceived(chip::WiFiPAF::WiFiPAFSession & RxInfo, chip::System::PacketBufferHandle && msg) override
    {
        for (auto & [sid, sess] : sSessions)
        {
            if (sess.peer_id == RxInfo.peer_id)
            {
                if (sHost != nullptr)
                {
                    sHost->Sessions().DispatchMessageResponse(sid, msg->Start(), msg->DataLength());
                }
                return CHIP_NO_ERROR;
            }
        }
        if (mOriginalTransport != nullptr)
        {
            return mOriginalTransport->WiFiPAFMessageReceived(RxInfo, std::move(msg));
        }
        ChipLogError(AppServer, "ProxyWiFiPAFDelegate: no original transport for non-proxy session");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR WiFiPAFMessageSend(chip::WiFiPAF::WiFiPAFSession & TxInfo, chip::System::PacketBufferHandle && msg) override
    {
        if (mOriginalTransport != nullptr)
        {
            return mOriginalTransport->WiFiPAFMessageSend(TxInfo, std::move(msg));
        }
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR WiFiPAFCloseSession(chip::WiFiPAF::WiFiPAFSession & SessionInfo) override
    {
        // The PAF session closed on its own — e.g. the ED dropped NAN when
        // ConnectNetwork switched it to infrastructure WiFi, so the PAFTP
        // ack-received timer fired and the layer is closing the session here.
        // There is no ProxyDisconnectRequest, so this handler must run the same
        // teardown the dispatcher would on disconnect: fail any in-flight
        // ProxyMessage, drop the session from every map, terminate the NAN
        // subscribe, and resume a background scan that was paused for the connect.
        // (Mirrors the BLE transport's OnEndPointConnectionClosed handler.)
        uint16_t sid                             = 0;
        bool found                               = false;
        chip::WiFiPAF::WiFiPAFSession pafSession = {};
        for (auto & [id, sess] : sSessions)
        {
            if (sess.peer_id == SessionInfo.peer_id)
            {
                sid        = id;
                pafSession = sess;
                found      = true;
                break;
            }
        }
        if (found)
        {
            ChipLogError(AppServer, "WiFiPAFCloseSession: PAF session for proxy session %u closed by peer — cleaning up", sid);
            // Resolve any in-flight ProxyMessageRequest so the commissioner gets a
            // timely error instead of waiting out its 30 s timeout.
            if (sHost != nullptr)
            {
                sHost->Sessions().DispatchMessageFailure(sid, chip::Protocols::InteractionModel::Status::Failure);
            }
            sSessions.erase(sid);
            if (sHost != nullptr)
            {
                CHIP_ERROR stateErr =
                    sHost->SetCPState(chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster::kState_CPDisconnected);
                if (stateErr != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "WiFiPAFCloseSession: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
                }
            }
            chip::WiFiPAF::WiFiPAFLayer & layer = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
            (void) layer.RmPafSession(chip::WiFiPAF::PafInfoAccess::kAccSessionId, pafSession);
            // Terminate the NAN subscribe (the subscriber session id IS the
            // subscribe_id) so it does not linger in wpa_supplicant — same leak
            // the ProxyDisconnect path guards against.  Do NOT CloseEndPoint here:
            // the layer is already closing the endpoint (that is why we were called).
            if (pafSession.id != 0)
            {
                (void) chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(pafSession.id);
            }
            if (sHost != nullptr)
            {
                sHost->Sessions().RemoveSession(sid);
            }
            // This path does not run through the dispatcher's OnAllSessionsClosed(),
            // so resume a bg-scan paused for the connect once the last session is
            // gone and no connect is mid-flight.
            if (sSessions.empty() && sPendingConnect == nullptr)
            {
                OnAllSessionsClosed();
            }
        }
        if (mOriginalTransport != nullptr)
        {
            return mOriginalTransport->WiFiPAFCloseSession(SessionInfo);
        }
        return CHIP_NO_ERROR;
    }

    bool WiFiPAFResourceAvailable() override
    {
        if (mOriginalTransport != nullptr)
        {
            return mOriginalTransport->WiFiPAFResourceAvailable();
        }
        return true;
    }

private:
    chip::WiFiPAF::WiFiPAFLayerDelegate * mOriginalTransport = nullptr;
};

static ProxyWiFiPAFDelegate sProxyWiFiPAFDelegate;

// ------------------------------------------------------------------
// PAF connect callbacks (success / error / timeout)
// ------------------------------------------------------------------

static void OnConnectTimeout(chip::System::Layer * layer, void * appState);

// Common teardown for an in-flight PAF connect that did not succeed (timeout,
// transport error, or commissioner-initiated cancel).  Cancels the timeout timer
// and NAN subscribe, closes any PAFTP endpoint the handshake created, drops the
// session, resolves the originating ProxyConnectRequest with `status`, frees the
// context, and resumes a background scan that was deferred/paused for the connect.
// `cancelTimer` is false only when called from the timeout handler itself (the
// timer has already fired).
static void FailPendingConnect(ConnectCtx * ctx, chip::Protocols::InteractionModel::Status status, bool cancelTimer)
{
    sPendingConnect = nullptr;

    if (cancelTimer)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(OnConnectTimeout, nullptr);
    }

    // Null the platform connect callbacks before cancelling the subscribe so the
    // kCHIPoWiFiPAFCancelConnect event posted by OnNanSubscribeTerminated does not
    // re-enter OnConnectError.
    CHIP_ERROR cancelIncompleteErr = chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelIncompleteSubscribe();
    if (cancelIncompleteErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "FailPendingConnect: WiFiPAFCancelIncompleteSubscribe: %" CHIP_ERROR_FORMAT,
                      cancelIncompleteErr.Format());
    }

    chip::WiFiPAF::WiFiPAFLayer & pafLayer = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    chip::WiFiPAF::WiFiPAFSession key{};
    key.nodeId        = static_cast<chip::NodeId>(ctx->discriminator);
    key.discriminator = ctx->discriminator;

    // Capture the session before RmPafSession clears the slot so we can close any
    // PAFTP endpoint the handshake created; otherwise it leaks from the 2-slot
    // pool until its own timer self-closes.  CloseEndPoint is a no-op if none.
    chip::WiFiPAF::WiFiPAFSession endpointSession{};
    bool haveEndpoint                     = false;
    chip::WiFiPAF::WiFiPAFSession * pInfo = pafLayer.GetPAFInfo(chip::WiFiPAF::PafInfoAccess::kAccDisc, key);
    if (pInfo != nullptr)
    {
        endpointSession = *pInfo;
        haveEndpoint    = true;
    }

    CHIP_ERROR rmErr = pafLayer.RmPafSession(chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, key);
    if (rmErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "FailPendingConnect: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr.Format());
    }
    if (haveEndpoint)
    {
        pafLayer.CloseEndPoint(endpointSession);
    }

    if (ctx->subscribeId != 0)
    {
        CHIP_ERROR cancelErr = chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(ctx->subscribeId);
        if (cancelErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "FailPendingConnect: WiFiPAFCancelSubscribe(%u): %" CHIP_ERROR_FORMAT, ctx->subscribeId,
                          cancelErr.Format());
        }
    }

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
    {
        cmd->AddStatus(ctx->path, status);
    }
    delete ctx;

    // The connect freed the NAN subscribe slot; resume a background scan that was
    // deferred or paused for this connect (a failed connect registers no session,
    // so the dispatcher's OnAllSessionsClosed path does not run).
    OnAllSessionsClosed();
}

static void OnConnectTimeout(chip::System::Layer * /*layer*/, void * /*appState*/)
{
    if (sPendingConnect == nullptr)
    {
        return; // Success or error callback already fired first; nothing to do.
    }
    ChipLogError(AppServer, "ProxyConnectRequest: timeout waiting for WiFiPAF connect (disc %u)", sPendingConnect->discriminator);
    FailPendingConnect(sPendingConnect, chip::Protocols::InteractionModel::Status::Timeout, /*cancelTimer=*/false);
}

static void OnConnectSuccess(void * /*context*/)
{
    auto * ctx = sPendingConnect;
    if (ctx == nullptr)
    {
        ChipLogProgress(AppServer, "OnConnectSuccess: no pending connect ctx; ignoring stale callback");
        return;
    }
    sPendingConnect = nullptr;
    chip::DeviceLayer::SystemLayer().CancelTimer(OnConnectTimeout, nullptr);

    chip::app::CommandHandler * cmd = ctx->handle.Get();

    chip::WiFiPAF::WiFiPAFSession keyInfo{};
    keyInfo.discriminator                    = ctx->discriminator;
    chip::WiFiPAF::WiFiPAFLayer & layer      = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    chip::WiFiPAF::WiFiPAFSession * pPafInfo = layer.GetPAFInfo(chip::WiFiPAF::PafInfoAccess::kAccDisc, keyInfo);

    if (cmd == nullptr || pPafInfo == nullptr)
    {
        ChipLogError(AppServer, "OnConnectSuccess: cmd=%p pPafInfo=%p for disc %u", (void *) cmd, (void *) pPafInfo,
                     ctx->discriminator);
        // The connect completed but the session cannot be handed to the commissioner
        // (originating exchange gone, or the PAF session vanished).  Run the full
        // teardown so an established-but-untracked session does not leak its PAF pool
        // slot, PAFTP endpoint, and NAN subscribe.  FailPendingConnect adds a Failure
        // status if the command handler is still alive, frees ctx, and resumes any
        // background scan deferred for this connect.  (The timeout timer was already
        // cancelled above, so cancelTimer=false.)
        FailPendingConnect(ctx, chip::Protocols::InteractionModel::Status::Failure, /*cancelTimer=*/false);
        return;
    }

    uint16_t sessionId   = ctx->cluster->Sessions().AllocSessionId();
    sSessions[sessionId] = *pPafInfo;
    ctx->cluster->Sessions().RegisterSession(sessionId, CapabilitiesBitmap::kWiFiPAF, ctx->fabricIndex);

    ChipLogProgress(AppServer, "ProxyConnectRequest: WiFiPAF connected, proxy session %u (disc %u peer_id %u)", sessionId,
                    ctx->discriminator, pPafInfo->peer_id);

    if (ctx->cluster)
    {
        CHIP_ERROR stateErr =
            ctx->cluster->SetCPState(chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster::kState_CPConnected);
        if (stateErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "OnConnectSuccess: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
        }
    }

    chip::app::Clusters::CommissioningProxy::Commands::ProxyConnectResponse::Type response;
    response.sessionID = sessionId;
    cmd->AddResponse(ctx->path, response);
    delete ctx;
}

static void OnConnectError(void * /*context*/, CHIP_ERROR err)
{
    auto * ctx = sPendingConnect;
    if (ctx == nullptr)
    {
        ChipLogProgress(AppServer,
                        "OnConnectError: ignoring stale callback after successful connect "
                        "(err: %" CHIP_ERROR_FORMAT ")",
                        err.Format());
        return;
    }
    ChipLogError(AppServer, "ProxyConnectRequest: WiFiPAF connect failed: %" CHIP_ERROR_FORMAT, err.Format());
    FailPendingConnect(sPendingConnect, chip::Protocols::InteractionModel::Status::Failure, /*cancelTimer=*/true);
}

static void OnScanDone(void * /*context*/, const std::vector<chip::DeviceLayer::NanPeerInfo> & peers)
{
    sScanInProgress = false;

    std::vector<ScanResultT> out;
    out.reserve(peers.size());

    for (const auto & p : peers)
    {
        ScanResultT r{};
        r.address.SetNonNull(chip::ByteSpan(p.mac, sizeof(p.mac)));
        r.transport = chip::BitMask<chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap>(
            chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap::kWiFiPAF);
        r.discriminator = p.discriminator;
        r.vendorID      = static_cast<chip::VendorId>(p.vid);
        r.productID     = p.pid;
        if (p.hasExtendedData && !p.storage.empty())
        {
            r.extendedData.SetNonNull(chip::ByteSpan(p.storage.data(), p.storage.size()));
        }
        else
        {
            r.extendedData.SetNull();
        }
        if (p.band != 0)
        {
            r.wiFiBand.SetValue(static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(p.band));
        }
        else
        {
            r.wiFiBand.ClearValue();
        }
        out.push_back(r);
    }

    // Hand the results to the cluster's scan aggregator; it owns the command
    // handle and emits the combined ProxyScanResponse once every transport's
    // sub-scan has reported.
    if (sHost != nullptr)
    {
        sHost->ScanAggregator().Contribute(chip::Span<const ScanResultT>(out.data(), out.size()));
    }
}

} // namespace

// ==================================================================
// Internal entry points (invoked by the CommissioningProxyPafTransport methods).
// ==================================================================

void SetHost(CommissioningProxyCluster * host)
{
    sHost = host;
}

chip::Protocols::InteractionModel::Status Connect(chip::app::CommandHandler * commandObj,
                                                  const chip::app::DataModel::InvokeRequest & request, uint16_t discriminator,
                                                  uint16_t timeout)
{
    // Only one PAF connect can be in flight at a time. Reject a second one up front,
    // before pausing the background scan or touching the PAF session pool, so a
    // refused connect leaves no side effects behind.
    if (sPendingConnect != nullptr)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: a PAF connect is already in progress");
        return chip::Protocols::InteractionModel::Status::Busy;
    }

    // Pause any background scan so the NAN subscribe slot can be reused for the PAF
    // connect subscribe; the registry resumes it on OnAllSessionsClosed().
    sBgScan.Pause();

    // The WiFiPAF layer is initialized once by the platform ConnectivityManager at
    // startup (ConnectivityManagerImpl::_Init -> WiFiPAFLayer::Init).  Do NOT call
    // Init() here: it memset()s the shared endpoint pool, which would wipe a live
    // PAFTP session (e.g. one mid-commissioning) while leaving its scheduled ack/
    // retransmit timers pointing at the zeroed slot.

    // Install the proxy receive delegate so messages arriving from the commissionee
    // over WiFiPAF are forwarded to the commissioner as ProxyMessageResponse rather
    // than being injected into the proxy app's own Matter transport stack.
    sProxyWiFiPAFDelegate.Install();

    // Register a PAF session so that OnDiscoveryResult (triggered from
    // WiFiPAFSubscribe) can locate the right entry by discriminator.
    chip::WiFiPAF::WiFiPAFSession pafSessionInfo{};
    pafSessionInfo.role          = chip::WiFiPAF::kWiFiPafRole_Subscriber;
    pafSessionInfo.nodeId        = static_cast<chip::NodeId>(discriminator);
    pafSessionInfo.discriminator = discriminator;
    CHIP_ERROR addErr =
        chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().AddPafSession(chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, pafSessionInfo);
    if (addErr != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: AddPafSession failed: %" CHIP_ERROR_FORMAT, addErr.Format());
        OnAllSessionsClosed();
        return (addErr == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED) ? chip::Protocols::InteractionModel::Status::ResourceExhausted
                                                              : chip::Protocols::InteractionModel::Status::Failure;
    }

    // Per spec a Timeout of 0 indicates no timeout: the connect runs until it
    // succeeds, fails, or is cancelled via ProxyDisconnectRequest(null).
    const bool hasTimeout = (timeout > 0);

    auto * ctx         = new ConnectCtx{};
    ctx->handle        = chip::app::CommandHandler::Handle(commandObj);
    ctx->path          = request.path;
    ctx->discriminator = discriminator;
    ctx->subscribeId   = 0;
    ctx->cluster       = sHost;
    ctx->fabricIndex   = request.subjectDescriptor.fabricIndex;
    sPendingConnect    = ctx;
    commandObj->FlushAcksRightAwayOnSlowCommand();

    if (auto * exchange = commandObj->GetExchangeContext())
    {
        // Keep the exchange open until just past the connect timeout, or disable
        // the response timer entirely (kZero) when there is no timeout. Clamp the
        // +5 s margin so a near-max timeout cannot wrap the uint16 seconds field.
        const uint16_t responseSecs = (timeout > static_cast<uint16_t>(0xFFFF - 5)) ? 0xFFFF : static_cast<uint16_t>(timeout + 5);
        exchange->SetResponseTimeout(hasTimeout ? chip::System::Clock::Seconds16(responseSecs) : chip::System::Clock::kZero);
    }

    CHIP_ERROR err = chip::DeviceLayer::ConnectivityMgr().WiFiPAFSubscribe(discriminator, ctx, OnConnectSuccess, OnConnectError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: WiFiPAFSubscribe failed: %" CHIP_ERROR_FORMAT, err.Format());
        CHIP_ERROR rmErr =
            chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, pafSessionInfo);
        if (rmErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "ProxyConnectRequest cleanup: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr.Format());
        }
        sPendingConnect = nullptr;
        delete ctx;
        OnAllSessionsClosed();
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    ctx->subscribeId = chip::DeviceLayer::ConnectivityMgrImpl().GetPendingConnectSubscribeId();

    if (hasTimeout)
    {
        CHIP_ERROR timerErr =
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(timeout), OnConnectTimeout, nullptr);
        if (timerErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyConnectRequest: StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            CHIP_ERROR rmErr2 = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(
                chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, pafSessionInfo);
            if (rmErr2 != CHIP_NO_ERROR)
            {
                ChipLogDetail(AppServer, "ProxyConnectRequest cleanup: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr2.Format());
            }
            if (ctx->subscribeId != 0)
            {
                (void) chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(ctx->subscribeId);
            }
            sPendingConnect = nullptr;
            delete ctx;
            OnAllSessionsClosed();
            return chip::Protocols::InteractionModel::Status::Failure;
        }
    }

    ChipLogProgress(AppServer, "ProxyConnectRequest: WiFiPAFSubscribe started for discriminator %u (subscribe_id %u)",
                    discriminator, ctx->subscribeId);

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status CancelPendingConnect(chip::FabricIndex fabricIndex)
{
    if (sPendingConnect == nullptr)
    {
        return chip::Protocols::InteractionModel::Status::InvalidInState;
    }

    if (fabricIndex != sPendingConnect->fabricIndex)
    {
        ChipLogProgress(AppServer, "CancelPendingConnect: pending PAF connect owned by fabric %u, rejected fabric %u",
                        sPendingConnect->fabricIndex, fabricIndex);
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    ChipLogProgress(AppServer, "CancelPendingConnect: cancelling pending PAF connect for fabric %u", fabricIndex);
    FailPendingConnect(sPendingConnect, chip::Protocols::InteractionModel::Status::Failure, /*cancelTimer=*/true);

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status Disconnect(uint16_t sessionId)
{
    auto it = sSessions.find(sessionId);
    if (it == sSessions.end())
    {
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    chip::WiFiPAF::WiFiPAFSession pafSession = it->second;
    sSessions.erase(it);

    CHIP_ERROR rmErr =
        chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(chip::WiFiPAF::PafInfoAccess::kAccSessionId, pafSession);
    if (rmErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "ProxyDisconnectRequest: RmPafSession for session %u: %" CHIP_ERROR_FORMAT, sessionId,
                      rmErr.Format());
    }

    // Explicitly close the PAFTP endpoint so its ack-received timer does not
    // fire 30 s later.
    chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().CloseEndPoint(pafSession);

    // Terminate the NAN subscribe instance that backed this session.  For a
    // subscriber session the WiFiPAFSession id IS the wpa_supplicant subscribe_id
    // (set in _WiFiPAFSubscribe), so cancelling it tears down the PAFTP session.
    // Without this the subscribe is left registered in the (long-lived)
    // wpa_supplicant and keeps firing discovery callbacks for the rest of the
    // proxy's lifetime.
    if (pafSession.id != 0)
    {
        CHIP_ERROR cancelErr = chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(pafSession.id);
        if (cancelErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "ProxyDisconnectRequest: WiFiPAFCancelSubscribe(%u): %" CHIP_ERROR_FORMAT, pafSession.id,
                          cancelErr.Format());
        }
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

CHIP_ERROR SendMessage(uint16_t sessionId, chip::System::PacketBufferHandle && buf)
{
    auto it = sSessions.find(sessionId);
    if (it == sSessions.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    // MUST use WiFiPAFLayer::SendMessage (not ConnectivityMgrImpl().WiFiPAFSend):
    // WiFiPAFSend is the low-level raw NAN transmit used internally by PAFTP;
    // calling it directly bypasses PAFTP framing and the end device cannot
    // parse the frame.  SendMessage goes through the PAFTP endpoint which
    // fragments and frames the message before calling WiFiPAFSend.
    return chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().SendMessage(it->second, std::move(buf));
}

chip::Protocols::InteractionModel::Status Scan(uint8_t scanMaxTime)
{
    if (sScanInProgress)
    {
        ChipLogProgress(AppServer, "ProxyScanRequest: PAF scan already in progress — returning Busy");
        return chip::Protocols::InteractionModel::Status::Busy;
    }

    // The WiFiPAF layer is initialized once by the platform ConnectivityManager at
    // startup; do NOT re-Init() here — it memset()s the shared endpoint pool and
    // would corrupt a live PAFTP session (see ProxyConnectRequest above).
    sScanInProgress = true;

    CHIP_ERROR err = chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFScan(scanMaxTime, &OnScanDone, nullptr);
    if (err != CHIP_NO_ERROR)
    {
        sScanInProgress = false;
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status BgScanStart(uint16_t timeout, chip::BitMask<WiFiBandBitmap> wiFiBandsMask,
                                                      chip::FabricIndex fabricIndex, chip::NodeId nodeId)
{
    // This driver services the kWiFiPAF transport.  The registry starts or defers
    // the hardware scan (StartHardwareScan reports BUSY while a ProxyConnect holds
    // the single NAN subscribe slot) and records the requested bands per fabric.
    return sBgScan.Start(fabricIndex, nodeId, chip::BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kWiFiPAF), wiFiBandsMask,
                         timeout);
}

chip::Protocols::InteractionModel::Status BgScanStop(chip::BitMask<CapabilitiesBitmap> transport,
                                                     chip::BitMask<WiFiBandBitmap> wiFiBands, chip::FabricIndex fabricIndex,
                                                     chip::NodeId nodeId)
{
    // The registry applies the spec transport/band overlap semantics (NOT_FOUND for
    // an unknown fabric, SUCCESS for a non-overlapping request, keep-scanning while
    // any fabric still covers the transport, stop+clear on the last fabric).
    return sBgScan.Stop(fabricIndex, nodeId, transport, wiFiBands);
}

// Resume a paused background scan on the next event-loop iteration.  Deferred via
// ScheduleWork rather than called inline because OnAllSessionsClosed can run from
// inside a PAFTP endpoint-close callstack (WiFiPAFCloseSession), and
// WiFiPAFStartBackgroundScan makes a blocking D-Bus nansubscribe call: running it
// inline would re-enter the WiFiPAF layer mid-teardown and stall the event loop.
static void ResumeBgScanWork(intptr_t /*arg*/)
{
    // The registry re-checks whether a connect is now pending (its StartHardwareScan
    // returns BUSY in that case) and stays paused if so.
    sBgScan.ResumeIfNeeded();
}

void OnAllSessionsClosed()
{
    if (sBgScan.IsPaused() && !sBgScan.IsEmpty())
    {
        LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().ScheduleWork(ResumeBgScanWork, 0));
    }
}

bool IsConnectPending()
{
    return sPendingConnect != nullptr;
}

void Shutdown()
{
    // Close any active PAF sessions: fail in-flight messages, close PAFTP
    // endpoints, cancel NAN subscribes, and remove from cluster session manager.
    for (auto & [sid, pafSession] : sSessions)
    {
        if (sHost != nullptr)
        {
            sHost->Sessions().DispatchMessageFailure(sid, chip::Protocols::InteractionModel::Status::Failure);
            sHost->Sessions().RemoveSession(sid);
        }
        (void) chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(chip::WiFiPAF::PafInfoAccess::kAccSessionId, pafSession);
        chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().CloseEndPoint(pafSession);
        if (pafSession.id != 0)
        {
            (void) chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(pafSession.id);
        }
    }
    sSessions.clear();

    // Tear down an in-flight ProxyConnect: cancel its timeout timer, cancel the
    // NAN subscribe, drop the PAF session and free the context so neither the
    // timer nor the heap context outlive the cluster.
    if (sPendingConnect != nullptr)
    {
        auto * ctx      = sPendingConnect;
        sPendingConnect = nullptr;

        chip::DeviceLayer::SystemLayer().CancelTimer(OnConnectTimeout, nullptr);
        (void) chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelIncompleteSubscribe();
        if (ctx->subscribeId != 0)
        {
            (void) chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(ctx->subscribeId);
        }

        chip::WiFiPAF::WiFiPAFSession keyInfo{};
        keyInfo.nodeId        = static_cast<chip::NodeId>(ctx->discriminator);
        keyInfo.discriminator = ctx->discriminator;
        (void) chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, keyInfo);
        delete ctx;
    }

    // Tear down the background scan: cancel every per-fabric lifetime timer and stop
    // the hardware scan if the registry currently owns it.
    sBgScan.Shutdown();

    sProxyWiFiPAFDelegate.Uninstall();
    sHost = nullptr;
}

} // namespace Paf

// ==================================================================
// CommissioningProxyTransport implementation (thin adapter over the Paf:: singleton).
// ==================================================================

void CommissioningProxyPafTransport::SetHost(CommissioningProxyCluster * host)
{
    Paf::SetHost(host);
}

Protocols::InteractionModel::Status CommissioningProxyPafTransport::Connect(app::CommandHandler * commandObj,
                                                                            const DataModel::InvokeRequest & request,
                                                                            uint16_t discriminator, uint16_t timeout)
{
    return Paf::Connect(commandObj, request, discriminator, timeout);
}

Protocols::InteractionModel::Status CommissioningProxyPafTransport::CancelPendingConnect(FabricIndex fabricIndex)
{
    return Paf::CancelPendingConnect(fabricIndex);
}

Protocols::InteractionModel::Status CommissioningProxyPafTransport::Disconnect(uint16_t sessionId)
{
    return Paf::Disconnect(sessionId);
}

CHIP_ERROR CommissioningProxyPafTransport::SendMessage(uint16_t sessionId, System::PacketBufferHandle && buf)
{
    return Paf::SendMessage(sessionId, std::move(buf));
}

Protocols::InteractionModel::Status CommissioningProxyPafTransport::Scan(uint8_t scanMaxTime)
{
    return Paf::Scan(scanMaxTime);
}

Protocols::InteractionModel::Status CommissioningProxyPafTransport::BgScanStart(uint16_t timeout, BitMask<WiFiBandBitmap> wiFiBands,
                                                                                FabricIndex fabricIndex, NodeId nodeId)
{
    return Paf::BgScanStart(timeout, wiFiBands, fabricIndex, nodeId);
}

Protocols::InteractionModel::Status CommissioningProxyPafTransport::BgScanStop(BitMask<CapabilitiesBitmap> transport,
                                                                               BitMask<WiFiBandBitmap> wiFiBands,
                                                                               FabricIndex fabricIndex, NodeId nodeId)
{
    return Paf::BgScanStop(transport, wiFiBands, fabricIndex, nodeId);
}

void CommissioningProxyPafTransport::OnAllSessionsClosed()
{
    Paf::OnAllSessionsClosed();
}

bool CommissioningProxyPafTransport::IsConnectPending() const
{
    return Paf::IsConnectPending();
}

void CommissioningProxyPafTransport::Shutdown()
{
    Paf::Shutdown();
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
