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
static std::map<uint16_t, chip::WiFiPAF::WiFiPAFSession> sPafSessions;

// Context for the async WiFiPAF connection initiated by ProxyConnectRequest.
struct PafConnectCtx
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
static PafConnectCtx * sPafPendingConnect = nullptr;

// Host cluster (set via the transport's SetHost). All transport-agnostic
// bookkeeping (sessions, message routing, scan cache/aggregation) is reached
// through its subsystem accessors.
static chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * sHost = nullptr;

// True while a ProxyScanRequest is in progress; used to return Busy for concurrent requests.
static bool sScanInProgress = false;

// ------------------------------------------------------------------
// Background scan state — PAF only for now.
// ------------------------------------------------------------------

struct FabricKey
{
    chip::FabricIndex fabricIndex;
    chip::NodeId nodeId;
    bool operator<(const FabricKey & o) const
    {
        if (fabricIndex != o.fabricIndex)
            return fabricIndex < o.fabricIndex;
        return nodeId < o.nodeId;
    }
};

struct FabricLifetimeCtx
{
    FabricKey key;
};

struct FabricScanRecord
{
    chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport;
    chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands;
    FabricLifetimeCtx * lifetimeCtx = nullptr;
};

static std::map<FabricKey, FabricScanRecord> sBgScanFabrics;
static bool sBgScanPaused                                                                  = false;
static chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * sBgScanCluster = nullptr;

static ScanResultT NanPeerToScanResult(const chip::DeviceLayer::NanPeerInfo & p)
{
    ScanResultT r{};
    r.address.SetNonNull(chip::ByteSpan(p.mac, sizeof(p.mac)));
    r.transport = chip::BitMask<chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap>(
        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap::kWiFiPAF);
    r.discriminator = p.discriminator;
    r.vendorID      = static_cast<chip::VendorId>(p.vid);
    r.productID     = p.pid;
    if (p.hasExtendedData && !p.storage.empty())
        r.extendedData.SetNonNull(chip::ByteSpan(p.storage.data(), p.storage.size()));
    else
        r.extendedData.SetNull();
    if (p.band != 0)
        r.wiFiBand.SetValue(static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(p.band));
    else
        r.wiFiBand.ClearValue();
    return r;
}

static void OnBgScanDiscovery(void * /*ctx*/, const chip::DeviceLayer::NanPeerInfo & peer)
{
    // Runs on the Matter event loop (wpa_supplicant NAN callbacks are dispatched
    // there), so the shared cache may be touched directly.
    if (sBgScanCluster != nullptr)
        sBgScanCluster->ScanCache().Report(NanPeerToScanResult(peer));
}

static void OnBgScanLifetimeExpiry(chip::System::Layer * /*layer*/, void * appState)
{
    auto * ctx    = static_cast<FabricLifetimeCtx *>(appState);
    FabricKey key = ctx->key;
    delete ctx;

    auto it = sBgScanFabrics.find(key);
    if (it == sBgScanFabrics.end())
        return;
    it->second.lifetimeCtx = nullptr;
    sBgScanFabrics.erase(it);

    ChipLogProgress(AppServer, "BgScan: lifetime expired for fabricIndex=%u nodeId=0x" ChipLogFormatX64, key.fabricIndex,
                    ChipLogValueX64(key.nodeId));

    if (sBgScanFabrics.empty())
    {
        chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStopBackgroundScan();
        sBgScanPaused = false;
        if (sBgScanCluster != nullptr)
            sBgScanCluster->ScanCache().ClearTransport(chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap::kWiFiPAF);
        sBgScanCluster = nullptr;
    }
}

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
            return;
        mOriginalTransport      = layer.mWiFiPAFTransport;
        layer.mWiFiPAFTransport = this;
        ChipLogProgress(AppServer, "ProxyWiFiPAFDelegate: installed (original=%p)", (void *) mOriginalTransport);
    }

    CHIP_ERROR WiFiPAFMessageReceived(chip::WiFiPAF::WiFiPAFSession & RxInfo, chip::System::PacketBufferHandle && msg) override
    {
        for (auto & [sid, sess] : sPafSessions)
        {
            if (sess.peer_id == RxInfo.peer_id)
            {
                if (sHost != nullptr)
                    sHost->Sessions().DispatchMessageResponse(sid, msg->Start(), msg->DataLength());
                return CHIP_NO_ERROR;
            }
        }
        if (mOriginalTransport != nullptr)
            return mOriginalTransport->WiFiPAFMessageReceived(RxInfo, std::move(msg));
        ChipLogError(AppServer, "ProxyWiFiPAFDelegate: no original transport for non-proxy session");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR WiFiPAFMessageSend(chip::WiFiPAF::WiFiPAFSession & TxInfo, chip::System::PacketBufferHandle && msg) override
    {
        if (mOriginalTransport != nullptr)
            return mOriginalTransport->WiFiPAFMessageSend(TxInfo, std::move(msg));
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
        for (auto & [id, sess] : sPafSessions)
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
                sHost->Sessions().DispatchMessageFailure(sid, chip::Protocols::InteractionModel::Status::Failure);
            sPafSessions.erase(sid);
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
                sHost->Sessions().RemoveSession(sid);
            // This path does not run through the dispatcher's OnAllSessionsClosed(),
            // so resume a bg-scan paused for the connect once the last session is
            // gone and no connect is mid-flight.
            if (sPafSessions.empty() && sPafPendingConnect == nullptr)
            {
                OnAllSessionsClosed();
            }
        }
        if (mOriginalTransport != nullptr)
            return mOriginalTransport->WiFiPAFCloseSession(SessionInfo);
        return CHIP_NO_ERROR;
    }

    bool WiFiPAFResourceAvailable() override
    {
        if (mOriginalTransport != nullptr)
            return mOriginalTransport->WiFiPAFResourceAvailable();
        return true;
    }

private:
    chip::WiFiPAF::WiFiPAFLayerDelegate * mOriginalTransport = nullptr;
};

static ProxyWiFiPAFDelegate sProxyWiFiPAFDelegate;

// ------------------------------------------------------------------
// PAF connect callbacks (success / error / timeout)
// ------------------------------------------------------------------

static void OnProxyConnectTimeout(chip::System::Layer * layer, void * appState);

// Common teardown for an in-flight PAF connect that did not succeed (timeout,
// transport error, or commissioner-initiated cancel).  Cancels the timeout timer
// and NAN subscribe, closes any PAFTP endpoint the handshake created, drops the
// session, resolves the originating ProxyConnectRequest with `status`, frees the
// context, and resumes a background scan that was deferred/paused for the connect.
// `cancelTimer` is false only when called from the timeout handler itself (the
// timer has already fired).
static void FailPendingPafConnect(PafConnectCtx * ctx, chip::Protocols::InteractionModel::Status status, bool cancelTimer)
{
    sPafPendingConnect = nullptr;

    if (cancelTimer)
        chip::DeviceLayer::SystemLayer().CancelTimer(OnProxyConnectTimeout, nullptr);

    // Null the platform connect callbacks before cancelling the subscribe so the
    // kCHIPoWiFiPAFCancelConnect event posted by OnNanSubscribeTerminated does not
    // re-enter OnPafConnectError.
    CHIP_ERROR cancelIncompleteErr = chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelIncompleteSubscribe();
    if (cancelIncompleteErr != CHIP_NO_ERROR)
        ChipLogDetail(AppServer, "FailPendingPafConnect: WiFiPAFCancelIncompleteSubscribe: %" CHIP_ERROR_FORMAT,
                      cancelIncompleteErr.Format());

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
        ChipLogDetail(AppServer, "FailPendingPafConnect: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr.Format());
    if (haveEndpoint)
        pafLayer.CloseEndPoint(endpointSession);

    if (ctx->subscribeId != 0)
    {
        CHIP_ERROR cancelErr = chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(ctx->subscribeId);
        if (cancelErr != CHIP_NO_ERROR)
            ChipLogDetail(AppServer, "FailPendingPafConnect: WiFiPAFCancelSubscribe(%u): %" CHIP_ERROR_FORMAT, ctx->subscribeId,
                          cancelErr.Format());
    }

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
        cmd->AddStatus(ctx->path, status);
    delete ctx;

    // The connect freed the NAN subscribe slot; resume a background scan that was
    // deferred or paused for this connect (a failed connect registers no session,
    // so the dispatcher's OnAllSessionsClosed path does not run).
    OnAllSessionsClosed();
}

static void OnProxyConnectTimeout(chip::System::Layer * /*layer*/, void * /*appState*/)
{
    if (sPafPendingConnect == nullptr)
        return; // Success or error callback already fired first; nothing to do.
    ChipLogError(AppServer, "ProxyConnectRequest: timeout waiting for WiFiPAF connect (disc %u)",
                 sPafPendingConnect->discriminator);
    FailPendingPafConnect(sPafPendingConnect, chip::Protocols::InteractionModel::Status::Timeout, /*cancelTimer=*/false);
}

static void OnPafConnectSuccess(void * /*context*/)
{
    auto * ctx = sPafPendingConnect;
    if (ctx == nullptr)
    {
        ChipLogProgress(AppServer, "OnPafConnectSuccess: no pending connect ctx; ignoring stale callback");
        return;
    }
    sPafPendingConnect = nullptr;
    chip::DeviceLayer::SystemLayer().CancelTimer(OnProxyConnectTimeout, nullptr);

    chip::app::CommandHandler * cmd = ctx->handle.Get();

    chip::WiFiPAF::WiFiPAFSession keyInfo{};
    keyInfo.discriminator                    = ctx->discriminator;
    chip::WiFiPAF::WiFiPAFLayer & layer      = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    chip::WiFiPAF::WiFiPAFSession * pPafInfo = layer.GetPAFInfo(chip::WiFiPAF::PafInfoAccess::kAccDisc, keyInfo);

    if (cmd == nullptr || pPafInfo == nullptr)
    {
        ChipLogError(AppServer, "OnPafConnectSuccess: cmd=%p pPafInfo=%p for disc %u", (void *) cmd, (void *) pPafInfo,
                     ctx->discriminator);
        if (cmd != nullptr)
            cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
        delete ctx;
        return;
    }

    uint16_t sessionId      = ctx->cluster->Sessions().AllocSessionId();
    sPafSessions[sessionId] = *pPafInfo;
    ctx->cluster->Sessions().RegisterSession(sessionId, CapabilitiesBitmap::kWiFiPAF, ctx->fabricIndex);

    ChipLogProgress(AppServer, "ProxyConnectRequest: WiFiPAF connected, proxy session %u (disc %u peer_id %u)", sessionId,
                    ctx->discriminator, pPafInfo->peer_id);

    if (ctx->cluster)
    {
        CHIP_ERROR stateErr =
            ctx->cluster->SetCPState(chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster::kState_CPConnected);
        if (stateErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "OnPafConnectSuccess: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
        }
    }

    chip::app::Clusters::CommissioningProxy::Commands::ProxyConnectResponse::Type response;
    response.sessionID = sessionId;
    cmd->AddResponse(ctx->path, response);
    delete ctx;
}

static void OnPafConnectError(void * /*context*/, CHIP_ERROR err)
{
    auto * ctx = sPafPendingConnect;
    if (ctx == nullptr)
    {
        ChipLogProgress(AppServer,
                        "OnPafConnectError: ignoring stale callback after successful connect "
                        "(err: %" CHIP_ERROR_FORMAT ")",
                        err.Format());
        return;
    }
    ChipLogError(AppServer, "ProxyConnectRequest: WiFiPAF connect failed: %" CHIP_ERROR_FORMAT, err.Format());
    FailPendingPafConnect(sPafPendingConnect, chip::Protocols::InteractionModel::Status::Failure, /*cancelTimer=*/true);
}

static void OnPafScanDone(void * /*context*/, const std::vector<chip::DeviceLayer::NanPeerInfo> & peers)
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
            r.extendedData.SetNonNull(chip::ByteSpan(p.storage.data(), p.storage.size()));
        else
            r.extendedData.SetNull();
        if (p.band != 0)
            r.wiFiBand.SetValue(static_cast<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap>(p.band));
        else
            r.wiFiBand.ClearValue();
        out.push_back(r);
    }

    // Hand the results to the cluster's scan aggregator; it owns the command
    // handle and emits the combined ProxyScanResponse once every transport's
    // sub-scan has reported.
    if (sHost != nullptr)
        sHost->ScanAggregator().Contribute(chip::Span<const ScanResultT>(out.data(), out.size()));
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
    if (sPafPendingConnect != nullptr)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: a PAF connect is already in progress");
        return chip::Protocols::InteractionModel::Status::Busy;
    }

    // If background scanning is active, pause it so the NAN subscribe slot can be
    // reused for the PAF connect subscribe; it resumes on OnAllSessionsClosed().
    if (!sBgScanFabrics.empty() && !sBgScanPaused)
    {
        ChipLogProgress(AppServer, "ProxyConnectRequest: pausing background scan");
        chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStopBackgroundScan();
        sBgScanPaused = true;
    }

    // Init PAF layer (idempotent).
    CHIP_ERROR initErr = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().Init(&chip::DeviceLayer::SystemLayer());
    if (initErr != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: WiFiPAFLayer Init failed: %" CHIP_ERROR_FORMAT, initErr.Format());
        return chip::Protocols::InteractionModel::Status::Failure;
    }

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
        return (addErr == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED) ? chip::Protocols::InteractionModel::Status::ResourceExhausted
                                                              : chip::Protocols::InteractionModel::Status::Failure;
    }

    // Per spec a Timeout of 0 indicates no timeout: the connect runs until it
    // succeeds, fails, or is cancelled via ProxyDisconnectRequest(null).
    const bool hasTimeout = (timeout > 0);

    auto * ctx         = new PafConnectCtx{};
    ctx->handle        = chip::app::CommandHandler::Handle(commandObj);
    ctx->path          = request.path;
    ctx->discriminator = discriminator;
    ctx->subscribeId   = 0;
    ctx->cluster       = sHost;
    ctx->fabricIndex   = request.subjectDescriptor.fabricIndex;
    sPafPendingConnect = ctx;
    commandObj->FlushAcksRightAwayOnSlowCommand();

    if (auto * exchange = commandObj->GetExchangeContext())
    {
        // Keep the exchange open until just past the connect timeout, or disable
        // the response timer entirely (kZero) when there is no timeout. Clamp the
        // +5 s margin so a near-max timeout cannot wrap the uint16 seconds field.
        const uint16_t responseSecs = (timeout > static_cast<uint16_t>(0xFFFF - 5)) ? 0xFFFF : static_cast<uint16_t>(timeout + 5);
        exchange->SetResponseTimeout(hasTimeout ? chip::System::Clock::Seconds16(responseSecs) : chip::System::Clock::kZero);
    }

    CHIP_ERROR err =
        chip::DeviceLayer::ConnectivityMgr().WiFiPAFSubscribe(discriminator, ctx, OnPafConnectSuccess, OnPafConnectError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: WiFiPAFSubscribe failed: %" CHIP_ERROR_FORMAT, err.Format());
        CHIP_ERROR rmErr =
            chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, pafSessionInfo);
        if (rmErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "ProxyConnectRequest cleanup: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr.Format());
        }
        sPafPendingConnect = nullptr;
        delete ctx;
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    ctx->subscribeId = chip::DeviceLayer::ConnectivityMgrImpl().GetPendingConnectSubscribeId();

    if (hasTimeout)
    {
        CHIP_ERROR timerErr =
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(timeout), OnProxyConnectTimeout, nullptr);
        if (timerErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyConnectRequest: StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            CHIP_ERROR rmErr2 = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(
                chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, pafSessionInfo);
            if (rmErr2 != CHIP_NO_ERROR)
            {
                ChipLogDetail(AppServer, "ProxyConnectRequest cleanup: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr2.Format());
            }
            sPafPendingConnect = nullptr;
            delete ctx;
            return chip::Protocols::InteractionModel::Status::Failure;
        }
    }

    ChipLogProgress(AppServer, "ProxyConnectRequest: WiFiPAFSubscribe started for discriminator %u (subscribe_id %u)",
                    discriminator, ctx->subscribeId);

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status CancelPendingConnect(chip::FabricIndex fabricIndex)
{
    if (sPafPendingConnect == nullptr)
        return chip::Protocols::InteractionModel::Status::InvalidInState;

    if (fabricIndex != sPafPendingConnect->fabricIndex)
    {
        ChipLogProgress(AppServer, "CancelPendingConnect: pending PAF connect owned by fabric %u, rejected fabric %u",
                        sPafPendingConnect->fabricIndex, fabricIndex);
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    ChipLogProgress(AppServer, "CancelPendingConnect: cancelling pending PAF connect for fabric %u", fabricIndex);
    FailPendingPafConnect(sPafPendingConnect, chip::Protocols::InteractionModel::Status::Failure, /*cancelTimer=*/true);

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status Disconnect(uint16_t sessionId)
{
    auto it = sPafSessions.find(sessionId);
    if (it == sPafSessions.end())
    {
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    chip::WiFiPAF::WiFiPAFSession pafSession = it->second;
    sPafSessions.erase(it);

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
    // (set in _WiFiPAFSubscribe), so cancelling it closes the PAFTP session per
    // PAFTP spec section 4.20.3.10.  Without this the subscribe is left
    // registered in the (long-lived) wpa_supplicant and keeps firing discovery
    // callbacks for the rest of the proxy's lifetime.
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
    auto it = sPafSessions.find(sessionId);
    if (it == sPafSessions.end())
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

    CHIP_ERROR err = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().Init(&chip::DeviceLayer::SystemLayer());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Commissioning Proxy PAF Scan Request Failed");
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    sScanInProgress = true;

    err = chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFScan(scanMaxTime, &OnPafScanDone, nullptr);
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
    // This driver services the kWiFiPAF transport; the requested bands are recorded
    // per-fabric so a later stop can match on them.
    const CapabilitiesBitmap transport = CapabilitiesBitmap::kWiFiPAF;
    const WiFiBandBitmap wiFiBands     = static_cast<WiFiBandBitmap>(wiFiBandsMask.Raw());
    FabricKey fabricKey{ fabricIndex, nodeId };
    bool wasEmpty      = sBgScanFabrics.empty();
    bool alreadyExists = (sBgScanFabrics.count(fabricKey) > 0);

    if (alreadyExists)
    {
        auto & rec = sBgScanFabrics[fabricKey];
        if (rec.lifetimeCtx != nullptr)
        {
            chip::DeviceLayer::SystemLayer().CancelTimer(OnBgScanLifetimeExpiry, rec.lifetimeCtx);
            delete rec.lifetimeCtx;
            rec.lifetimeCtx = nullptr;
        }
        ChipLogProgress(AppServer, "ProxyBackgroundScanStartRequest: updating timeout for existing fabric");
    }

    sBgScanFabrics[fabricKey] = FabricScanRecord{ transport, wiFiBands, nullptr };
    if (sBgScanCluster == nullptr)
        sBgScanCluster = sHost;

    if (sPafPendingConnect != nullptr)
    {
        // A ProxyConnect is in flight and has taken the single NAN subscribe slot
        // (Connect pauses any background scan for exactly this reason).  Register
        // this fabric but defer starting the hardware scan, and mark it paused so
        // OnAllSessionsClosed() starts it once the connect resolves — starting it
        // now would re-create the subscribe-slot contention the pause avoids.
        sBgScanPaused = true;
        ChipLogProgress(AppServer, "ProxyBackgroundScanStartRequest: connect in flight, deferring hardware scan start");
    }
    else if (wasEmpty || sBgScanPaused)
    {
        CHIP_ERROR err = chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStartBackgroundScan(OnBgScanDiscovery, nullptr);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyBackgroundScanStartRequest: WiFiPAFStartBackgroundScan failed: %" CHIP_ERROR_FORMAT,
                         err.Format());
            sBgScanFabrics.erase(fabricKey);
            if (sBgScanFabrics.empty())
            {
                sBgScanCluster = nullptr;
            }
            return chip::Protocols::InteractionModel::Status::Failure;
        }
        sBgScanPaused = false;
        ChipLogProgress(AppServer, "ProxyBackgroundScanStartRequest: hardware scan started");
    }
    else
    {
        ChipLogProgress(AppServer, "ProxyBackgroundScanStartRequest: hardware scan already running, registering fabric (%zu total)",
                        sBgScanFabrics.size());
    }

    if (timeout > 0)
    {
        auto * lCtx = new FabricLifetimeCtx{ fabricKey };
        CHIP_ERROR timerErr =
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(timeout), OnBgScanLifetimeExpiry, lCtx);
        if (timerErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyBackgroundScanStartRequest: StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            delete lCtx;
        }
        else
        {
            sBgScanFabrics[fabricKey].lifetimeCtx = lCtx;
        }
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status BgScanStop(chip::BitMask<CapabilitiesBitmap> transport,
                                                     chip::BitMask<WiFiBandBitmap> wiFiBands, chip::FabricIndex fabricIndex,
                                                     chip::NodeId nodeId)
{
    FabricKey fabricKey{ fabricIndex, nodeId };
    auto it = sBgScanFabrics.find(fabricKey);

    if (it == sBgScanFabrics.end())
    {
        ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: no matching fabric record");
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    uint8_t reqTransportBits = transport.Raw();
    uint16_t reqBandBits     = wiFiBands.Raw();
    uint8_t fabTransportBits = static_cast<uint8_t>(it->second.transport);
    uint16_t fabBandBits     = static_cast<uint16_t>(it->second.wiFiBands);

    uint8_t stopTransportBits;
    uint16_t stopBandBits;
    if (reqTransportBits == 0)
    {
        stopTransportBits = 0;
        stopBandBits      = reqBandBits & fabBandBits;
    }
    else
    {
        stopTransportBits = reqTransportBits & fabTransportBits;
        stopBandBits      = reqBandBits & fabBandBits;
    }

    if (stopTransportBits == 0 && stopBandBits == 0)
    {
        ChipLogProgress(AppServer,
                        "ProxyBackgroundScanStopRequest: no overlap with started transports/bands (started transport:0x%x "
                        "bands:0x%x), returning SUCCESS",
                        fabTransportBits, fabBandBits);
        return chip::Protocols::InteractionModel::Status::Success;
    }

    uint8_t remainTransport = fabTransportBits & ~stopTransportBits;
    uint16_t remainBands    = fabBandBits & ~stopBandBits;
    bool fabricNowEmpty     = (remainTransport == 0);

    if (fabricNowEmpty)
    {
        if (it->second.lifetimeCtx != nullptr)
        {
            chip::DeviceLayer::SystemLayer().CancelTimer(OnBgScanLifetimeExpiry, it->second.lifetimeCtx);
            delete it->second.lifetimeCtx;
            it->second.lifetimeCtx = nullptr;
        }
        sBgScanFabrics.erase(it);
        ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: fabric fully stopped");
    }
    else
    {
        it->second.transport = static_cast<CapabilitiesBitmap>(remainTransport);
        it->second.wiFiBands = static_cast<WiFiBandBitmap>(remainBands);
        ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: fabric partially stopped, remaining transport:0x%x bands:0x%x",
                        remainTransport, remainBands);
    }

    bool otherFabricCovers = false;
    for (const auto & [key, rec] : sBgScanFabrics)
    {
        uint8_t ot  = static_cast<uint8_t>(rec.transport);
        uint16_t ob = static_cast<uint16_t>(rec.wiFiBands);
        if ((ot & stopTransportBits) != 0 || (ob & stopBandBits) != 0)
        {
            otherFabricCovers = true;
            break;
        }
    }

    if (!otherFabricCovers)
    {
        if (sBgScanFabrics.empty())
        {
            chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStopBackgroundScan();
            if (sBgScanCluster != nullptr)
                sBgScanCluster->ScanCache().ClearTransport(chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap::kWiFiPAF);
            sBgScanCluster = nullptr;
            ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: last fabric stopped, hardware scan stopped");
        }
        else
        {
            ChipLogProgress(AppServer,
                            "ProxyBackgroundScanStopRequest: stopped bands not covered by other fabrics, "
                            "%zu fabric(s) scanning other bands",
                            sBgScanFabrics.size());
        }
    }
    else
    {
        ChipLogProgress(AppServer,
                        "ProxyBackgroundScanStopRequest: %zu other fabric(s) still cover stopped bands, "
                        "hardware scan continues",
                        sBgScanFabrics.size());
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

void OnAllSessionsClosed()
{
    if (sBgScanPaused && !sBgScanFabrics.empty())
    {
        ChipLogProgress(AppServer, "ProxyDisconnectRequest: resuming background scan");
        CHIP_ERROR resumeErr = chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStartBackgroundScan(OnBgScanDiscovery, nullptr);
        if (resumeErr == CHIP_NO_ERROR)
            sBgScanPaused = false;
        else
            ChipLogError(AppServer, "ProxyDisconnectRequest: resume background scan failed: %" CHIP_ERROR_FORMAT,
                         resumeErr.Format());
    }
}

bool IsConnectPending()
{
    return sPafPendingConnect != nullptr;
}

void Shutdown()
{
    // Tear down an in-flight ProxyConnect: cancel its timeout timer, cancel the
    // NAN subscribe, drop the PAF session and free the context so neither the
    // timer nor the heap context outlive the cluster.
    if (sPafPendingConnect != nullptr)
    {
        auto * ctx         = sPafPendingConnect;
        sPafPendingConnect = nullptr;

        chip::DeviceLayer::SystemLayer().CancelTimer(OnProxyConnectTimeout, nullptr);
        (void) chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelIncompleteSubscribe();
        if (ctx->subscribeId != 0)
            (void) chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(ctx->subscribeId);

        chip::WiFiPAF::WiFiPAFSession keyInfo{};
        keyInfo.nodeId        = static_cast<chip::NodeId>(ctx->discriminator);
        keyInfo.discriminator = ctx->discriminator;
        (void) chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, keyInfo);
        delete ctx;
    }

    // Cancel every outstanding per-fabric lifetime timer and free its context.
    for (auto & [key, rec] : sBgScanFabrics)
    {
        if (rec.lifetimeCtx != nullptr)
        {
            chip::DeviceLayer::SystemLayer().CancelTimer(OnBgScanLifetimeExpiry, rec.lifetimeCtx);
            delete rec.lifetimeCtx;
            rec.lifetimeCtx = nullptr;
        }
    }

    // Stop the hardware background scan only if it currently owns the radio.
    if (!sBgScanFabrics.empty() && !sBgScanPaused)
        (void) chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStopBackgroundScan();

    sBgScanFabrics.clear();
    sBgScanPaused  = false;
    sBgScanCluster = nullptr;
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
