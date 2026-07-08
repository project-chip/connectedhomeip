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
#include "CommissioningProxyBleTransport.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <ble/Ble.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <lib/support/SetupDiscriminator.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <platform/Linux/BLEManagerImpl.h>
#include <platform/PlatformManager.h>

#include <cstring>
#include <map>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {
namespace Ble {
namespace {

using chip::Ble::BLEEndPoint;
using chip::Ble::BleLayer;
using chip::Ble::BleLayerDelegate;

// Active proxy sessions: sessionId → BLEEndPoint owned by the proxy.
static std::map<uint16_t, BLEEndPoint *> sEndpoints;

// Context kept alive for the duration of an asynchronous ProxyConnectRequest.
struct ConnectCtx
{
    chip::app::CommandHandler::Handle handle;
    chip::app::ConcreteCommandPath path;
    uint16_t discriminator;
    chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * cluster = nullptr;
    chip::FabricIndex fabricIndex                                                = chip::kUndefinedFabricIndex;
    BLEEndPoint * endpoint                                                       = nullptr; // set when L2CAP wrap completes
};

// Single in-flight proxy connect (MaxSessions=1 shared across transports).
static ConnectCtx * sPendingConnect = nullptr;
// Endpoint that owns a close we initiated but do not want forwarded to mOriginalTransport.
// Set in two cases: (a) mid-BTP-handshake endpoint being closed on timeout/cancel/failure
// (set in OnBleConnectionComplete, cleared by OnEndPointConnectionClosed); (b) a promoted
// session being closed by Disconnect() (set/cleared around ep->Close()).
// OnEndPointConnectionClosed matches this sentinel and returns early — no forwarding.
static BLEEndPoint * sBtpHandshakeEndpoint = nullptr;

// Host cluster (set via the transport's SetHost). All transport-agnostic
// bookkeeping (sessions, message routing, scan cache/aggregation) is reached
// through its subsystem accessors.
static chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * sHost = nullptr;

// ------------------------------------------------------------------
// Foreground scan state (ProxyScanRequest path)
// ------------------------------------------------------------------

struct BleScanResult
{
    uint8_t mac[6];
    uint16_t discriminator;
    uint16_t vid;
    uint16_t pid;
};

static std::vector<BleScanResult> sScanResults;
static bool sScanInProgress = false;

// ------------------------------------------------------------------
// Background scan state (ProxyBackGroundScanStartRequest path)
//
// Mirrors the PAF transport's background-scan bookkeeping: a per-fabric record
// of the requested transport (kBle only here) plus a per-device result cache
// with TTL.  BLE has a single ChipDeviceScanner shared with the foreground scan
// and with connect (NewBleConnectionByDiscriminator), so the background scan is
// paused (StopProxyScan) whenever one of those needs the scanner and resumed
// afterwards — see PauseBleBgScan / ResumeBleBgScanIfNeeded.
// ------------------------------------------------------------------

struct FabricKey
{
    chip::FabricIndex fabricIndex;
    chip::NodeId nodeId;
    bool operator<(const FabricKey & o) const
    {
        if (fabricIndex != o.fabricIndex)
        {
            return fabricIndex < o.fabricIndex;
        }
        return nodeId < o.nodeId;
    }
};

struct FabricLifetimeCtx
{
    FabricKey key;
};

struct FabricScanRecord
{
    FabricLifetimeCtx * lifetimeCtx = nullptr;
};

static std::map<FabricKey, FabricScanRecord> sBgScanFabrics;
// True when fabrics are registered but the hardware scan is suspended because a
// connect or foreground scan currently owns the single BLE scanner.
static bool sBgScanPaused                                                                  = false;
static chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * sBgScanCluster = nullptr;

// Build a kBle ScanResultStruct from a discovered device.  Shared by the
// foreground scan and the background-scan discovery callback so the field set
// stays identical across both paths.
using ScanResultT = chip::app::Clusters::CommissioningProxy::Structs::ScanResultStruct::Type;

static ScanResultT MakeScanResult(const uint8_t bdAddr[6], uint16_t discriminator, uint16_t vendorId, uint16_t productId)
{
    ScanResultT r{};
    r.address.SetNonNull(chip::ByteSpan(bdAddr, 6));
    r.transport = chip::BitMask<chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap>(
        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap::kBle);
    r.discriminator = discriminator;
    r.vendorID      = static_cast<chip::VendorId>(vendorId);
    r.productID     = productId;
    r.extendedData.SetNull();
    r.wiFiBand.ClearValue();
    return r;
}

// Result cache, TTL, and combined MaxCachedResults cap live in the shared
// BgScanCache module; this callback just builds the kBle result and reports it.
// Runs on the Matter event loop (dispatched via ScheduleWork by ProxyScanForwarder).
static void OnBgScanDiscovery(void * /*context*/, const uint8_t bdAddr[6], uint16_t discriminator, uint16_t vendorId,
                              uint16_t productId)
{
    if (sBgScanCluster != nullptr)
    {
        sBgScanCluster->ScanCache().Report(MakeScanResult(bdAddr, discriminator, vendorId, productId));
    }
}

static void OnBgScanLifetimeExpiry(chip::System::Layer * /*layer*/, void * appState)
{
    auto * ctx    = static_cast<FabricLifetimeCtx *>(appState);
    FabricKey key = ctx->key;
    delete ctx;

    auto it = sBgScanFabrics.find(key);
    if (it == sBgScanFabrics.end())
    {
        return;
    }
    it->second.lifetimeCtx = nullptr;
    sBgScanFabrics.erase(it);

    ChipLogProgress(AppServer, "BLE BgScan: lifetime expired for fabricIndex=%u nodeId=0x" ChipLogFormatX64, key.fabricIndex,
                    ChipLogValueX64(key.nodeId));

    if (sBgScanFabrics.empty())
    {
        if (!sBgScanPaused)
        {
            (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
        }
        sBgScanPaused = false;
        if (sBgScanCluster != nullptr)
        {
            sBgScanCluster->ScanCache().ClearTransport(CapabilitiesBitmap::kBle);
        }
        sBgScanCluster = nullptr;
    }
}

// ------------------------------------------------------------------
// BLE-only helpers (no PAF equivalent)
// ------------------------------------------------------------------

static BleLayer * GetBleLayer()
{
    return chip::DeviceLayer::ConnectivityMgr().GetBleLayer();
}

// Start (or resume) the hardware background scan if it is not already running.
// Returns CHIP_ERROR_BUSY if the BLE scanner is currently held by a connect or
// foreground scan — the caller keeps the fabric registered and the scan resumes
// later via ResumeBleBgScanIfNeeded.
static CHIP_ERROR StartBleBgHardwareScan()
{
    CHIP_ERROR err = chip::DeviceLayer::Internal::BLEMgrImpl().StartProxyScan(OnBgScanDiscovery, nullptr);
    if (err == CHIP_NO_ERROR)
    {
        sBgScanPaused = false;
    }
    return err;
}

// Suspend the hardware background scan so a connect or foreground scan can use
// the single BLE scanner.  Idempotent; no-op when no fabrics are registered.
static void PauseBleBgScan()
{
    if (!sBgScanFabrics.empty() && !sBgScanPaused)
    {
        (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
        sBgScanPaused = true;
        ChipLogProgress(AppServer, "BLE BgScan: paused (scanner needed for connect/foreground scan)");
    }
}

// Resume a previously paused hardware background scan when the scanner frees up.
static void ResumeBleBgScanIfNeeded()
{
    if (sBgScanPaused && !sBgScanFabrics.empty())
    {
        CHIP_ERROR err = StartBleBgHardwareScan();
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(AppServer, "BLE BgScan: resumed");
        }
        else
        {
            ChipLogError(AppServer, "BLE BgScan: resume failed: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
}

// Look up the sessionId for an endpoint already promoted to sEndpoints.
// Returns true if found; writes the id to outSessionId.
static bool FindSessionId(BLEEndPoint * ep, uint16_t & outSessionId)
{
    for (auto & [sid, e] : sEndpoints)
    {
        if (e == ep)
        {
            outSessionId = sid;
            return true;
        }
    }
    return false;
}

// Forward-declarations for helpers and IM-thread connect callbacks (defined below the delegate).
static void FailPendingConnect(chip::Protocols::InteractionModel::Status status);
static void OnConnectFound(void * appState, BLE_CONNECTION_OBJECT connObj);
static void OnConnectError(void * appState, CHIP_ERROR err);
static void OnConnectTimeout(chip::System::Layer * layer, void * appState);

// ------------------------------------------------------------------
// BleLayerDelegate wrapper.
//
// Wraps the original BleLayer::mBleTransport (set by transport/raw/BLE.cpp's
// BLEBase) so that BLE central events for proxy-owned endpoints are routed
// back to the commissioner via the dispatcher; all other events fall through
// to the original delegate (e.g. the proxy app's own peripheral commissioning).
// ------------------------------------------------------------------
class ProxyBleDelegate : public BleLayerDelegate
{
public:
    void Install()
    {
        auto * layer = GetBleLayer();
        if (layer == nullptr)
        {
            ChipLogError(AppServer, "ProxyBleDelegate::Install: no BleLayer");
            return;
        }
        if (layer->mBleTransport == this)
        {
            return;
        }
        mOriginalTransport   = layer->mBleTransport;
        layer->mBleTransport = this;
        ChipLogProgress(AppServer, "ProxyBleDelegate: installed (original=%p)", (void *) mOriginalTransport);
    }

    void OnBleConnectionComplete(BLEEndPoint * endpoint) override
    {
        // Central role: BleLayer just wrapped a new L2CAP connection in an
        // endpoint.  If we have a pending proxy connect with no endpoint yet,
        // this one is ours.  Initiate the BTP handshake on it.
        if (sPendingConnect != nullptr && sPendingConnect->endpoint == nullptr)
        {
            sPendingConnect->endpoint = endpoint;
            sBtpHandshakeEndpoint     = endpoint;
            CHIP_ERROR err            = endpoint->StartConnect();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "ProxyBleDelegate: StartConnect failed: %" CHIP_ERROR_FORMAT, err.Format());
                // The endpoint's own DoClose path will fire OnEndPointConnectComplete
                // with the error; that path completes the IM response below.
            }
            return;
        }
        if (mOriginalTransport != nullptr)
        {
            mOriginalTransport->OnBleConnectionComplete(endpoint);
        }
    }

    void OnBleConnectionError(CHIP_ERROR err) override
    {
        // Central role: pre-BTP-handshake error (scan failure, L2CAP rejected, etc.).
        if (sPendingConnect != nullptr && sPendingConnect->endpoint == nullptr)
        {
            ChipLogError(AppServer, "ProxyBleDelegate: OnBleConnectionError: %" CHIP_ERROR_FORMAT, err.Format());
            FailPendingConnect(chip::Protocols::InteractionModel::Status::Failure);
            return;
        }
        if (mOriginalTransport != nullptr)
        {
            mOriginalTransport->OnBleConnectionError(err);
        }
    }

    void OnEndPointConnectComplete(BLEEndPoint * endpoint, CHIP_ERROR err) override
    {
        // BTP handshake completed (or failed).  If this endpoint is ours,
        // complete the ProxyConnectRequest exchange.
        if (sPendingConnect != nullptr && sPendingConnect->endpoint == endpoint)
        {
            chip::DeviceLayer::SystemLayer().CancelTimer(OnConnectTimeout, nullptr);
            auto * ctx                      = sPendingConnect;
            sPendingConnect                 = nullptr;
            chip::app::CommandHandler * cmd = ctx->handle.Get();

            if (err != CHIP_NO_ERROR || cmd == nullptr)
            {
                ChipLogError(AppServer, "ProxyBleDelegate: OnEndPointConnectComplete err=%" CHIP_ERROR_FORMAT " cmd=%p",
                             err.Format(), (void *) cmd);
                if (cmd != nullptr)
                {
                    cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
                }
                if (endpoint != nullptr)
                {
                    endpoint->Close();
                }
                // Clear the sentinel after Close(): if Close() fired synchronously
                // (normal BTP failure), OnEndPointConnectionClosed already cleared it;
                // if Close() was a no-op (StartConnect failed internally and the
                // endpoint was already kState_Closed), the sentinel remains stale and
                // must be cleared here.
                sBtpHandshakeEndpoint = nullptr;
                delete ctx;
                // BTP handshake failed: no session was established, so release the
                // scanner back to a background scan paused for this connect.
                ResumeBleBgScanIfNeeded();
                return;
            }

            if (ctx->cluster == nullptr)
            {
                ChipLogError(AppServer, "ProxyBleDelegate: cluster gone at connect complete; closing endpoint");
                if (endpoint != nullptr)
                {
                    endpoint->Close();
                }
                sBtpHandshakeEndpoint = nullptr;
                cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
                delete ctx;
                ResumeBleBgScanIfNeeded();
                return;
            }

            sBtpHandshakeEndpoint = nullptr; // moved to sEndpoints below
            uint16_t sessionId    = ctx->cluster->Sessions().AllocSessionId();
            sEndpoints[sessionId] = endpoint;
            ctx->cluster->Sessions().RegisterSession(sessionId, CapabilitiesBitmap::kBle, ctx->fabricIndex);

            ChipLogProgress(AppServer, "ProxyConnectRequest: BLE connected, proxy session %u (disc %u)", sessionId,
                            ctx->discriminator);

            CHIP_ERROR stateErr =
                ctx->cluster->SetCPState(chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster::kState_CPConnected);
            if (stateErr != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "ProxyBleDelegate: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
            }

            chip::app::Clusters::CommissioningProxy::Commands::ProxyConnectResponse::Type response;
            response.sessionID = sessionId;
            cmd->AddResponse(ctx->path, response);
            delete ctx;
            return;
        }
        if (mOriginalTransport != nullptr)
        {
            mOriginalTransport->OnEndPointConnectComplete(endpoint, err);
        }
    }

    void OnEndPointMessageReceived(BLEEndPoint * endpoint, chip::System::PacketBufferHandle && msg) override
    {
        uint16_t sid;
        if (FindSessionId(endpoint, sid))
        {
            // DispatchMessageResponse copies the payload into the IM ProxyMessageResponse
            // synchronously, so msg can be released when this scope returns.
            if (sHost != nullptr)
            {
                sHost->Sessions().DispatchMessageResponse(sid, msg->Start(), msg->DataLength());
            }
            return;
        }
        if (mOriginalTransport != nullptr)
        {
            mOriginalTransport->OnEndPointMessageReceived(endpoint, std::move(msg));
        }
    }

    void OnEndPointConnectionClosed(BLEEndPoint * endpoint, CHIP_ERROR err) override
    {
        uint16_t sid;
        if (FindSessionId(endpoint, sid))
        {
            ChipLogProgress(AppServer, "ProxyBleDelegate: session %u closed: %" CHIP_ERROR_FORMAT, sid, err.Format());
            // Fail any in-flight ProxyMessageRequest for this session and drop it.
            if (sHost != nullptr)
            {
                sHost->Sessions().DispatchMessageFailure(sid, chip::Protocols::InteractionModel::Status::Failure);
                sHost->Sessions().RemoveSession(sid);
            }
            sEndpoints.erase(sid);
            // Peer (commissionee) closed the session. Unlike ProxyDisconnectRequest,
            // this path does not run through the dispatcher's OnAllSessionsClosed(),
            // so resume a background scan paused for the connect once the last
            // session is gone and no connect is mid-flight.
            if (sEndpoints.empty() && sPendingConnect == nullptr)
            {
                ResumeBleBgScanIfNeeded();
            }
            return;
        }
        // Endpoint was mid-BTP-handshake and closed by timeout/cancel/failure before
        // a session was established.  It was never passed to mOriginalTransport, so do
        // not forward the close notification to it.
        if (endpoint != nullptr && endpoint == sBtpHandshakeEndpoint)
        {
            sBtpHandshakeEndpoint = nullptr;
            return;
        }
        if (mOriginalTransport != nullptr)
        {
            mOriginalTransport->OnEndPointConnectionClosed(endpoint, err);
        }
    }

    CHIP_ERROR SetEndPoint(BLEEndPoint * endpoint) override
    {
        if (mOriginalTransport != nullptr)
        {
            return mOriginalTransport->SetEndPoint(endpoint);
        }
        return CHIP_NO_ERROR;
    }

private:
    BleLayerDelegate * mOriginalTransport = nullptr;
};

static ProxyBleDelegate sProxyBleDelegate;

// ------------------------------------------------------------------
// BleConnectionDelegate callbacks: fire from the platform when the L2CAP scan
// + connect step completes (still before BTP handshake).  These run inside
// the Matter event loop.
// ------------------------------------------------------------------

// Tear down a pending connect synchronously and complete the IM exchange with
// the given status.  Caller has already verified sPendingConnect != nullptr.
static void FailPendingConnect(chip::Protocols::InteractionModel::Status status)
{
    chip::DeviceLayer::SystemLayer().CancelTimer(OnConnectTimeout, nullptr);
    auto * ctx                      = sPendingConnect;
    sPendingConnect                 = nullptr;
    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
    {
        cmd->AddStatus(ctx->path, status);
    }
    delete ctx;
    ResumeBleBgScanIfNeeded();
}

static void OnConnectFound(void * /*appState*/, BLE_CONNECTION_OBJECT connObj)
{
    // We pass nullptr for appState to NewBleConnectionByDiscriminator and rely on
    // sPendingConnect being non-null (MaxSessions=1 guarantees uniqueness).
    if (sPendingConnect == nullptr)
    {
        ChipLogProgress(AppServer, "OnConnectFound: no pending connect ctx; ignoring stale callback");
        return;
    }

    // Wrap the raw L2CAP connection into a BLEEndPoint (Central role, autoClose=true)
    // via the no-callback NewBleConnectionByObject path so OnConnectionComplete in
    // BleLayer creates the endpoint and dispatches to our BleLayerDelegate (which
    // is sProxyBleDelegate).
    auto * layer = GetBleLayer();
    if (layer == nullptr)
    {
        ChipLogError(AppServer, "OnConnectFound: BleLayer null");
        FailPendingConnect(chip::Protocols::InteractionModel::Status::Failure);
        return;
    }
    CHIP_ERROR err = layer->NewBleConnectionByObject(connObj);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "OnConnectFound: NewBleConnectionByObject failed: %" CHIP_ERROR_FORMAT, err.Format());
        // No further callback will fire — clean up synchronously rather than
        // waiting for the connect timer to expire.
        FailPendingConnect(chip::Protocols::InteractionModel::Status::Failure);
        return;
    }
    // After this returns the delegate's OnBleConnectionComplete has run and either
    // recorded the endpoint + started BTP, or failed (cleanup happens in
    // OnBleConnectionError or the connect timeout).
}

static void OnConnectError(void * /*appState*/, CHIP_ERROR err)
{
    if (sPendingConnect == nullptr)
    {
        ChipLogProgress(AppServer,
                        "OnConnectError: ignoring stale callback "
                        "(err: %" CHIP_ERROR_FORMAT ")",
                        err.Format());
        return;
    }
    ChipLogError(AppServer, "ProxyConnectRequest: BLE scan/connect failed: %" CHIP_ERROR_FORMAT, err.Format());
    FailPendingConnect(chip::Protocols::InteractionModel::Status::Failure);
}

static void OnConnectTimeout(chip::System::Layer * /*layer*/, void * /*appState*/)
{
    if (sPendingConnect == nullptr)
    {
        return; // Already resolved.
    }

    ChipLogError(AppServer, "ProxyConnectRequest: timeout waiting for BLE connect");

    auto * ctx      = sPendingConnect;
    sPendingConnect = nullptr;

    // Stop the BleLayer's in-flight scan/connect, suppressing any later
    // OnConnectFound / OnConnectError callbacks.
    if (auto * layer = GetBleLayer())
    {
        CHIP_ERROR cancelErr = layer->CancelBleIncompleteConnection();
        if (cancelErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "OnConnectTimeout: CancelBleIncompleteConnection: %" CHIP_ERROR_FORMAT, cancelErr.Format());
        }
    }

    // If the endpoint was already created (BTP handshake in flight), close it.
    // Leave sBtpHandshakeEndpoint set: OnEndPointConnectionClosed checks it to
    // suppress forwarding the close to mOriginalTransport.  The callback clears it.
    if (ctx->endpoint != nullptr)
    {
        ctx->endpoint->Close();
    }

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
    {
        cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Timeout);
    }
    delete ctx;
    ResumeBleBgScanIfNeeded();
}

// Free-function callback fired by BLEManagerImpl::StartProxyScan once per
// discovered CHIP-bearing BLE peripheral.  We buffer results and emit a
// ProxyScanResponse when the scan timer expires.
static void OnScanResult(void * /*context*/, const uint8_t bdAddr[6], uint16_t discriminator, uint16_t vendorId,
                         uint16_t productId)
{
    // Fires on the Matter event loop (dispatched via ScheduleWork by ProxyScanForwarder),
    // so access to sScanResults (also touched by OnScanTimer) is serialized.
    // ProxyScanResponse spec rule (CommissioningProxy.adoc, ProxyScanResult field):
    // "Each found device SHALL be reported once based on discriminator/VendorID/
    // ProductID per transport."  Transport is fixed to kBle here so we dedupe on
    // the remaining three.
    for (const auto & existing : sScanResults)
    {
        if (existing.discriminator == discriminator && existing.vid == vendorId && existing.pid == productId)
        {
            return;
        }
    }
    BleScanResult r{};
    memcpy(r.mac, bdAddr, sizeof(r.mac));
    r.discriminator = discriminator;
    r.vid           = vendorId;
    r.pid           = productId;
    sScanResults.push_back(r);
    ChipLogProgress(AppServer, "Ble::Scan: discovered discriminator=%u vid=0x%04x pid=0x%04x", discriminator, vendorId, productId);
}

static void OnScanTimer(chip::System::Layer * /*layer*/, void * /*appState*/)
{
    (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
    sScanInProgress = false;

    std::vector<ScanResultT> out;
    out.reserve(sScanResults.size());
    for (const auto & d : sScanResults)
    {
        out.push_back(MakeScanResult(d.mac, d.discriminator, d.vid, d.pid));
    }

    // Hand the results to the cluster's scan aggregator; it owns the command
    // handle and emits the combined ProxyScanResponse once every transport's
    // sub-scan has reported.  Cap at MaxCachedResults before handing off so the
    // aggregator's numberOfResults field (uint8_t) cannot silently overflow.
    if (sHost != nullptr)
    {
        size_t cap = sHost->GetMaxCachedResults();
        if (out.size() > cap)
        {
            out.resize(cap);
        }
        sHost->ScanAggregator().Contribute(chip::Span<const ScanResultT>(out.data(), out.size()));
    }
    sScanResults.clear();

    // The foreground scan has released the BLE scanner; resume a background scan
    // that was paused to make room for it.
    ResumeBleBgScanIfNeeded();
}

} // namespace

// ==================================================================
// Internal entry points (invoked by the CommissioningProxyBleTransport methods).
// ==================================================================

void SetHost(CommissioningProxyCluster * host)
{
    sHost = host;
}

chip::Protocols::InteractionModel::Status Connect(chip::app::CommandHandler * commandObj,
                                                  const chip::app::DataModel::InvokeRequest & request, uint16_t discriminator,
                                                  uint16_t timeout)
{
    if (sPendingConnect != nullptr)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: a BLE connect is already in progress");
        return chip::Protocols::InteractionModel::Status::Busy;
    }

    auto * layer = GetBleLayer();
    if (layer == nullptr)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: BleLayer null");
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    // Sequential mode switch: the CP started in BLE peripheral mode so it could
    // be commissioned onto the fabric (Step 1).  The first BLE ProxyConnectRequest
    // transitions BLE to central role so we can scan / connect to the commissionee
    // (Step 2).  Idempotent — subsequent ProxyConnectRequest calls are no-ops.
    {
        CHIP_ERROR switchErr = chip::DeviceLayer::Internal::BLEMgrImpl().SwitchToCentralMode();
        if (switchErr == CHIP_ERROR_BUSY)
        {
            ChipLogError(AppServer, "ProxyConnectRequest: BLE busy with prior peripheral activity; retry shortly");
            return chip::Protocols::InteractionModel::Status::Busy;
        }
        if (switchErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyConnectRequest: SwitchToCentralMode failed: %" CHIP_ERROR_FORMAT, switchErr.Format());
            return chip::Protocols::InteractionModel::Status::Failure;
        }
    }

    // Install our BleLayerDelegate wrapper (idempotent).
    sProxyBleDelegate.Install();

    // The connect's NewBleConnectionByDiscriminator uses the single BLE scanner.
    // Pause any running background scan so it does not contend for the scanner;
    // it resumes once the session closes (OnAllSessionsClosed) or the connect
    // fails below.
    PauseBleBgScan();

    // Per spec a Timeout of 0 indicates no timeout: the connect runs until it
    // succeeds, fails, or is cancelled via ProxyDisconnectRequest(null).
    const bool hasTimeout = (timeout > 0);

    auto * ctx         = new ConnectCtx{};
    ctx->handle        = chip::app::CommandHandler::Handle(commandObj);
    ctx->path          = request.path;
    ctx->discriminator = discriminator;
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

    chip::SetupDiscriminator setupDisc;
    setupDisc.SetLongValue(discriminator);

    CHIP_ERROR err = layer->NewBleConnectionByDiscriminator(setupDisc, nullptr, OnConnectFound, OnConnectError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: NewBleConnectionByDiscriminator failed: %" CHIP_ERROR_FORMAT, err.Format());
        sPendingConnect = nullptr;
        delete ctx;
        ResumeBleBgScanIfNeeded();
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    if (hasTimeout)
    {
        CHIP_ERROR timerErr =
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(timeout), OnConnectTimeout, nullptr);
        if (timerErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyConnectRequest: StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            // Cancel the BLE scan we just started so we don't leak it.
            (void) layer->CancelBleIncompleteConnection();
            sPendingConnect = nullptr;
            delete ctx;
            ResumeBleBgScanIfNeeded();
            return chip::Protocols::InteractionModel::Status::Failure;
        }
    }

    ChipLogProgress(AppServer, "ProxyConnectRequest: BLE scan/connect started for discriminator %u", discriminator);
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
        ChipLogProgress(AppServer, "CancelPendingConnect: pending BLE connect owned by fabric %u, rejected fabric %u",
                        sPendingConnect->fabricIndex, fabricIndex);
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    auto * ctx      = sPendingConnect;
    sPendingConnect = nullptr;

    chip::DeviceLayer::SystemLayer().CancelTimer(OnConnectTimeout, nullptr);

    if (auto * layer = GetBleLayer())
    {
        CHIP_ERROR cancelErr = layer->CancelBleIncompleteConnection();
        if (cancelErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "CancelPendingConnect: CancelBleIncompleteConnection: %" CHIP_ERROR_FORMAT,
                          cancelErr.Format());
        }
    }

    // Leave sBtpHandshakeEndpoint set so OnEndPointConnectionClosed suppresses
    // forwarding the close to mOriginalTransport.  The callback clears it.
    if (ctx->endpoint != nullptr)
    {
        ctx->endpoint->Close();
    }

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
    {
        cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
    }
    delete ctx;

    ResumeBleBgScanIfNeeded();

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status Disconnect(uint16_t sessionId)
{
    auto it = sEndpoints.find(sessionId);
    if (it == sEndpoints.end())
    {
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    BLEEndPoint * ep = it->second;
    sEndpoints.erase(it);

    // Erase before Close so OnEndPointConnectionClosed (which fires synchronously)
    // finds no matching session via FindSessionId and does not call
    // DispatchMessageFailure for this local-initiated disconnect.  Set the BTP
    // sentinel so the close is not forwarded to mOriginalTransport.
    if (ep != nullptr)
    {
        sBtpHandshakeEndpoint = ep;
        ep->Close();
        // Close() fires synchronously; the callback clears sBtpHandshakeEndpoint.
        // Clear explicitly here in case Close() was a no-op (already-closed endpoint).
        sBtpHandshakeEndpoint = nullptr;
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

CHIP_ERROR SendMessage(uint16_t sessionId, chip::System::PacketBufferHandle && buf)
{
    auto it = sEndpoints.find(sessionId);
    if (it == sEndpoints.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    if (it->second == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return it->second->Send(std::move(buf));
}

chip::Protocols::InteractionModel::Status Scan(uint8_t scanMaxTime)
{
    if (sScanInProgress)
    {
        ChipLogProgress(AppServer, "Ble::Scan: scan already in progress — returning Busy");
        return chip::Protocols::InteractionModel::Status::Busy;
    }

    // The foreground scan and the background scan share the single BLE scanner.
    // Pause any running background scan so StartProxyScan below does not fail
    // with BUSY; OnScanTimer resumes it when the foreground scan completes.
    PauseBleBgScan();

    sScanResults.clear();
    CHIP_ERROR err = chip::DeviceLayer::Internal::BLEMgrImpl().StartProxyScan(OnScanResult, nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Ble::Scan: StartProxyScan failed: %" CHIP_ERROR_FORMAT, err.Format());
        ResumeBleBgScanIfNeeded();
        return chip::Protocols::InteractionModel::Status::Failure;
    }
    sScanInProgress = true;

    CHIP_ERROR timerErr =
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(scanMaxTime), OnScanTimer, nullptr);
    if (timerErr != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Ble::Scan: StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
        (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
        sScanInProgress = false;
        ResumeBleBgScanIfNeeded();
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    ChipLogProgress(AppServer, "Ble::Scan: started, scanMaxTime=%us", scanMaxTime);
    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status BgScanStart(uint16_t timeout, chip::BitMask<WiFiBandBitmap> /*wiFiBands*/,
                                                      chip::FabricIndex fabricIndex, chip::NodeId nodeId)
{
    // BLE has no Wi-Fi bands, so the WiFiBands field is meaningless and ignored.
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
        ChipLogProgress(AppServer, "BLE BgScan: updating timeout for existing fabric");
    }

    sBgScanFabrics[fabricKey] = FabricScanRecord{ nullptr };
    if (sBgScanCluster == nullptr)
    {
        sBgScanCluster = sHost;
    }

    if (wasEmpty || sBgScanPaused)
    {
        CHIP_ERROR err = StartBleBgHardwareScan();
        if (err == CHIP_ERROR_BUSY)
        {
            // The scanner is busy with a connect or foreground scan.  Keep the
            // fabric registered and defer; ResumeBleBgScanIfNeeded restarts the
            // hardware scan once the scanner frees up.
            sBgScanPaused = true;
            ChipLogProgress(AppServer, "BLE BgScan: scanner busy, deferring hardware scan (will resume when free)");
        }
        else if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyBackgroundScanStartRequest: StartProxyScan failed: %" CHIP_ERROR_FORMAT, err.Format());
            sBgScanFabrics.erase(fabricKey);
            if (sBgScanFabrics.empty())
            {
                sBgScanCluster = nullptr;
            }
            return chip::Protocols::InteractionModel::Status::Failure;
        }
        else
        {
            ChipLogProgress(AppServer, "ProxyBackgroundScanStartRequest: BLE hardware scan started");
        }
    }
    else
    {
        ChipLogProgress(AppServer,
                        "ProxyBackgroundScanStartRequest: BLE hardware scan already running, registering fabric (%zu total)",
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
            // Remove the fabric record we just added; without a lifetime timer the
            // BLE scanner would run indefinitely blocking all future background scans.
            sBgScanFabrics.erase(fabricKey);
            if (sBgScanFabrics.empty())
            {
                if (!sBgScanPaused)
                {
                    (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
                }
                sBgScanPaused  = false;
                sBgScanCluster = nullptr;
            }
            return chip::Protocols::InteractionModel::Status::Failure;
        }
        else
        {
            sBgScanFabrics[fabricKey].lifetimeCtx = lCtx;
        }
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status BgScanStop(chip::BitMask<CapabilitiesBitmap> transport,
                                                     chip::BitMask<WiFiBandBitmap> /*wiFiBands*/, chip::FabricIndex fabricIndex,
                                                     chip::NodeId nodeId)
{
    FabricKey fabricKey{ fabricIndex, nodeId };
    auto it = sBgScanFabrics.find(fabricKey);
    if (it == sBgScanFabrics.end())
    {
        ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: no matching BLE fabric record");
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    // BLE has no bands; only the kBle transport bit is meaningful.  A request that
    // does not name kBle (e.g. a band-only stop the cluster fans to every driver)
    // does not stop BLE scanning (spec: valid but not-requested transports/bands
    // return SUCCESS).
    if (!transport.Has(CapabilitiesBitmap::kBle))
    {
        ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: request does not target BLE, returning SUCCESS");
        return chip::Protocols::InteractionModel::Status::Success;
    }

    if (it->second.lifetimeCtx != nullptr)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(OnBgScanLifetimeExpiry, it->second.lifetimeCtx);
        delete it->second.lifetimeCtx;
        it->second.lifetimeCtx = nullptr;
    }
    sBgScanFabrics.erase(it);
    ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: BLE fabric stopped (%zu remaining)", sBgScanFabrics.size());

    if (sBgScanFabrics.empty())
    {
        // Only touch the scanner if the background scan actually owns it; if it
        // was paused, a connect or foreground scan owns the scanner and must not
        // be disturbed.
        if (!sBgScanPaused)
        {
            (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
        }
        sBgScanPaused = false;
        if (sBgScanCluster != nullptr)
        {
            sBgScanCluster->ScanCache().ClearTransport(CapabilitiesBitmap::kBle);
        }
        sBgScanCluster = nullptr;
        ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: last BLE fabric stopped, hardware scan stopped");
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

void OnAllSessionsClosed()
{
    // The proxy uses a one-way peripheral→central role switch
    // (BLEManagerImpl::SwitchToCentralMode) on the first ProxyConnectRequest(kBle),
    // so there is no peripheral advertising state to resume.  But a background
    // scan paused for the connect must be resumed now that the session has closed
    // and the scanner is free again.
    ResumeBleBgScanIfNeeded();
}

bool IsConnectPending()
{
    return sPendingConnect != nullptr;
}

void Shutdown()
{
    // Move the map out first so sEndpoints is empty when Close() fires
    // OnEndPointConnectionClosed synchronously — FindSessionId returns false and
    // the callbacks are no-ops rather than erasing the iterator mid-iteration.
    auto endpoints = std::move(sEndpoints);
    for (auto & [sid, ep] : endpoints)
    {
        if (ep != nullptr)
        {
            ep->Close();
        }
    }

    // If a ProxyConnectRequest was in flight, cancel its timeout and fail the
    // exchange.  Do not call FailPendingConnect() here — that would invoke
    // ResumeBleBgScanIfNeeded(), which would restart the HW scan immediately
    // before the cleanup below stops and clears it.
    if (sPendingConnect != nullptr)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(OnConnectTimeout, nullptr);
        auto * ctx                      = sPendingConnect;
        sPendingConnect                 = nullptr;
        chip::app::CommandHandler * cmd = ctx->handle.Get();
        if (cmd != nullptr)
        {
            cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
        }
        delete ctx;
    }
    sBtpHandshakeEndpoint = nullptr;

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

    // Stop the hardware scan only if the background scan currently owns it.
    if (!sBgScanFabrics.empty() && !sBgScanPaused)
    {
        (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
    }

    sBgScanFabrics.clear();
    sBgScanPaused  = false;
    sBgScanCluster = nullptr;
}

} // namespace Ble

// ==================================================================
// CommissioningProxyTransport implementation (thin adapter over the Ble:: singleton).
// ==================================================================

void CommissioningProxyBleTransport::SetHost(CommissioningProxyCluster * host)
{
    Ble::SetHost(host);
}

Protocols::InteractionModel::Status CommissioningProxyBleTransport::Connect(app::CommandHandler * commandObj,
                                                                            const DataModel::InvokeRequest & request,
                                                                            uint16_t discriminator, uint16_t timeout)
{
    return Ble::Connect(commandObj, request, discriminator, timeout);
}

Protocols::InteractionModel::Status CommissioningProxyBleTransport::CancelPendingConnect(FabricIndex fabricIndex)
{
    return Ble::CancelPendingConnect(fabricIndex);
}

Protocols::InteractionModel::Status CommissioningProxyBleTransport::Disconnect(uint16_t sessionId)
{
    return Ble::Disconnect(sessionId);
}

CHIP_ERROR CommissioningProxyBleTransport::SendMessage(uint16_t sessionId, System::PacketBufferHandle && buf)
{
    return Ble::SendMessage(sessionId, std::move(buf));
}

Protocols::InteractionModel::Status CommissioningProxyBleTransport::Scan(uint8_t scanMaxTime)
{
    return Ble::Scan(scanMaxTime);
}

Protocols::InteractionModel::Status CommissioningProxyBleTransport::BgScanStart(uint16_t timeout, BitMask<WiFiBandBitmap> wiFiBands,
                                                                                FabricIndex fabricIndex, NodeId nodeId)
{
    return Ble::BgScanStart(timeout, wiFiBands, fabricIndex, nodeId);
}

Protocols::InteractionModel::Status CommissioningProxyBleTransport::BgScanStop(BitMask<CapabilitiesBitmap> transport,
                                                                               BitMask<WiFiBandBitmap> wiFiBands,
                                                                               FabricIndex fabricIndex, NodeId nodeId)
{
    return Ble::BgScanStop(transport, wiFiBands, fabricIndex, nodeId);
}

void CommissioningProxyBleTransport::OnAllSessionsClosed()
{
    Ble::OnAllSessionsClosed();
}

bool CommissioningProxyBleTransport::IsConnectPending() const
{
    return Ble::IsConnectPending();
}

void CommissioningProxyBleTransport::Shutdown()
{
    Ble::Shutdown();
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
