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

#include "CommissioningProxyBgScanCache.h"
#include "CommissioningProxyDispatcher.h"

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
static std::map<uint16_t, BLEEndPoint *> sBleEndpoints;

// Context kept alive for the duration of an asynchronous ProxyConnectRequest.
struct BleConnectCtx
{
    chip::app::CommandHandler::Handle handle;
    chip::app::ConcreteCommandPath path;
    uint16_t discriminator;
    chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * cluster = nullptr;
    chip::FabricIndex fabricIndex                                                = chip::kUndefinedFabricIndex;
    BLEEndPoint * endpoint                                                       = nullptr; // set when L2CAP wrap completes
};

// Single in-flight proxy connect (MaxSessions=1 shared across transports).
static BleConnectCtx * sBlePendingConnect = nullptr;

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

static std::vector<BleScanResult> sBleScanResults;
static bool sScanInProgress = false;

static BleLayer * GetBleLayer()
{
    return chip::DeviceLayer::ConnectivityMgr().GetBleLayer();
}

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

using ScanResultT = chip::app::Clusters::CommissioningProxy::Structs::ScanResultStruct::Type;

// Build a kBle ScanResultStruct from a discovered device.  Shared by the
// foreground scan and the background-scan discovery callback so the field set
// stays identical across both paths.
static ScanResultT MakeBleScanResult(const uint8_t bdAddr[6], uint16_t discriminator, uint16_t vendorId, uint16_t productId)
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

struct BgFabricKey
{
    chip::FabricIndex fabricIndex;
    chip::NodeId nodeId;
    bool operator<(const BgFabricKey & o) const
    {
        if (fabricIndex != o.fabricIndex)
            return fabricIndex < o.fabricIndex;
        return nodeId < o.nodeId;
    }
};

struct BgFabricLifetimeCtx
{
    BgFabricKey key;
};

struct BleBgFabricRecord
{
    BgFabricLifetimeCtx * lifetimeCtx = nullptr;
};

static std::map<BgFabricKey, BleBgFabricRecord> sBleBgFabrics;
// True when fabrics are registered but the hardware scan is suspended because a
// connect or foreground scan currently owns the single BLE scanner.
static bool sBleBgPaused                                                                  = false;
static chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster * sBleBgCluster = nullptr;

// Result cache, TTL, and combined MaxCachedResults cap live in the shared
// BgScanCache module; this callback just builds the kBle result and reports it.
// Runs with the CHIP stack lock held (the platform ProxyScanForwarder locks
// before invoking scan-result callbacks).
static void OnBleBgDiscovery(void * /*context*/, const uint8_t bdAddr[6], uint16_t discriminator, uint16_t vendorId,
                             uint16_t productId)
{
    BgScanCache::Report(MakeBleScanResult(bdAddr, discriminator, vendorId, productId), sBleBgCluster);
}

static void OnBleBgLifetimeExpiry(chip::System::Layer * /*layer*/, void * appState)
{
    auto * ctx      = static_cast<BgFabricLifetimeCtx *>(appState);
    BgFabricKey key = ctx->key;
    delete ctx;

    auto it = sBleBgFabrics.find(key);
    if (it == sBleBgFabrics.end())
        return;
    it->second.lifetimeCtx = nullptr;
    sBleBgFabrics.erase(it);

    ChipLogProgress(AppServer, "BLE BgScan: lifetime expired for fabricIndex=%u nodeId=0x" ChipLogFormatX64, key.fabricIndex,
                    ChipLogValueX64(key.nodeId));

    if (sBleBgFabrics.empty())
    {
        if (!sBleBgPaused)
            (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
        sBleBgPaused = false;
        BgScanCache::ClearTransport(CapabilitiesBitmap::kBle, sBleBgCluster);
        sBleBgCluster = nullptr;
    }
}

// Start (or resume) the hardware background scan if it is not already running.
// Returns CHIP_ERROR_BUSY if the BLE scanner is currently held by a connect or
// foreground scan — the caller keeps the fabric registered and the scan resumes
// later via ResumeBleBgScanIfNeeded.
static CHIP_ERROR StartBleBgHardwareScan()
{
    CHIP_ERROR err = chip::DeviceLayer::Internal::BLEMgrImpl().StartProxyScan(OnBleBgDiscovery, nullptr);
    if (err == CHIP_NO_ERROR)
        sBleBgPaused = false;
    return err;
}

// Suspend the hardware background scan so a connect or foreground scan can use
// the single BLE scanner.  Idempotent; no-op when no fabrics are registered.
static void PauseBleBgScan()
{
    if (!sBleBgFabrics.empty() && !sBleBgPaused)
    {
        (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
        sBleBgPaused = true;
        ChipLogProgress(AppServer, "BLE BgScan: paused (scanner needed for connect/foreground scan)");
    }
}

// Resume a previously paused hardware background scan when the scanner frees up.
static void ResumeBleBgScanIfNeeded()
{
    if (sBleBgPaused && !sBleBgFabrics.empty())
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

// Look up the sessionId for an endpoint already promoted to sBleEndpoints.
// Returns true if found; writes the id to outSessionId.
static bool FindSessionId(BLEEndPoint * ep, uint16_t & outSessionId)
{
    for (auto & [sid, e] : sBleEndpoints)
    {
        if (e == ep)
        {
            outSessionId = sid;
            return true;
        }
    }
    return false;
}

// Forward-declarations for the IM-thread connect callbacks (defined below the delegate).
static void OnBleConnect_Found(void * appState, BLE_CONNECTION_OBJECT connObj);
static void OnBleConnect_Error(void * appState, CHIP_ERROR err);
static void OnBleConnect_Timeout(chip::System::Layer * layer, void * appState);

// ------------------------------------------------------------------
// BleLayerDelegate wrapper.
//
// Wraps the original BleLayer::mBleTransport (set by transport/raw/BLE.cpp's
// BLEBase) so that BLE central events for proxy-owned endpoints are routed
// back to the commissioner via the dispatcher; all other events fall through
// to the original delegate (e.g. the proxy app's own peripheral commissioning).
// ------------------------------------------------------------------
class BleProxyDelegate : public BleLayerDelegate
{
public:
    void Install()
    {
        auto * layer = GetBleLayer();
        if (layer == nullptr)
        {
            ChipLogError(AppServer, "BleProxyDelegate::Install: no BleLayer");
            return;
        }
        if (layer->mBleTransport == this)
            return;
        mOriginalTransport   = layer->mBleTransport;
        layer->mBleTransport = this;
        ChipLogProgress(AppServer, "BleProxyDelegate: installed (original=%p)", (void *) mOriginalTransport);
    }

    void OnBleConnectionComplete(BLEEndPoint * endpoint) override
    {
        // Central role: BleLayer just wrapped a new L2CAP connection in an
        // endpoint.  If we have a pending proxy connect with no endpoint yet,
        // this one is ours.  Initiate the BTP handshake on it.
        if (sBlePendingConnect != nullptr && sBlePendingConnect->endpoint == nullptr)
        {
            sBlePendingConnect->endpoint = endpoint;
            CHIP_ERROR err               = endpoint->StartConnect();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "BleProxyDelegate: StartConnect failed: %" CHIP_ERROR_FORMAT, err.Format());
                // The endpoint's own DoClose path will fire OnEndPointConnectComplete
                // with the error; that path completes the IM response below.
            }
            return;
        }
        if (mOriginalTransport != nullptr)
            mOriginalTransport->OnBleConnectionComplete(endpoint);
    }

    void OnBleConnectionError(CHIP_ERROR err) override
    {
        // Central role: pre-BTP-handshake error (scan failure, L2CAP rejected, etc.).
        if (sBlePendingConnect != nullptr && sBlePendingConnect->endpoint == nullptr)
        {
            ChipLogError(AppServer, "BleProxyDelegate: OnBleConnectionError: %" CHIP_ERROR_FORMAT, err.Format());
            chip::DeviceLayer::SystemLayer().CancelTimer(OnBleConnect_Timeout, nullptr);
            auto * ctx                      = sBlePendingConnect;
            sBlePendingConnect              = nullptr;
            chip::app::CommandHandler * cmd = ctx->handle.Get();
            if (cmd != nullptr)
                cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
            delete ctx;
            return;
        }
        if (mOriginalTransport != nullptr)
            mOriginalTransport->OnBleConnectionError(err);
    }

    void OnEndPointConnectComplete(BLEEndPoint * endpoint, CHIP_ERROR err) override
    {
        // BTP handshake completed (or failed).  If this endpoint is ours,
        // complete the ProxyConnectRequest exchange.
        if (sBlePendingConnect != nullptr && sBlePendingConnect->endpoint == endpoint)
        {
            chip::DeviceLayer::SystemLayer().CancelTimer(OnBleConnect_Timeout, nullptr);
            auto * ctx                      = sBlePendingConnect;
            sBlePendingConnect              = nullptr;
            chip::app::CommandHandler * cmd = ctx->handle.Get();

            if (err != CHIP_NO_ERROR || cmd == nullptr)
            {
                ChipLogError(AppServer, "BleProxyDelegate: OnEndPointConnectComplete err=%" CHIP_ERROR_FORMAT " cmd=%p",
                             err.Format(), (void *) cmd);
                if (cmd != nullptr)
                    cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
                if (endpoint != nullptr)
                    endpoint->Close();
                delete ctx;
                return;
            }

            uint16_t sessionId       = ProxyDispatcher::AllocSessionId();
            sBleEndpoints[sessionId] = endpoint;
            ProxyDispatcher::RegisterSession(sessionId, CapabilitiesBitmap::kBle, ctx->fabricIndex);

            ChipLogProgress(AppServer, "ProxyConnectRequest: BLE connected, proxy session %u (disc %u)", sessionId,
                            ctx->discriminator);

            if (ctx->cluster != nullptr)
            {
                CHIP_ERROR stateErr = ctx->cluster->SetCPState(
                    chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster::kState_CPConnected);
                if (stateErr != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "BleProxyDelegate: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
                }
            }

            chip::app::Clusters::CommissioningProxy::Commands::ProxyConnectResponse::Type response;
            response.sessionID = sessionId;
            cmd->AddResponse(ctx->path, response);
            delete ctx;
            return;
        }
        if (mOriginalTransport != nullptr)
            mOriginalTransport->OnEndPointConnectComplete(endpoint, err);
    }

    void OnEndPointMessageReceived(BLEEndPoint * endpoint, chip::System::PacketBufferHandle && msg) override
    {
        uint16_t sid;
        if (FindSessionId(endpoint, sid))
        {
            // DispatchMessageResponse copies the payload into the IM ProxyMessageResponse
            // synchronously, so msg can be released when this scope returns.
            ProxyDispatcher::DispatchMessageResponse(sid, msg->Start(), msg->DataLength());
            return;
        }
        if (mOriginalTransport != nullptr)
            mOriginalTransport->OnEndPointMessageReceived(endpoint, std::move(msg));
    }

    void OnEndPointConnectionClosed(BLEEndPoint * endpoint, CHIP_ERROR err) override
    {
        uint16_t sid;
        if (FindSessionId(endpoint, sid))
        {
            ChipLogProgress(AppServer, "BleProxyDelegate: session %u closed: %" CHIP_ERROR_FORMAT, sid, err.Format());
            // Fail any in-flight ProxyMessageRequest for this session.
            ProxyDispatcher::DispatchMessageFailure(sid, chip::Protocols::InteractionModel::Status::Failure);
            sBleEndpoints.erase(sid);
            ProxyDispatcher::RemoveSession(sid);
            return;
        }
        if (mOriginalTransport != nullptr)
            mOriginalTransport->OnEndPointConnectionClosed(endpoint, err);
    }

    CHIP_ERROR SetEndPoint(BLEEndPoint * endpoint) override
    {
        if (mOriginalTransport != nullptr)
            return mOriginalTransport->SetEndPoint(endpoint);
        return CHIP_NO_ERROR;
    }

private:
    BleLayerDelegate * mOriginalTransport = nullptr;
};

static BleProxyDelegate sBleProxyDelegate;

// ------------------------------------------------------------------
// BleConnectionDelegate callbacks: fire from the platform when the L2CAP scan
// + connect step completes (still before BTP handshake).  These run inside
// the Matter event loop.
// ------------------------------------------------------------------

// Tear down a pending connect synchronously and complete the IM exchange with
// the given status.  Caller has already verified sBlePendingConnect != nullptr.
static void FailPendingConnect(chip::Protocols::InteractionModel::Status status)
{
    chip::DeviceLayer::SystemLayer().CancelTimer(OnBleConnect_Timeout, nullptr);
    auto * ctx                      = sBlePendingConnect;
    sBlePendingConnect              = nullptr;
    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
        cmd->AddStatus(ctx->path, status);
    delete ctx;
    ResumeBleBgScanIfNeeded();
}

static void OnBleConnect_Found(void * /*appState*/, BLE_CONNECTION_OBJECT connObj)
{
    // We pass nullptr for appState to NewBleConnectionByDiscriminator and rely on
    // sBlePendingConnect being non-null (MaxSessions=1 guarantees uniqueness).
    if (sBlePendingConnect == nullptr)
    {
        ChipLogProgress(AppServer, "OnBleConnect_Found: no pending connect ctx; ignoring stale callback");
        return;
    }

    // Wrap the raw L2CAP connection into a BLEEndPoint (Central role, autoClose=true)
    // via the no-callback NewBleConnectionByObject path so OnConnectionComplete in
    // BleLayer creates the endpoint and dispatches to our BleLayerDelegate (which
    // is sBleProxyDelegate).
    auto * layer = GetBleLayer();
    if (layer == nullptr)
    {
        ChipLogError(AppServer, "OnBleConnect_Found: BleLayer null");
        FailPendingConnect(chip::Protocols::InteractionModel::Status::Failure);
        return;
    }
    CHIP_ERROR err = layer->NewBleConnectionByObject(connObj);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "OnBleConnect_Found: NewBleConnectionByObject failed: %" CHIP_ERROR_FORMAT, err.Format());
        // No further callback will fire — clean up synchronously rather than
        // waiting for the connect timer to expire.
        FailPendingConnect(chip::Protocols::InteractionModel::Status::Failure);
        return;
    }
    // After this returns the delegate's OnBleConnectionComplete has run and either
    // recorded the endpoint + started BTP, or failed (cleanup happens in
    // OnBleConnectionError or the connect timeout).
}

static void OnBleConnect_Error(void * /*appState*/, CHIP_ERROR err)
{
    if (sBlePendingConnect == nullptr)
    {
        ChipLogProgress(AppServer,
                        "OnBleConnect_Error: ignoring stale callback "
                        "(err: %" CHIP_ERROR_FORMAT ")",
                        err.Format());
        return;
    }
    ChipLogError(AppServer, "ProxyConnectRequest: BLE scan/connect failed: %" CHIP_ERROR_FORMAT, err.Format());
    chip::DeviceLayer::SystemLayer().CancelTimer(OnBleConnect_Timeout, nullptr);
    auto * ctx                      = sBlePendingConnect;
    sBlePendingConnect              = nullptr;
    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
        cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
    delete ctx;
    ResumeBleBgScanIfNeeded();
}

static void OnBleConnect_Timeout(chip::System::Layer * /*layer*/, void * /*appState*/)
{
    if (sBlePendingConnect == nullptr)
        return; // Already resolved.

    ChipLogError(AppServer, "ProxyConnectRequest: timeout waiting for BLE connect");

    auto * ctx         = sBlePendingConnect;
    sBlePendingConnect = nullptr;

    // Stop the BleLayer's in-flight scan/connect, suppressing any later
    // OnBleConnect_Found / OnBleConnect_Error callbacks.
    if (auto * layer = GetBleLayer())
    {
        CHIP_ERROR cancelErr = layer->CancelBleIncompleteConnection();
        if (cancelErr != CHIP_NO_ERROR)
            ChipLogDetail(AppServer, "OnBleConnect_Timeout: CancelBleIncompleteConnection: %" CHIP_ERROR_FORMAT,
                          cancelErr.Format());
    }

    // If the endpoint was already created (BTP handshake in flight), close it.
    if (ctx->endpoint != nullptr)
    {
        ctx->endpoint->Close();
    }

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
        cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Timeout);
    delete ctx;
    ResumeBleBgScanIfNeeded();
}

// Free-function callback fired by BLEManagerImpl::StartProxyScan once per
// discovered CHIP-bearing BLE peripheral.  We buffer results and emit a
// ProxyScanResponse when the scan timer expires.
static void OnBleScanResult(void * /*context*/, const uint8_t bdAddr[6], uint16_t discriminator, uint16_t vendorId,
                            uint16_t productId)
{
    // Fires on the BlueZ/GLib thread; the platform ProxyScanForwarder holds the
    // CHIP stack lock around this call, so access to sBleScanResults (also touched
    // by OnBleScanTimer on the Matter thread) is serialized.
    // ProxyScanResponse spec rule (CommissioningProxy.adoc, ProxyScanResult field):
    // "Each found device SHALL be reported once based on discriminator/VendorID/
    // ProductID per transport."  Transport is fixed to kBle here so we dedupe on
    // the remaining three.
    for (const auto & existing : sBleScanResults)
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
    sBleScanResults.push_back(r);
    ChipLogProgress(AppServer, "Ble::Scan: discovered discriminator=%u vid=0x%04x pid=0x%04x", discriminator, vendorId, productId);
}

static void OnBleScanTimer(chip::System::Layer * /*layer*/, void * /*appState*/)
{
    (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
    sScanInProgress = false;

    std::vector<ScanResultT> out;
    out.reserve(sBleScanResults.size());
    for (const auto & d : sBleScanResults)
    {
        out.push_back(MakeBleScanResult(d.mac, d.discriminator, d.vid, d.pid));
    }

    // Hand the results to the dispatcher's aggregator; it owns the command
    // handle and emits the combined ProxyScanResponse once every transport's
    // sub-scan has reported.
    ProxyDispatcher::ContributeScanResults(chip::Span<const ScanResultT>(out.data(), out.size()));
    sBleScanResults.clear();

    // The foreground scan has released the BLE scanner; resume a background scan
    // that was paused to make room for it.
    ResumeBleBgScanIfNeeded();
}

} // namespace

// ==================================================================
// Public entry points (called by the dispatcher)
// ==================================================================

chip::Protocols::InteractionModel::Status Connect(chip::app::CommandHandler * commandObj,
                                                  const chip::app::DataModel::InvokeRequest & request, uint16_t discriminator,
                                                  uint16_t timeout, CommissioningProxyCluster * cluster)
{
    if (sBlePendingConnect != nullptr)
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
    sBleProxyDelegate.Install();

    // The connect's NewBleConnectionByDiscriminator uses the single BLE scanner.
    // Pause any running background scan so it does not contend for the scanner;
    // it resumes once the session closes (OnAllSessionsClosed) or the connect
    // fails below.
    PauseBleBgScan();

    // Per spec a Timeout of 0 indicates no timeout: the connect runs until it
    // succeeds, fails, or is cancelled via ProxyDisconnectRequest(null).
    const bool hasTimeout = (timeout > 0);

    auto * ctx         = new BleConnectCtx{};
    ctx->handle        = chip::app::CommandHandler::Handle(commandObj);
    ctx->path          = request.path;
    ctx->discriminator = discriminator;
    ctx->cluster       = cluster;
    ctx->fabricIndex   = request.subjectDescriptor.fabricIndex;
    sBlePendingConnect = ctx;
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

    CHIP_ERROR err = layer->NewBleConnectionByDiscriminator(setupDisc, nullptr, OnBleConnect_Found, OnBleConnect_Error);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: NewBleConnectionByDiscriminator failed: %" CHIP_ERROR_FORMAT, err.Format());
        sBlePendingConnect = nullptr;
        delete ctx;
        ResumeBleBgScanIfNeeded();
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    if (hasTimeout)
    {
        CHIP_ERROR timerErr =
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(timeout), OnBleConnect_Timeout, nullptr);
        if (timerErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyConnectRequest: StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            // Cancel the BLE scan we just started so we don't leak it.
            (void) layer->CancelBleIncompleteConnection();
            sBlePendingConnect = nullptr;
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
    if (sBlePendingConnect == nullptr)
        return chip::Protocols::InteractionModel::Status::InvalidInState;

    if (fabricIndex != sBlePendingConnect->fabricIndex)
    {
        ChipLogProgress(AppServer, "CancelPendingConnect: pending BLE connect owned by fabric %u, rejected fabric %u",
                        sBlePendingConnect->fabricIndex, fabricIndex);
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    auto * ctx         = sBlePendingConnect;
    sBlePendingConnect = nullptr;

    chip::DeviceLayer::SystemLayer().CancelTimer(OnBleConnect_Timeout, nullptr);

    if (auto * layer = GetBleLayer())
    {
        CHIP_ERROR cancelErr = layer->CancelBleIncompleteConnection();
        if (cancelErr != CHIP_NO_ERROR)
            ChipLogDetail(AppServer, "CancelPendingConnect: CancelBleIncompleteConnection: %" CHIP_ERROR_FORMAT,
                          cancelErr.Format());
    }

    if (ctx->endpoint != nullptr)
    {
        ctx->endpoint->Close();
    }

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
        cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
    delete ctx;

    ResumeBleBgScanIfNeeded();

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status Disconnect(uint16_t sessionId)
{
    auto it = sBleEndpoints.find(sessionId);
    if (it == sBleEndpoints.end())
        return chip::Protocols::InteractionModel::Status::NotFound;

    BLEEndPoint * ep = it->second;
    sBleEndpoints.erase(it);

    // Gracefully close the endpoint.  OnEndPointConnectionClosed will fire later
    // but will be a no-op for this session because we've already erased it from
    // sBleEndpoints; the dispatcher's RemoveSession is the source of truth.
    if (ep != nullptr)
    {
        ep->Close();
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

CHIP_ERROR SendMessage(uint16_t sessionId, chip::System::PacketBufferHandle && buf)
{
    auto it = sBleEndpoints.find(sessionId);
    if (it == sBleEndpoints.end())
        return CHIP_ERROR_KEY_NOT_FOUND;
    if (it->second == nullptr)
        return CHIP_ERROR_INCORRECT_STATE;
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
    // with BUSY; OnBleScanTimer resumes it when the foreground scan completes.
    PauseBleBgScan();

    sBleScanResults.clear();
    CHIP_ERROR err = chip::DeviceLayer::Internal::BLEMgrImpl().StartProxyScan(OnBleScanResult, nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Ble::Scan: StartProxyScan failed: %" CHIP_ERROR_FORMAT, err.Format());
        ResumeBleBgScanIfNeeded();
        return chip::Protocols::InteractionModel::Status::Failure;
    }
    sScanInProgress = true;

    CHIP_ERROR timerErr =
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(scanMaxTime), OnBleScanTimer, nullptr);
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

chip::Protocols::InteractionModel::Status BgScanStart(CapabilitiesBitmap /*transport*/, uint16_t timeout,
                                                      WiFiBandBitmap /*wiFiBands*/, chip::FabricIndex fabricIndex,
                                                      chip::NodeId nodeId, CommissioningProxyCluster * cluster)
{
    // The dispatcher passes only kBle here; BLE has no Wi-Fi bands, so the
    // WiFiBands field is meaningless and ignored.
    BgFabricKey fabricKey{ fabricIndex, nodeId };
    bool wasEmpty      = sBleBgFabrics.empty();
    bool alreadyExists = (sBleBgFabrics.count(fabricKey) > 0);

    if (alreadyExists)
    {
        auto & rec = sBleBgFabrics[fabricKey];
        if (rec.lifetimeCtx != nullptr)
        {
            chip::DeviceLayer::SystemLayer().CancelTimer(OnBleBgLifetimeExpiry, rec.lifetimeCtx);
            delete rec.lifetimeCtx;
            rec.lifetimeCtx = nullptr;
        }
        ChipLogProgress(AppServer, "BLE BgScan: updating timeout for existing fabric");
    }

    sBleBgFabrics[fabricKey] = BleBgFabricRecord{ nullptr };
    if (sBleBgCluster == nullptr)
        sBleBgCluster = cluster;

    if (wasEmpty || sBleBgPaused)
    {
        CHIP_ERROR err = StartBleBgHardwareScan();
        if (err == CHIP_ERROR_BUSY)
        {
            // The scanner is busy with a connect or foreground scan.  Keep the
            // fabric registered and defer; ResumeBleBgScanIfNeeded restarts the
            // hardware scan once the scanner frees up.
            sBleBgPaused = true;
            ChipLogProgress(AppServer, "BLE BgScan: scanner busy, deferring hardware scan (will resume when free)");
        }
        else if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyBackgroundScanStartRequest: StartProxyScan failed: %" CHIP_ERROR_FORMAT, err.Format());
            sBleBgFabrics.erase(fabricKey);
            if (sBleBgFabrics.empty())
                sBleBgCluster = nullptr;
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
                        sBleBgFabrics.size());
    }

    if (timeout > 0)
    {
        auto * lCtx = new BgFabricLifetimeCtx{ fabricKey };
        CHIP_ERROR timerErr =
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(timeout), OnBleBgLifetimeExpiry, lCtx);
        if (timerErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyBackgroundScanStartRequest: StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            delete lCtx;
        }
        else
        {
            sBleBgFabrics[fabricKey].lifetimeCtx = lCtx;
        }
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status BgScanStop(CapabilitiesBitmap transport, WiFiBandBitmap /*wiFiBands*/,
                                                     chip::FabricIndex fabricIndex, chip::NodeId nodeId)
{
    BgFabricKey fabricKey{ fabricIndex, nodeId };
    auto it = sBleBgFabrics.find(fabricKey);
    if (it == sBleBgFabrics.end())
    {
        ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: no matching BLE fabric record");
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    // BLE has no bands; only the kBle transport bit is meaningful.  A request
    // that does not name kBle does not stop BLE scanning (spec: valid but not
    // originally requested transports/bands return SUCCESS).
    uint8_t stopBle = static_cast<uint8_t>(transport) & static_cast<uint8_t>(CapabilitiesBitmap::kBle);
    if (stopBle == 0)
    {
        ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: request does not target BLE, returning SUCCESS");
        return chip::Protocols::InteractionModel::Status::Success;
    }

    if (it->second.lifetimeCtx != nullptr)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(OnBleBgLifetimeExpiry, it->second.lifetimeCtx);
        delete it->second.lifetimeCtx;
        it->second.lifetimeCtx = nullptr;
    }
    sBleBgFabrics.erase(it);
    ChipLogProgress(AppServer, "ProxyBackgroundScanStopRequest: BLE fabric stopped (%zu remaining)", sBleBgFabrics.size());

    if (sBleBgFabrics.empty())
    {
        // Only touch the scanner if the background scan actually owns it; if it
        // was paused, a connect or foreground scan owns the scanner and must not
        // be disturbed.
        if (!sBleBgPaused)
            (void) chip::DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
        sBleBgPaused = false;
        BgScanCache::ClearTransport(CapabilitiesBitmap::kBle, sBleBgCluster);
        sBleBgCluster = nullptr;
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
    return sBlePendingConnect != nullptr;
}

} // namespace Ble
} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
