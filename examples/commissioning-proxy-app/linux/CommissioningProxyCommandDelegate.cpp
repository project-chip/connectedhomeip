/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/general-diagnostics-server/CodegenIntegration.h>
#include <app/clusters/software-diagnostics-server/software-fault-listener.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <platform/PlatformManager.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyDelegate.h>
#include <clusters/CommissioningProxy/Commands.h>
#include "commissioning-proxy-delegate-impl.h"

#include <map>
#include <wifipaf/WiFiPAFLayer.h>
#include <wifipaf/WiFiPAFLayerDelegate.h>
#include <wifipaf/WiFiPAFRole.h>
#include <platform/ConnectivityManager.h>
#include <platform/Linux/ConnectivityManagerImpl.h>

using namespace chip;
using namespace chip::app;
using namespace Clusters::CommissioningProxy;

namespace {

using ScanResultT = chip::app::Clusters::CommissioningProxy::Structs::ScanResultStruct::Type;

// Default ProxyConnectRequest timeout when the commissioner sends 0 (spec §10.5.7.1).
constexpr uint16_t kProxyConnectDefaultTimeoutSecs = 30;
// ------------------------------------------------------------------
// Proxy session tracking
//
// Maps a proxy sessionId (assigned in ProxyConnectRequest) to the
// WiFiPAF session info needed to forward packets to the commissionee.
// ------------------------------------------------------------------

struct ProxySessionInfo
{
    chip::WiFiPAF::WiFiPAFSession pafSession; // includes peer_addr, discriminator, etc.
};

// Session map: sessionId → ProxySessionInfo
static std::map<uint16_t, ProxySessionInfo> sProxySessions;

// Session ID counter.  Starts at 1; 0 is never a valid session ID.
static uint16_t sNextProxySessionId = 1;

// Allocate the next free session ID, skipping 0 and any already-active session.
static uint16_t AllocSessionId()
{
    uint16_t id;
    do {
        if (sNextProxySessionId == 0)
            sNextProxySessionId = 1;
        id = sNextProxySessionId++;
    } while (sProxySessions.count(id) > 0);
    return id;
}

// Context that keeps a ProxyMessageRequest handler alive until the
// commissionee response arrives over WiFiPAF.
struct ProxyMsgCtx
{
    chip::app::CommandHandler::Handle handle;
    chip::app::ConcreteCommandPath path;
    uint16_t sessionId;
};

// Map sessionId → pending ProxyMessageRequest context (at most one per session).
static std::map<uint16_t, ProxyMsgCtx *> sPendingProxyMsgCtx;

// Context that keeps the IM command alive until scan completes.
struct ProxyScanCtx
{
    chip::app::CommandHandler::Handle handle;
    chip::app::ConcreteCommandPath path;
    uint8_t scanMaxTime = 0;
};

// Context for the async WiFiPAF connection initiated by ProxyConnectRequest.
struct ProxyConnectCtx
{
    chip::app::CommandHandler::Handle handle;
    chip::app::ConcreteCommandPath path;
    uint16_t discriminator;
    uint32_t subscribeId; // NAN subscribe_id from wpa_supplicant, stored after WiFiPAFSubscribe
    // Back-pointer so OnPafConnectSuccess can transition the cluster state.
    chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * cluster = nullptr;
};

// Owning pointer to the in-flight ProxyConnectRequest context.
// Nulled by whichever callback fires first (success or error) so the other
// callback can detect it has already been handled and avoid use-after-free.
static ProxyConnectCtx * sPendingConnectCtx = nullptr;

// ------------------------------------------------------------------
// WiFiPAF receive path: commissionee → proxy → commissioner
//
// When a fully-reassembled PAFTP message arrives from the commissionee,
// ProxyWiFiPAFDelegate intercepts it (instead of injecting it into the
// proxy app's own Matter transport stack) and calls
// OnProxyWiFiPAFMessageReceived to send it back as ProxyMessageResponse.
// ------------------------------------------------------------------

// Sends a received commissionee packet back to chip-tool as ProxyMessageResponse.
static void OnProxyWiFiPAFMessageReceived(uint16_t sessionId, const uint8_t * data, size_t length)
{
    auto it = sPendingProxyMsgCtx.find(sessionId);
    if (it == sPendingProxyMsgCtx.end())
    {
        ChipLogDetail(AppServer, "OnProxyWiFiPAFMessageReceived: no pending ctx for session %u — "
                                 "commissionee sent unsolicited data; dropping", sessionId);
        return;
    }

    ProxyMsgCtx * ctx = it->second;
    sPendingProxyMsgCtx.erase(it);

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd == nullptr)
    {
        // Exchange timed out before the commissionee replied.
        ChipLogDetail(AppServer, "OnProxyWiFiPAFMessageReceived: exchange for session %u already expired", sessionId);
        delete ctx;
        return;
    }

    chip::app::Clusters::CommissioningProxy::Commands::ProxyMessageResponse::Type response;
    response.sessionId = sessionId;
    response.message.SetNonNull(chip::ByteSpan(data, length));
    cmd->AddResponse(ctx->path, response);
    delete ctx;
}

// Sits between the WiFiPAFLayer and the original server transport.
// For proxy sessions: routes received messages to OnProxyWiFiPAFMessageReceived.
// For all other sessions: delegates to the original transport unchanged.
class ProxyWiFiPAFDelegate : public chip::WiFiPAF::WiFiPAFLayerDelegate
{
public:
    // Replace the WiFiPAFLayer's transport delegate with ourselves, saving
    // the previous delegate so non-proxy sessions continue to work.
    // Idempotent — safe to call on every ProxyConnectRequest.
    void Install()
    {
        auto & layer = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
        if (layer.mWiFiPAFTransport == this)
            return;
        mOriginalTransport      = layer.mWiFiPAFTransport;
        layer.mWiFiPAFTransport = this;
        ChipLogProgress(AppServer, "ProxyWiFiPAFDelegate: installed (original=%p)", (void *) mOriginalTransport);
    }

    CHIP_ERROR WiFiPAFMessageReceived(chip::WiFiPAF::WiFiPAFSession & RxInfo,
                                      chip::System::PacketBufferHandle && msg) override
    {
        // Match against active proxy sessions by peer_id.
        for (auto & [sid, info] : sProxySessions)
        {
            if (info.pafSession.peer_id == RxInfo.peer_id)
            {
                OnProxyWiFiPAFMessageReceived(sid, msg->Start(), msg->DataLength());
                return CHIP_NO_ERROR;
            }
        }
        // Not a proxy session — route to the original server transport.
        if (mOriginalTransport != nullptr)
            return mOriginalTransport->WiFiPAFMessageReceived(RxInfo, std::move(msg));
        ChipLogError(AppServer, "ProxyWiFiPAFDelegate: no original transport for non-proxy session");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR WiFiPAFMessageSend(chip::WiFiPAF::WiFiPAFSession & TxInfo,
                                  chip::System::PacketBufferHandle && msg) override
    {
        if (mOriginalTransport != nullptr)
            return mOriginalTransport->WiFiPAFMessageSend(TxInfo, std::move(msg));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR WiFiPAFCloseSession(chip::WiFiPAF::WiFiPAFSession & SessionInfo) override
    {
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

// Called when the ProxyConnectRequest Timeout field expires before the PAF
// handshake completes.  Per spec the connection attempt is terminated
// and Status::Timeout is returned.
static void OnProxyConnectTimeout(chip::System::Layer * layer, void * appState)
{
    auto * ctx = sPendingConnectCtx;
    if (ctx == nullptr)
    {
        // Success or error callback already fired first; nothing to do.
        return;
    }
    sPendingConnectCtx = nullptr;

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    ChipLogError(AppServer, "ProxyConnectRequest: timeout waiting for WiFiPAF connect (disc %u)", ctx->discriminator);

    // Use the subscribe_id stored at subscribe time
    uint32_t subscribeId = ctx->subscribeId;

    // Null out the platform callbacks FIRST so the kCHIPoWiFiPAFCancelConnect
    // event posted by OnNanSubscribeTerminated does not call OnPafConnectError.
    CHIP_ERROR cancelIncompleteErr = chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelIncompleteSubscribe();
    if (cancelIncompleteErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "OnProxyConnectTimeout: WiFiPAFCancelIncompleteSubscribe: %" CHIP_ERROR_FORMAT,
                      cancelIncompleteErr.Format());
    }

    // Cancel the actual NAN subscribe in wpa_supplicant so the hardware stops
    // scanning and nandiscovery-result signals stop arriving.
    if (subscribeId != 0)
    {
        CHIP_ERROR cancelErr = chip::DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(subscribeId);
        if (cancelErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "OnProxyConnectTimeout: WiFiPAFCancelSubscribe(%u): %" CHIP_ERROR_FORMAT,
                          subscribeId, cancelErr.Format());
        }
    }

    // Remove the PAF session registration that was added in ProxyConnectRequest.
    chip::WiFiPAF::WiFiPAFLayer & pafLayer = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    chip::WiFiPAF::WiFiPAFSession keyNodeInfo{};
    keyNodeInfo.nodeId        = static_cast<chip::NodeId>(ctx->discriminator);
    keyNodeInfo.discriminator = ctx->discriminator;
    CHIP_ERROR rmErr = pafLayer.RmPafSession(chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, keyNodeInfo);
    if (rmErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "OnProxyConnectTimeout: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr.Format());
    }

    // Return Status::Timeout per spec.
    if (cmd != nullptr)
        cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Timeout);
    delete ctx;
}

// Called when WiFiPAF connect (subscribe + PAF handshake) succeeds.
static void OnPafConnectSuccess(void * context)
{
    // Use sPendingConnectCtx (not the raw context pointer) so that if the
    // platform later fires OnPafConnectError for the same subscribe (e.g. on
    // PAFTP endpoint timeout), the second callback sees nullptr and exits.
    auto * ctx = sPendingConnectCtx;
    if (ctx == nullptr)
    {
        ChipLogProgress(AppServer, "OnPafConnectSuccess: no pending connect ctx; ignoring stale callback");
        return;
    }
    sPendingConnectCtx = nullptr;
    // Cancel the ProxyConnectRequest timeout — connect succeeded before it fired.
    chip::DeviceLayer::SystemLayer().CancelTimer(OnProxyConnectTimeout, ctx);

    chip::app::CommandHandler * cmd = ctx->handle.Get();

    // Look up the completed session by discriminator to get peer_id and peer_addr.
    chip::WiFiPAF::WiFiPAFSession keyInfo{};
    keyInfo.discriminator                     = ctx->discriminator;
    chip::WiFiPAF::WiFiPAFLayer & layer       = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    chip::WiFiPAF::WiFiPAFSession * pPafInfo  = layer.GetPAFInfo(chip::WiFiPAF::PafInfoAccess::kAccDisc, keyInfo);

    if (cmd == nullptr || pPafInfo == nullptr)
    {
        ChipLogError(AppServer, "OnPafConnectSuccess: cmd=%p pPafInfo=%p for disc %u",
                     (void *)cmd, (void *)pPafInfo, ctx->discriminator);
        if (cmd != nullptr)
            cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
        delete ctx;
        return;
    }

    uint16_t sessionId = AllocSessionId();
    ProxySessionInfo info{};
    info.pafSession = *pPafInfo;
    sProxySessions[sessionId] = std::move(info);

    ChipLogProgress(AppServer, "ProxyConnectRequest: WiFiPAF connected, proxy session %u (disc %u peer_id %u)",
                    sessionId, ctx->discriminator, pPafInfo->peer_id);

    // Now that the transport connection is actually established, transition the
    // cluster state — this is the correct place, not HandleProxyConnectRequest.
    if (ctx->cluster)
    {
        CHIP_ERROR stateErr = ctx->cluster->SetCPState(
            chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster::kState_CPConnected);
        if (stateErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "OnPafConnectSuccess: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
        }
    }

    chip::app::Clusters::CommissioningProxy::Commands::ProxyConnectResponse::Type response;
    response.sessionId = sessionId;
    cmd->AddResponse(ctx->path, response);
    delete ctx;
}

// Called when WiFiPAF connect fails (or when a PAFTP endpoint times out after
// an already-successful connect fires kCHIPoWiFiPAFCancelConnect).
static void OnPafConnectError(void * context, CHIP_ERROR err)
{
    // Guard: if success already ran and cleared sPendingConnectCtx, the raw
    // context pointer is stale (deleted).  Do NOT dereference it.
    auto * ctx = sPendingConnectCtx;
    if (ctx == nullptr)
    {
        ChipLogProgress(AppServer, "OnPafConnectError: ignoring stale callback after successful connect "
                                   "(err: %" CHIP_ERROR_FORMAT ")", err.Format());
        return;
    }
    sPendingConnectCtx = nullptr;
    // Cancel the ProxyConnectRequest timeout — error callback is handling this instead.
    chip::DeviceLayer::SystemLayer().CancelTimer(OnProxyConnectTimeout, ctx);

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    ChipLogError(AppServer, "ProxyConnectRequest: WiFiPAF connect failed: %" CHIP_ERROR_FORMAT, err.Format());

    // Remove the PAF session registration added in ProxyConnectRequest so the
    // WiFiPAF layer does not hold a stale entry for this discriminator.
    chip::WiFiPAF::WiFiPAFSession keyInfo{};
    keyInfo.nodeId        = static_cast<chip::NodeId>(ctx->discriminator);
    keyInfo.discriminator = ctx->discriminator;
    CHIP_ERROR rmErr = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(
        chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, keyInfo);
    if (rmErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "OnPafConnectError: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr.Format());
    }

    if (cmd != nullptr)
        cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
    delete ctx;
}

// This is a callback that called from FinishWiFiPAFScan (in platform layer)
// to keep the application CommandHandler seperated (in app layer) as per
// the SDK architecture
static void OnPafScanDone(void * context,
                          const std::vector<chip::DeviceLayer::NanPeerInfo> & peers)
{
    auto * ctx = static_cast<ProxyScanCtx *>(context);
    chip::app::CommandHandler * cmd = ctx->handle.Get();

    if (cmd == nullptr)
    {
        delete ctx;
        return;
    }

    chip::app::Clusters::CommissioningProxy::Commands::ProxyScanResponse::Type response;

    // Build output list; must live until AddResponse returns.
    std::vector<ScanResultT> out;
    out.reserve(peers.size());

    for (const auto & p : peers)
    {
        ScanResultT r{};

        r.address.SetNonNull(chip::ByteSpan(p.mac, sizeof(p.mac)));

        // NOTE: transport field type is BitMask<CapabilitiesBitmap> in the struct
        r.transport = chip::BitMask<chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap>(
            chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap::kWiFiPAF);

        r.discriminator = p.discriminator;
        r.vendorId      = static_cast<chip::VendorId>(p.vid);
        r.productId     = p.pid;

        if (p.hasExtendedData && !p.storage.empty())
        {
            r.extendedData.SetNonNull(chip::ByteSpan(p.storage.data(), p.storage.size()));
        }
        else
        {
            r.extendedData.SetNull();
        }

        // Optional wifi band
        r.wiFiBand.ClearValue();

        out.push_back(r);
    }

    chip::app::DataModel::List<const ScanResultT> list{ chip::Span<const ScanResultT>(out.data(), out.size()) };
    response.proxyScanResult = list;
    response.numberOfResults = static_cast<uint8_t>(out.size());

    // AddResponse sends the ProxyScanResponse
    cmd->AddResponse(ctx->path, response);

    delete ctx;
}

// ------------------------------------------------------------------
// Background scan state
//
// Maintains the cache of NAN-discovered devices for
// ProxyBackgroundScanStartRequest.  Each entry has a per-result TTL timer;
// the discriminator is used as the timer appState (cast to void*) so the
// right entry can be removed when the timer fires.
// ------------------------------------------------------------------

struct BgScanEntry
{
    // Store NanPeerInfo (which owns mac[6] and the extendedData storage vector)
    // rather than ScanResultT (which only holds non-owning ByteSpans).
    // NanPeerToScanResult is called at encode time while this entry is alive.
    chip::DeviceLayer::NanPeerInfo peer;
};

// discriminator → cached scan result
static std::map<uint16_t, BgScanEntry> sBgScanCache;

// Identity of the commissioner that started the background scan (for stop matching).
static chip::FabricIndex sBgScanFabricIndex = chip::kUndefinedFabricIndex;
static chip::NodeId      sBgScanNodeId      = chip::kUndefinedNodeId;

static bool sBgScanActive = false;

// Back-pointer so background scan callbacks can notify attribute change.
static chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * sBgScanCluster = nullptr;

// Called when a per-result TTL timer fires: remove the stale entry and
// notify the cluster that CachedResults has changed.
static void OnBgScanTTLExpiry(chip::System::Layer * /*layer*/, void * appState)
{
    uint16_t discriminator = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(appState));
    if (sBgScanCache.erase(discriminator) > 0)
    {
        ChipLogProgress(AppServer, "BgScan: TTL expired for discriminator %u; cache size=%zu",
                        discriminator, sBgScanCache.size());
        if (sBgScanCluster != nullptr)
            sBgScanCluster->MarkCachedResultsDirty();
    }
}

// Convert a NanPeerInfo into the ScanResultStruct used by CachedResults.
static ScanResultT NanPeerToScanResult(const chip::DeviceLayer::NanPeerInfo & p)
{
    ScanResultT r{};
    r.address.SetNonNull(chip::ByteSpan(p.mac, sizeof(p.mac)));
    r.transport = chip::BitMask<chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap>(
        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap::kWiFiPAF);
    r.discriminator = p.discriminator;
    r.vendorId      = static_cast<chip::VendorId>(p.vid);
    r.productId     = p.pid;
    if (p.hasExtendedData && !p.storage.empty())
        r.extendedData.SetNonNull(chip::ByteSpan(p.storage.data(), p.storage.size()));
    else
        r.extendedData.SetNull();
    r.wiFiBand.ClearValue();
    return r;
}

// Per-peer callback fired by WiFiPAFStartBackgroundScan on every discovery
// result, including re-discoveries (so TTL can be reset).
static void OnBgScanDiscovery(void * /*ctx*/, const chip::DeviceLayer::NanPeerInfo & peer)
{
    uint16_t disc = peer.discriminator;

    // Cancel any existing TTL timer for this discriminator (reset on rediscovery).
    chip::DeviceLayer::SystemLayer().CancelTimer(OnBgScanTTLExpiry, reinterpret_cast<void *>(static_cast<uintptr_t>(disc)));

    auto it    = sBgScanCache.find(disc);
    bool isNew = (it == sBgScanCache.end());

    if (isNew)
    {
        // New entry: check capacity and drop silently when at MaxCachedResults.
        uint8_t maxResults = (sBgScanCluster != nullptr)
            ? sBgScanCluster->GetDelegate().GetMaxCachedResults()
            : 10;
        if (sBgScanCache.size() >= static_cast<size_t>(maxResults))
        {
            ChipLogDetail(AppServer, "BgScan: cache full (%u entries), dropping discriminator %u",
                          maxResults, disc);
            return;
        }
    }

    // Retrieve CacheTimeout from the delegate.
    uint16_t cacheTimeout = (sBgScanCluster != nullptr)
        ? static_cast<uint16_t>(sBgScanCluster->GetDelegate().GetCacheTimeout())
        : 30;

    // Insert or update the cache entry (stores a copy of NanPeerInfo).
    // Note: on rediscovery (isNew == false) the entry data is refreshed in case
    // any fields changed (e.g. extended data), but subscribers are NOT notified
    // because the discriminator — the unique device identity — is unchanged.
    // If a device were to change its MAC while keeping its discriminator, the
    // update would be silent.  This is acceptable for current hardware.
    sBgScanCache[disc] = BgScanEntry{ peer };

    ChipLogProgress(AppServer, "BgScan: %s discriminator %u (cache size=%zu, TTL=%us)",
                    isNew ? "cached" : "refreshed TTL for", disc, sBgScanCache.size(), cacheTimeout);

    // (Re)start per-result TTL timer.
    chip::DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Seconds16(cacheTimeout),
        OnBgScanTTLExpiry,
        reinterpret_cast<void *>(static_cast<uintptr_t>(disc)));

    // Only notify subscribers when the cache content actually changes (new entry).
    // Rediscovery of an existing device only refreshes the TTL; the visible data
    // is unchanged so no subscription notification should be sent.
    if (isNew && sBgScanCluster != nullptr)
        sBgScanCluster->MarkCachedResultsDirty();
}

} // namespace

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyConnectRequest(
                        DataModel::Nullable<chip::ByteSpan> address,
                        CapabilitiesBitmap transport,
                        uint16_t discriminator,
                        chip::VendorId vendorid,
                        uint16_t productid,
                        uint16_t timeout,
                        WiFiBandBitmap wiFiBand,
                        app::CommandHandler * commandObj,
                        const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "===SHM %s(), transport:%d wiFiBand:%d timeout:%u discriminator:%u",
                    __func__, (int)transport, (int)wiFiBand, timeout, discriminator);

    // Init PAF layer (idempotent).
    CHIP_ERROR initErr = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().Init(&DeviceLayer::SystemLayer());
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
    // A synthetic non-zero nodeId derived from the discriminator is used
    // because AddPafSession(kAccNodeInfo, ...) requires nodeId != 0.
    chip::WiFiPAF::WiFiPAFSession pafSessionInfo{};
    pafSessionInfo.role          = chip::WiFiPAF::kWiFiPafRole_Subscriber;
    pafSessionInfo.nodeId        = static_cast<chip::NodeId>(discriminator);
    pafSessionInfo.discriminator = discriminator;
    CHIP_ERROR addErr = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().AddPafSession(
        chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, pafSessionInfo);
    if (addErr != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: AddPafSession failed: %" CHIP_ERROR_FORMAT, addErr.Format());
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    // Keep the IM invoke alive until the PAF connect succeeds or fails.
    // Pass mServer so OnPafConnectSuccess can update the cluster state.
    uint16_t effectiveTimeout = timeout > 0 ? timeout : kProxyConnectDefaultTimeoutSecs;
    auto * ctx = new ProxyConnectCtx{ chip::app::CommandHandler::Handle(commandObj), request.path, discriminator, 0, mServer };
    sPendingConnectCtx = ctx;

    if (auto * exchange = commandObj->GetExchangeContext())
    {
        // Extend response timeout to cover PAF discovery + handshake (buffer of +5 s
        // so the exchange stays open until our application-level timer fires first).
        exchange->SetResponseTimeout(chip::System::Clock::Seconds16(effectiveTimeout + 5));
    }

    // Initiate WiFiPAF NAN subscribe → discovery → PAF handshake.
    // OnPafConnectSuccess / OnPafConnectError send the ProxyConnectResponse.
    CHIP_ERROR err = chip::DeviceLayer::ConnectivityMgr().WiFiPAFSubscribe(
        discriminator, ctx,
        OnPafConnectSuccess,
        OnPafConnectError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: WiFiPAFSubscribe failed: %" CHIP_ERROR_FORMAT, err.Format());
        CHIP_ERROR rmErr = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(
            chip::WiFiPAF::PafInfoAccess::kAccNodeInfo, pafSessionInfo);
        if (rmErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "ProxyConnectRequest cleanup: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr.Format());
        }
        sPendingConnectCtx = nullptr;
        delete ctx;
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    // Store the subscribe_id assigned by wpa_supplicant so OnProxyConnectTimeout
    // can cancel the correct NAN session (GetPAFInfo cannot be used here: it
    // short-circuits on publisher entries before reaching the subscriber).
    ctx->subscribeId = chip::DeviceLayer::ConnectivityMgrImpl().GetPendingConnectSubscribeId();

    // Start the timeout timer now that subscribeId is populated — so if it fires
    // immediately it has a valid id to cancel.
    chip::DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Seconds16(effectiveTimeout), OnProxyConnectTimeout, ctx);

    ChipLogProgress(AppServer, "ProxyConnectRequest: WiFiPAFSubscribe started for discriminator %u (subscribe_id %u)",
                    discriminator, ctx->subscribeId);

    // Response sent asynchronously via OnPafConnectSuccess / OnPafConnectError.
    return chip::Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyScanRequest(
    CapabilitiesBitmap transport, WiFiBandBitmap wiFiBands, app::CommandHandler * commandObj,
    const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "===SHM %s(), transport:%d wiFiBands:%d", __func__, (int) transport, (int) wiFiBands);

    // Start PAF (you already do this)
    CHIP_ERROR err = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().Init(&DeviceLayer::SystemLayer());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Commissioning Proxy PAF Scan Request Failed");
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    uint8_t scanMaxTime = GetScanMaxTime();

    // Hold the invoke open and move it into ConnectivityManagerImpl
    // This keeps the ProxyScanRequest open, so the scan can complete before the ProxyScanResponse is sent
    // Scan results are processed in ConnectivityManagerImpl::ScanDiscoveryResult()
    // scanMaxTime expiry handled in ConnectivityManagerImpl::FinishWiFiPAFScan()
    // Callback OnPafScanDone() sends the ProxyScanResponse to the client
    auto * ctx = new ProxyScanCtx{ chip::app::CommandHandler::Handle(commandObj), request.path, scanMaxTime };

    err = chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFScan(scanMaxTime, &OnPafScanDone, ctx);
    if (err != CHIP_NO_ERROR)
    {
        delete ctx;
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    // Ensure response timeout is > scan time
    if (auto * exchange = commandObj->GetExchangeContext())
    {
        exchange->SetResponseTimeout(chip::System::Clock::Seconds16(scanMaxTime + 5));
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyMessageRequest(
    uint16_t sessionId,
    chip::Optional<chip::ByteSpan> message,
    uint8_t responseTimeout,
    app::CommandHandler * commandObj,
    const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "===SHM %s(): sessionId=%u msgLen=%zu",
                    __func__, sessionId,
                    message.HasValue() ? message.Value().size() : 0);

    // Look up the proxy session.
    auto it = sProxySessions.find(sessionId);
    if (it == sProxySessions.end())
    {
        ChipLogError(AppServer, "ProxyMessageRequest: unknown sessionId %u", sessionId);
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    // Per spec §10.168: reject if another ProxyMessageRequest for this session
    // is still outstanding.  However, if the exchange for the previous request
    // has already timed out (handle.Get() == nullptr), clean it up rather than
    // permanently blocking the session.
    auto pendingIt = sPendingProxyMsgCtx.find(sessionId);
    if (pendingIt != sPendingProxyMsgCtx.end())
    {
        ProxyMsgCtx * existing = pendingIt->second;
        if (existing->handle.Get() != nullptr)
        {
            ChipLogError(AppServer, "ProxyMessageRequest: session %u already has a pending request (BUSY)", sessionId);
            return chip::Protocols::InteractionModel::Status::Busy;
        }
        // Previous request timed out without a commissionee reply; clean up so
        // this session is not permanently stuck.
        ChipLogProgress(AppServer, "ProxyMessageRequest: cleaning up expired pending ctx for session %u", sessionId);
        delete existing;
        sPendingProxyMsgCtx.erase(pendingIt);
    }

    if (!message.HasValue() || message.Value().empty())
    {
        // Per spec §10.161: null/empty message means the Commissioner is polling
        // for a queued response from the Commissionee.  Respond immediately with
        // ProxyMessageResponse(null) — nothing is queued.
        chip::app::Clusters::CommissioningProxy::Commands::ProxyMessageResponse::Type pollResponse;
        pollResponse.sessionId = sessionId;
        pollResponse.message.SetNull();
        commandObj->AddResponse(request.path, pollResponse);
        return chip::Protocols::InteractionModel::Status::Success;
    }

    // Build a PacketBuffer containing the raw Matter bytes to forward.
    chip::System::PacketBufferHandle buf =
        chip::System::PacketBufferHandle::NewWithData(message.Value().data(), message.Value().size());
    if (buf.IsNull())
    {
        ChipLogError(AppServer, "ProxyMessageRequest: out of memory");
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    // Keep the invoke handler alive until the commissionee replies.
    // sPendingProxyMsgCtx takes ownership; cleaned up in the WiFiPAF receive callback.
    auto * ctx = new ProxyMsgCtx{ chip::app::CommandHandler::Handle(commandObj), request.path, sessionId };
    sPendingProxyMsgCtx[sessionId] = ctx;

    // Extend the exchange response timeout to accommodate the WiFiPAF round-trip.
    if (auto * exchange = commandObj->GetExchangeContext())
    {
        exchange->SetResponseTimeout(
            chip::System::Clock::Seconds16(responseTimeout > 0 ? responseTimeout : 5));
    }

    // Forward the raw Matter packet over WiFiPAF to the commissionee.
    // MUST use WiFiPAFLayer::SendMessage (not ConnectivityMgrImpl().WiFiPAFSend):
    // WiFiPAFSend is the low-level raw NAN transmit used internally by PAFTP;
    // calling it directly bypasses PAFTP framing and the end device cannot
    // parse the frame.  SendMessage goes through the PAFTP endpoint which
    // fragments and frames the message before calling WiFiPAFSend.
    CHIP_ERROR err = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().SendMessage(
        it->second.pafSession, std::move(buf));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyMessageRequest: SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
        sPendingProxyMsgCtx.erase(sessionId);
        delete ctx;
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    // Response will be sent asynchronously via OnProxyWiFiPAFMessageReceived()
    // when the commissionee's reply arrives.
    return chip::Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyDisconnectRequest(
    uint16_t sessionId)
{
    ChipLogProgress(AppServer, "===SHM %s(): sessionId=%u", __func__, sessionId);

    auto it = sProxySessions.find(sessionId);
    if (it == sProxySessions.end())
    {
        ChipLogError(AppServer, "ProxyDisconnectRequest: unknown sessionId %u", sessionId);
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    // Release any outstanding ProxyMessageRequest context for this session.
    // Do NOT call AddStatus() here: if the commissioner has already closed the
    // TCP connection, AddStatus() triggers synchronous session teardown which
    // evicts the ProxyDisconnectRequest exchange before this handler returns.
    // The cluster framework then tries to send the ProxyDisconnectRequest
    // response on the already-defunct exchange, crashing via VerifyOrDie.
    // Releasing the handle here is safe — the commissioner sent
    // ProxyDisconnectRequest because it is done with the session, so it will
    // not be waiting for a further ProxyMessageResponse.
    auto pendingIt = sPendingProxyMsgCtx.find(sessionId);
    if (pendingIt != sPendingProxyMsgCtx.end())
    {
        delete pendingIt->second;
        sPendingProxyMsgCtx.erase(pendingIt);
    }

    // Remove the PAF session from the WiFiPAF layer so it stops routing
    // further messages for this session.  Use kAccSessionId: the subscribe_id
    // (stored as pafSession.id by the platform during WiFiPAFSubscribe) is the
    // only key RmPafSession matched reliably.
    chip::WiFiPAF::WiFiPAFSession pafSession = it->second.pafSession;
    CHIP_ERROR rmErr = chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(
        chip::WiFiPAF::PafInfoAccess::kAccSessionId, pafSession);
    if (rmErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "ProxyDisconnectRequest: RmPafSession for session %u: %" CHIP_ERROR_FORMAT,
                      sessionId, rmErr.Format());
    }

    // Explicitly close the PAFTP endpoint so its ack-received timer does not
    // fire 30 s later.  RmPafSession only removes the session from the lookup
    // table; CloseEndPoint calls DoClose() on the endpoint itself, cancelling
    // all pending timers and triggering the NAN subscription cancel.
    chip::WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().CloseEndPoint(pafSession);

    sProxySessions.erase(it);
    ChipLogProgress(AppServer, "ProxyDisconnectRequest: session %u cleaned up", sessionId);
    return chip::Protocols::InteractionModel::Status::Success;
}

// Helper: cancel all per-result TTL timers and clear the background scan cache.
static void ClearBgScanCache()
{
    for (auto & [disc, entry] : sBgScanCache)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(
            OnBgScanTTLExpiry,
            reinterpret_cast<void *>(static_cast<uintptr_t>(disc)));
    }
    sBgScanCache.clear();
}

// Called when the scan lifetime timer (Timeout field > 0) expires.
static void OnBgScanLifetimeExpiry(chip::System::Layer * /*layer*/, void * /*appState*/)
{
    if (!sBgScanActive)
        return;
    ChipLogProgress(AppServer, "BgScan: scan lifetime expired");
    chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStopBackgroundScan();
    ClearBgScanCache();
    sBgScanActive     = false;
    sBgScanFabricIndex = chip::kUndefinedFabricIndex;
    sBgScanNodeId     = chip::kUndefinedNodeId;
    if (sBgScanCluster != nullptr)
    {
        sBgScanCluster->MarkCachedResultsDirty();
        sBgScanCluster = nullptr;
    }
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyBackgroundScanStartRequest(
    CapabilitiesBitmap transport,
    uint16_t timeout,
    WiFiBandBitmap wiFiBands,
    chip::FabricIndex fabricIndex,
    chip::NodeId nodeId,
    app::CommandHandler * commandObj,
    const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "===SHM %s() transport:%u timeout:%u fabricIndex:%u nodeId:0x" ChipLogFormatX64,
                    __func__, static_cast<uint8_t>(transport), timeout, fabricIndex, ChipLogValueX64(nodeId));

    if (sBgScanActive)
    {
        // Per spec: if the same commissioner re-issues the command, restart the scan.
        // If a different commissioner's scan is running, return Busy.
        if (sBgScanFabricIndex != fabricIndex || sBgScanNodeId != nodeId)
        {
            ChipLogError(AppServer, "ProxyBackgroundScanStartRequest: scan already running for different commissioner");
            return chip::Protocols::InteractionModel::Status::Busy;
        }

        // Same commissioner: stop the current scan and restart below.
        ChipLogProgress(AppServer, "ProxyBackgroundScanStartRequest: restarting scan for same commissioner");
        chip::DeviceLayer::SystemLayer().CancelTimer(OnBgScanLifetimeExpiry, nullptr);
        chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStopBackgroundScan();
        ClearBgScanCache();
        sBgScanActive = false;
    }

    sBgScanCluster    = mServer;
    sBgScanFabricIndex = fabricIndex;
    sBgScanNodeId     = nodeId;

    CHIP_ERROR err = chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStartBackgroundScan(OnBgScanDiscovery, nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyBackgroundScanStartRequest: WiFiPAFStartBackgroundScan failed: %" CHIP_ERROR_FORMAT,
                     err.Format());
        sBgScanCluster    = nullptr;
        sBgScanFabricIndex = chip::kUndefinedFabricIndex;
        sBgScanNodeId     = chip::kUndefinedNodeId;
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    sBgScanActive = true;
    ChipLogProgress(AppServer, "ProxyBackgroundScanStartRequest: background scan started");

    // If timeout > 0, start a scan-lifetime timer.  timeout = 0 means infinite (spec §10.5.7.6).
    if (timeout > 0)
    {
        chip::DeviceLayer::SystemLayer().StartTimer(
            chip::System::Clock::Seconds16(timeout), OnBgScanLifetimeExpiry, nullptr);
    }

    // Synchronous response — IM status Success; no response struct.
    return chip::Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyBackgroundScanStopRequest(
    CapabilitiesBitmap transport,
    WiFiBandBitmap wiFiBands,
    chip::FabricIndex fabricIndex,
    chip::NodeId nodeId)
{
    ChipLogProgress(AppServer, "===SHM %s() transport:%u fabricIndex:%u nodeId:0x" ChipLogFormatX64,
                    __func__, static_cast<uint8_t>(transport), fabricIndex, ChipLogValueX64(nodeId));

    // Per spec: silently ignore if NodeId+FabricIndex don't match the
    // original requester, or if no background scan is currently running.
    if (!sBgScanActive || sBgScanFabricIndex != fabricIndex || sBgScanNodeId != nodeId)
    {
        ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: no matching active scan; ignoring");
        return chip::Protocols::InteractionModel::Status::Success;
    }

    // Cancel the scan-lifetime timer if one was started.
    chip::DeviceLayer::SystemLayer().CancelTimer(OnBgScanLifetimeExpiry, nullptr);

    chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFStopBackgroundScan();
    ClearBgScanCache();

    sBgScanActive     = false;
    sBgScanFabricIndex = chip::kUndefinedFabricIndex;
    sBgScanNodeId     = chip::kUndefinedNodeId;

    if (sBgScanCluster != nullptr)
    {
        sBgScanCluster->MarkCachedResultsDirty();
        sBgScanCluster = nullptr;
    }

    ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: background scan stopped");
    return chip::Protocols::InteractionModel::Status::Success;
}

uint8_t Clusters::CommissioningProxy::MyCPDelegate::GetNumCachedResults()
{
    return static_cast<uint8_t>(sBgScanCache.size());
}

CHIP_ERROR Clusters::CommissioningProxy::MyCPDelegate::EncodeCachedResults(app::AttributeValueEncoder & encoder)
{
    if (!sBgScanActive || sBgScanCache.empty())
    {
        // Encode the attribute as null (no results / scan not active).
        chip::app::DataModel::Nullable<chip::app::DataModel::List<const ScanResultT>> nullValue;
        return encoder.Encode(nullValue);
    }

    return encoder.EncodeList([](const auto & listEncoder) -> CHIP_ERROR {
        for (const auto & [disc, entry] : sBgScanCache)
        {
            // NanPeerToScanResult creates ByteSpans into entry.peer which is
            // alive for the duration of this EncodeList call.
            ReturnErrorOnFailure(listEncoder.Encode(NanPeerToScanResult(entry.peer)));
        }
        return CHIP_NO_ERROR;
    });
}
