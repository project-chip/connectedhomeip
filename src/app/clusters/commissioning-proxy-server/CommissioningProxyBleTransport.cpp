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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleTransport.h>

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <lib/support/SetupDiscriminator.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>

#include <algorithm>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

using chip::Ble::BLEEndPoint;
using chip::Ble::BleLayer;
using Protocols::InteractionModel::Status;

CommissioningProxyBleTransport::CommissioningProxyBleTransport(CommissioningProxyBleAdapter & adapter,
                                                               TimerDelegate & timerDelegate) :
    mAdapter(adapter),
    mTimerDelegate(timerDelegate), mBgScan(mBgScanHardware, timerDelegate)
{}

CommissioningProxyBleTransport::~CommissioningProxyBleTransport()
{
    // Idempotent, and required if the owner never called it: the timers and the
    // installed BleLayer delegate would otherwise outlive this object.
    Shutdown();
}

// ==================================================================
// Small helpers
// ==================================================================

BleLayer * CommissioningProxyBleTransport::GetBleLayer() const
{
    return DeviceLayer::ConnectivityMgr().GetBleLayer();
}

CommissioningProxyBleTransport::EndpointSlot * CommissioningProxyBleTransport::FindSlot(uint16_t sessionId)
{
    for (auto & slot : mEndpoints)
    {
        if (slot.inUse && slot.sessionId == sessionId)
        {
            return &slot;
        }
    }
    return nullptr;
}

CommissioningProxyBleTransport::EndpointSlot * CommissioningProxyBleTransport::FindSlot(const BLEEndPoint * endpoint)
{
    if (endpoint == nullptr)
    {
        return nullptr;
    }
    for (auto & slot : mEndpoints)
    {
        if (slot.inUse && slot.endpoint == endpoint)
        {
            return &slot;
        }
    }
    return nullptr;
}

CommissioningProxyBleTransport::EndpointSlot * CommissioningProxyBleTransport::ClaimSlot()
{
    for (auto & slot : mEndpoints)
    {
        if (!slot.inUse)
        {
            return &slot;
        }
    }
    return nullptr;
}

bool CommissioningProxyBleTransport::AnySessionOpen() const
{
    for (const auto & slot : mEndpoints)
    {
        if (slot.inUse)
        {
            return true;
        }
    }
    return false;
}

CommissioningProxyBleTransport::ScanResultT CommissioningProxyBleTransport::MakeScanResult(ByteSpan address, uint16_t discriminator,
                                                                                           uint16_t vendorId, uint16_t productId)
{
    ScanResultT r{};
    r.address.SetNonNull(address);
    r.transport     = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle);
    r.discriminator = discriminator;
    r.vendorID      = static_cast<VendorId>(vendorId);
    r.productID     = productId;
    // A BLE advertisement carries neither of these.
    r.extendedData.SetNull();
    r.wiFiBand.ClearValue();
    return r;
}

void CommissioningProxyBleTransport::FailPendingConnect(Status status)
{
    // Caller has already verified a connect is pending.
    mTimerDelegate.CancelTimer(&mConnectTimer);

    // Move out and reset before answering, so a re-entrant callback sees no pending
    // connect.
    ConnectCtx ctx = std::move(*mPendingConnect);
    mPendingConnect.reset();

    if (app::CommandHandler * cmd = ctx.handle.Get())
    {
        cmd->AddStatus(ctx.path, status);
    }
    ResumeBgScanIfNeeded();
}

// ==================================================================
// ProxyBleDelegate
// ==================================================================

void CommissioningProxyBleTransport::ProxyBleDelegate::Install()
{
    auto * layer = mOwner.GetBleLayer();
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

void CommissioningProxyBleTransport::ProxyBleDelegate::Uninstall()
{
    auto * layer = mOwner.GetBleLayer();
    if (layer == nullptr)
    {
        return;
    }
    if (layer->mBleTransport == this)
    {
        layer->mBleTransport = mOriginalTransport;
        ChipLogProgress(AppServer, "ProxyBleDelegate: uninstalled");
    }
    mOriginalTransport = nullptr;
}

void CommissioningProxyBleTransport::ProxyBleDelegate::OnBleConnectionComplete(BLEEndPoint * endpoint)
{
    // Central role: BleLayer just wrapped a new L2CAP connection in an endpoint. If we
    // have a pending proxy connect with no endpoint yet, this one is ours. Initiate the
    // BTP handshake on it.
    if (mOwner.mPendingConnect.has_value() && mOwner.mPendingConnect->endpoint == nullptr)
    {
        mOwner.mPendingConnect->endpoint = endpoint;
        mOwner.mBtpHandshakeEndpoint     = endpoint;
        CHIP_ERROR err                   = endpoint->StartConnect();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyBleDelegate: StartConnect failed: %" CHIP_ERROR_FORMAT, err.Format());
            // The endpoint's own DoClose path will fire OnEndPointConnectComplete with
            // the error; that path completes the IM response below.
        }
        return;
    }
    if (mOriginalTransport != nullptr)
    {
        mOriginalTransport->OnBleConnectionComplete(endpoint);
    }
}

void CommissioningProxyBleTransport::ProxyBleDelegate::OnBleConnectionError(CHIP_ERROR err)
{
    // Central role: pre-BTP-handshake error (scan failure, L2CAP rejected, etc.).
    if (mOwner.mPendingConnect.has_value() && mOwner.mPendingConnect->endpoint == nullptr)
    {
        ChipLogError(AppServer, "ProxyBleDelegate: OnBleConnectionError: %" CHIP_ERROR_FORMAT, err.Format());
        mOwner.FailPendingConnect(Status::Failure);
        return;
    }
    if (mOriginalTransport != nullptr)
    {
        mOriginalTransport->OnBleConnectionError(err);
    }
}

void CommissioningProxyBleTransport::ProxyBleDelegate::OnEndPointConnectComplete(BLEEndPoint * endpoint, CHIP_ERROR err)
{
    // BTP handshake completed (or failed). If this endpoint is ours, complete the
    // ProxyConnectRequest exchange.
    if (mOwner.mPendingConnect.has_value() && mOwner.mPendingConnect->endpoint == endpoint)
    {
        mOwner.mTimerDelegate.CancelTimer(&mOwner.mConnectTimer);

        ConnectCtx ctx = std::move(*mOwner.mPendingConnect);
        mOwner.mPendingConnect.reset();
        app::CommandHandler * cmd = ctx.handle.Get();

        if (err != CHIP_NO_ERROR || cmd == nullptr)
        {
            ChipLogError(AppServer, "ProxyBleDelegate: OnEndPointConnectComplete err=%" CHIP_ERROR_FORMAT " cmd=%p", err.Format(),
                         (void *) cmd);
            if (cmd != nullptr)
            {
                cmd->AddStatus(ctx.path, Status::Failure);
            }
            if (endpoint != nullptr)
            {
                endpoint->Close();
            }
            // Clear the sentinel after Close(): if Close() fired synchronously (normal
            // BTP failure), OnEndPointConnectionClosed already cleared it; if Close() was
            // a no-op (StartConnect failed internally and the endpoint was already
            // kState_Closed), the sentinel remains stale and must be cleared here.
            mOwner.mBtpHandshakeEndpoint = nullptr;
            // BTP handshake failed: no session was established, so release the scanner
            // back to a background scan paused for this connect.
            mOwner.ResumeBgScanIfNeeded();
            return;
        }

        if (ctx.cluster == nullptr)
        {
            ChipLogError(AppServer, "ProxyBleDelegate: cluster gone at connect complete; closing endpoint");
            if (endpoint != nullptr)
            {
                endpoint->Close();
            }
            mOwner.mBtpHandshakeEndpoint = nullptr;
            cmd->AddStatus(ctx.path, Status::Failure);
            mOwner.ResumeBgScanIfNeeded();
            return;
        }

        EndpointSlot * slot = mOwner.ClaimSlot();
        if (slot == nullptr)
        {
            // The cluster's MaxSessions gate should have rejected the ProxyConnectRequest
            // before it ever reached the driver, so this means the two disagree.
            ChipLogError(AppServer, "ProxyBleDelegate: no free session slot; closing endpoint");
            if (endpoint != nullptr)
            {
                endpoint->Close();
            }
            mOwner.mBtpHandshakeEndpoint = nullptr;
            cmd->AddStatus(ctx.path, Status::ResourceExhausted);
            mOwner.ResumeBgScanIfNeeded();
            return;
        }

        mOwner.mBtpHandshakeEndpoint = nullptr; // ownership moves to the slot below
        uint16_t sessionId           = ctx.cluster->Sessions().AllocSessionId();
        slot->inUse                  = true;
        slot->sessionId              = sessionId;
        slot->endpoint               = endpoint;
        ctx.cluster->Sessions().RegisterSession(sessionId, CapabilitiesBitmap::kBle, ctx.fabricIndex);

        ChipLogProgress(AppServer, "ProxyConnectRequest: BLE connected, proxy session %u (disc %u)", sessionId, ctx.discriminator);

        CHIP_ERROR stateErr = ctx.cluster->SetCPState(CommissioningProxyCluster::kState_CPConnected);
        if (stateErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyBleDelegate: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
        }

        Commands::ProxyConnectResponse::Type response;
        response.sessionID = sessionId;
        cmd->AddResponse(ctx.path, response);
        return;
    }
    if (mOriginalTransport != nullptr)
    {
        mOriginalTransport->OnEndPointConnectComplete(endpoint, err);
    }
}

void CommissioningProxyBleTransport::ProxyBleDelegate::OnEndPointMessageReceived(BLEEndPoint * endpoint,
                                                                                 System::PacketBufferHandle && msg)
{
    if (EndpointSlot * slot = mOwner.FindSlot(endpoint))
    {
        // DispatchMessageResponse copies the payload into the IM ProxyMessageResponse
        // synchronously, so msg can be released when this scope returns.
        if (mOwner.mHost != nullptr)
        {
            mOwner.mHost->Sessions().DispatchMessageResponse(slot->sessionId, msg->Start(), msg->DataLength());
        }
        return;
    }
    if (mOriginalTransport != nullptr)
    {
        mOriginalTransport->OnEndPointMessageReceived(endpoint, std::move(msg));
    }
}

void CommissioningProxyBleTransport::ProxyBleDelegate::OnEndPointConnectionClosed(BLEEndPoint * endpoint, CHIP_ERROR err)
{
    if (EndpointSlot * slot = mOwner.FindSlot(endpoint))
    {
        const uint16_t sid = slot->sessionId;
        ChipLogProgress(AppServer, "ProxyBleDelegate: session %u closed: %" CHIP_ERROR_FORMAT, sid, err.Format());

        // Free the slot before notifying, so a re-entrant callback cannot find it.
        *slot = EndpointSlot{};

        // Fail any in-flight ProxyMessageRequest for this session and drop it.
        if (mOwner.mHost != nullptr)
        {
            mOwner.mHost->Sessions().DispatchMessageFailure(sid, Status::Failure);
            mOwner.mHost->Sessions().RemoveSession(sid);
        }

        // Peer (commissionee) closed the session. Unlike ProxyDisconnectRequest, this
        // path does not run through the dispatcher's OnAllSessionsClosed(), so resume a
        // background scan paused for the connect once the last session is gone and no
        // connect is mid-flight.
        if (!mOwner.AnySessionOpen() && !mOwner.mPendingConnect.has_value())
        {
            mOwner.ResumeBgScanIfNeeded();
        }
        return;
    }
    // Endpoint was mid-BTP-handshake and closed by timeout/cancel/failure before a
    // session was established. It was never passed to mOriginalTransport, so do not
    // forward the close notification to it.
    if (endpoint != nullptr && endpoint == mOwner.mBtpHandshakeEndpoint)
    {
        mOwner.mBtpHandshakeEndpoint = nullptr;
        return;
    }
    if (mOriginalTransport != nullptr)
    {
        mOriginalTransport->OnEndPointConnectionClosed(endpoint, err);
    }
}

CHIP_ERROR CommissioningProxyBleTransport::ProxyBleDelegate::SetEndPoint(BLEEndPoint * endpoint)
{
    if (mOriginalTransport != nullptr)
    {
        return mOriginalTransport->SetEndPoint(endpoint);
    }
    return CHIP_NO_ERROR;
}

// ==================================================================
// Background-scan hardware hooks
// ==================================================================

CHIP_ERROR CommissioningProxyBleTransport::BgScanHardware::StartHardwareScan()
{
    return mOwner.mAdapter.StartScan(HandleBackgroundScanResult, &mOwner);
}

void CommissioningProxyBleTransport::BgScanHardware::StopHardwareScan()
{
    mOwner.mAdapter.StopScan();
}

void CommissioningProxyBleTransport::BgScanHardware::ClearCachedResults(BitMask<WiFiBandBitmap> bands)
{
    // BLE results carry no band, so this always arrives as a whole-transport clear.
    if (mOwner.mHost != nullptr)
    {
        mOwner.mHost->ScanCache().ClearTransport(CapabilitiesBitmap::kBle, bands);
    }
}

// ==================================================================
// Adapter discovery callbacks
// ==================================================================

void CommissioningProxyBleTransport::HandleBackgroundScanResult(void * context, ByteSpan address, uint16_t discriminator,
                                                                uint16_t vendorId, uint16_t productId)
{
    auto * self = static_cast<CommissioningProxyBleTransport *>(context);
    // Result cache, TTL and the combined MaxCachedResults cap all live in the cluster's
    // scan cache; this just builds the kBle result and reports it.
    if (self != nullptr && self->mHost != nullptr)
    {
        self->mHost->ScanCache().Report(MakeScanResult(address, discriminator, vendorId, productId));
    }
}

void CommissioningProxyBleTransport::HandleForegroundScanResult(void * context, ByteSpan address, uint16_t discriminator,
                                                                uint16_t vendorId, uint16_t productId)
{
    auto * self = static_cast<CommissioningProxyBleTransport *>(context);
    VerifyOrReturn(self != nullptr);

    // ProxyScanResponse spec rule (CommissioningProxy.adoc, ProxyScanResult field):
    // "Each found device SHALL be reported once based on discriminator/VendorID/
    // ProductID per transport." Transport is fixed to kBle here, so dedupe on the
    // remaining three.
    for (size_t i = 0; i < self->mScanResultCount; i++)
    {
        const ScanRecord & existing = self->mScanResults[i];
        if (existing.discriminator == discriminator && existing.vendorId == vendorId && existing.productId == productId)
        {
            return;
        }
    }

    // Bounded at insert rather than truncated at emit: the store is exactly
    // MaxCachedResults deep, which is also the cap the ProxyScanResponse must respect.
    if (self->mScanResultCount >= kMaxScanResults)
    {
        ChipLogProgress(AppServer, "Ble::Scan: result store full (%u); dropping discriminator=%u",
                        static_cast<unsigned>(kMaxScanResults), discriminator);
        return;
    }

    ScanRecord & r = self->mScanResults[self->mScanResultCount];
    r              = ScanRecord{};
    if (address.size() >= sizeof(r.mac))
    {
        memcpy(r.mac, address.data(), sizeof(r.mac));
    }
    r.discriminator = discriminator;
    r.vendorId      = vendorId;
    r.productId     = productId;
    self->mScanResultCount++;

    ChipLogProgress(AppServer, "Ble::Scan: discovered discriminator=%u vid=0x%04x pid=0x%04x", discriminator, vendorId, productId);
}

// ==================================================================
// BleLayer connect callbacks
// ==================================================================

void CommissioningProxyBleTransport::HandleConnectFound(void * appState, BLE_CONNECTION_OBJECT connObj)
{
    auto * self = static_cast<CommissioningProxyBleTransport *>(appState);
    VerifyOrReturn(self != nullptr);

    if (!self->mPendingConnect.has_value())
    {
        ChipLogProgress(AppServer, "OnConnectFound: no pending connect ctx; ignoring stale callback");
        return;
    }

    // Wrap the raw L2CAP connection into a BLEEndPoint (Central role, autoClose=true)
    // via the no-callback NewBleConnectionByObject path so OnConnectionComplete in
    // BleLayer creates the endpoint and dispatches to our BleLayerDelegate.
    auto * layer = self->GetBleLayer();
    if (layer == nullptr)
    {
        ChipLogError(AppServer, "OnConnectFound: BleLayer null");
        self->FailPendingConnect(Status::Failure);
        return;
    }
    CHIP_ERROR err = layer->NewBleConnectionByObject(connObj);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "OnConnectFound: NewBleConnectionByObject failed: %" CHIP_ERROR_FORMAT, err.Format());
        // No further callback will fire — clean up synchronously rather than waiting for
        // the connect timer to expire.
        self->FailPendingConnect(Status::Failure);
        return;
    }
    // After this returns the delegate's OnBleConnectionComplete has run and either
    // recorded the endpoint + started BTP, or failed (cleanup happens in
    // OnBleConnectionError or the connect timeout).
}

void CommissioningProxyBleTransport::HandleConnectError(void * appState, CHIP_ERROR err)
{
    auto * self = static_cast<CommissioningProxyBleTransport *>(appState);
    VerifyOrReturn(self != nullptr);

    if (!self->mPendingConnect.has_value())
    {
        ChipLogProgress(AppServer,
                        "OnConnectError: ignoring stale callback "
                        "(err: %" CHIP_ERROR_FORMAT ")",
                        err.Format());
        return;
    }
    ChipLogError(AppServer, "ProxyConnectRequest: BLE scan/connect failed: %" CHIP_ERROR_FORMAT, err.Format());
    self->FailPendingConnect(Status::Failure);
}

void CommissioningProxyBleTransport::OnConnectTimeout()
{

    if (!mPendingConnect.has_value())
    {
        return; // Already resolved.
    }

    ChipLogError(AppServer, "ProxyConnectRequest: timeout waiting for BLE connect");

    ConnectCtx ctx = std::move(*mPendingConnect);
    mPendingConnect.reset();

    // Stop the BleLayer's in-flight scan/connect, suppressing any later OnConnectFound /
    // OnConnectError callbacks.
    if (auto * layer = GetBleLayer())
    {
        CHIP_ERROR cancelErr = layer->CancelBleIncompleteConnection();
        if (cancelErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "OnConnectTimeout: CancelBleIncompleteConnection: %" CHIP_ERROR_FORMAT, cancelErr.Format());
        }
    }

    // If the endpoint was already created (BTP handshake in flight), close it. Leave
    // mBtpHandshakeEndpoint set: OnEndPointConnectionClosed checks it to suppress
    // forwarding the close to mOriginalTransport. The callback clears it.
    if (ctx.endpoint != nullptr)
    {
        ctx.endpoint->Close();
    }

    if (app::CommandHandler * cmd = ctx.handle.Get())
    {
        cmd->AddStatus(ctx.path, Status::Timeout);
    }
    ResumeBgScanIfNeeded();
}

void CommissioningProxyBleTransport::OnScanWindowExpired()
{
    mAdapter.StopScan();
    mScanInProgress = false;

    // Hand the results to the cluster's scan aggregator; it owns the command handle and
    // emits the combined ProxyScanResponse once every transport's sub-scan has reported.
    if (mHost != nullptr)
    {
        // Clamp to the cluster's advertised cap as well as the store's own bound, so the
        // aggregator's numberOfResults field (uint8_t) cannot overflow even if the two
        // ever diverge.
        const size_t count = std::min<size_t>(mScanResultCount, mHost->GetMaxCachedResults());
        ScanResultT out[kMaxScanResults];
        for (size_t i = 0; i < count; i++)
        {
            const ScanRecord & d = mScanResults[i];
            out[i]               = MakeScanResult(ByteSpan(d.mac, sizeof(d.mac)), d.discriminator, d.vendorId, d.productId);
        }
        mHost->ScanAggregator().Contribute(Span<const ScanResultT>(out, count));
    }
    mScanResultCount = 0;

    // The foreground scan has released the BLE scanner; resume a background scan that
    // was paused to make room for it.
    ResumeBgScanIfNeeded();
}

// ==================================================================
// CommissioningProxyTransport
// ==================================================================

void CommissioningProxyBleTransport::SetHost(CommissioningProxyCluster * host)
{
    mHost = host;
}

Status CommissioningProxyBleTransport::Connect(app::CommandHandler * commandObj, const DataModel::InvokeRequest & request,
                                               uint16_t discriminator, System::Clock::Seconds16 timeout)
{
    if (mPendingConnect.has_value())
    {
        ChipLogError(AppServer, "ProxyConnectRequest: a BLE connect is already in progress");
        return Status::Busy;
    }

    auto * layer = GetBleLayer();
    if (layer == nullptr)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: BleLayer null");
        return Status::Failure;
    }

    // Sequential mode switch: the CP started in BLE peripheral mode so it could be
    // commissioned onto the fabric (Step 1). The first BLE ProxyConnectRequest
    // transitions BLE to central role so we can scan / connect to the commissionee
    // (Step 2). Idempotent — subsequent ProxyConnectRequest calls are no-ops.
    {
        CHIP_ERROR switchErr = mAdapter.EnableCentralRole();
        if (switchErr == CHIP_ERROR_BUSY)
        {
            ChipLogError(AppServer, "ProxyConnectRequest: BLE busy with prior peripheral activity; retry shortly");
            return Status::Busy;
        }
        if (switchErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyConnectRequest: EnableCentralRole failed: %" CHIP_ERROR_FORMAT, switchErr.Format());
            return Status::Failure;
        }
    }

    // Install our BleLayerDelegate wrapper (idempotent).
    mProxyDelegate.Install();

    // The connect's NewBleConnectionByDiscriminator uses the single BLE scanner. Pause
    // any running background scan so it does not contend for the scanner; it resumes
    // once the session closes (OnAllSessionsClosed) or the connect fails below.
    PauseBgScan();

    // Per spec a Timeout of 0 indicates no timeout: the connect runs until it succeeds,
    // fails, or is cancelled via ProxyDisconnectRequest(null).
    const bool hasTimeout = (timeout.count() > 0);

    ConnectCtx & ctx  = mPendingConnect.emplace();
    ctx.handle        = app::CommandHandler::Handle(commandObj);
    ctx.path          = request.path;
    ctx.discriminator = discriminator;
    ctx.cluster       = mHost;
    ctx.fabricIndex   = request.subjectDescriptor.fabricIndex;
    commandObj->FlushAcksRightAwayOnSlowCommand();

    if (auto * exchange = commandObj->GetExchangeContext())
    {
        // Keep the exchange open until just past the connect timeout, or disable the
        // response timer entirely (kZero) when there is no timeout. Clamp the +5 s margin
        // so a near-max timeout cannot wrap the uint16 seconds field.
        const uint16_t timeoutSecs = timeout.count();
        const uint16_t responseSecs =
            (timeoutSecs > static_cast<uint16_t>(0xFFFF - 5)) ? 0xFFFF : static_cast<uint16_t>(timeoutSecs + 5);
        exchange->SetResponseTimeout(hasTimeout ? System::Clock::Seconds16(responseSecs) : System::Clock::kZero);
    }

    SetupDiscriminator setupDisc;
    setupDisc.SetLongValue(discriminator);

    CHIP_ERROR err = layer->NewBleConnectionByDiscriminator(setupDisc, this, HandleConnectFound, HandleConnectError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: NewBleConnectionByDiscriminator failed: %" CHIP_ERROR_FORMAT, err.Format());
        mPendingConnect.reset();
        ResumeBgScanIfNeeded();
        return Status::Failure;
    }

    if (hasTimeout)
    {
        CHIP_ERROR timerErr = mTimerDelegate.StartTimer(&mConnectTimer, timeout);
        if (timerErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyConnectRequest: StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            // Cancel the BLE scan we just started so we don't leak it.
            (void) layer->CancelBleIncompleteConnection();
            mPendingConnect.reset();
            ResumeBgScanIfNeeded();
            return Status::Failure;
        }
    }

    ChipLogProgress(AppServer, "ProxyConnectRequest: BLE scan/connect started for discriminator %u", discriminator);
    return Status::Success;
}

Status CommissioningProxyBleTransport::CancelPendingConnect(FabricIndex fabricIndex)
{
    if (!mPendingConnect.has_value())
    {
        return Status::InvalidInState;
    }

    if (fabricIndex != mPendingConnect->fabricIndex)
    {
        ChipLogProgress(AppServer, "CancelPendingConnect: pending BLE connect owned by fabric %u, rejected fabric %u",
                        mPendingConnect->fabricIndex, fabricIndex);
        return Status::NotFound;
    }

    mTimerDelegate.CancelTimer(&mConnectTimer);

    ConnectCtx ctx = std::move(*mPendingConnect);
    mPendingConnect.reset();

    if (auto * layer = GetBleLayer())
    {
        CHIP_ERROR cancelErr = layer->CancelBleIncompleteConnection();
        if (cancelErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "CancelPendingConnect: CancelBleIncompleteConnection: %" CHIP_ERROR_FORMAT,
                          cancelErr.Format());
        }
    }

    // Leave mBtpHandshakeEndpoint set so OnEndPointConnectionClosed suppresses
    // forwarding the close to mOriginalTransport. The callback clears it.
    if (ctx.endpoint != nullptr)
    {
        ctx.endpoint->Close();
    }

    if (app::CommandHandler * cmd = ctx.handle.Get())
    {
        cmd->AddStatus(ctx.path, Status::Failure);
    }

    ResumeBgScanIfNeeded();

    return Status::Success;
}

Status CommissioningProxyBleTransport::Disconnect(uint16_t sessionId)
{
    EndpointSlot * slot = FindSlot(sessionId);
    if (slot == nullptr)
    {
        return Status::NotFound;
    }

    BLEEndPoint * ep = slot->endpoint;

    // Free the slot before Close so OnEndPointConnectionClosed (which fires
    // synchronously) finds no matching session and does not call DispatchMessageFailure
    // for this local-initiated disconnect. Set the BTP sentinel so the close is not
    // forwarded to mOriginalTransport.
    *slot = EndpointSlot{};

    if (ep != nullptr)
    {
        mBtpHandshakeEndpoint = ep;
        ep->Close();
        // Close() fires synchronously; the callback clears mBtpHandshakeEndpoint. Clear
        // explicitly here in case Close() was a no-op (already-closed endpoint).
        mBtpHandshakeEndpoint = nullptr;
    }

    return Status::Success;
}

CHIP_ERROR CommissioningProxyBleTransport::SendMessage(uint16_t sessionId, System::PacketBufferHandle && buf)
{
    EndpointSlot * slot = FindSlot(sessionId);
    if (slot == nullptr)
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    if (slot->endpoint == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return slot->endpoint->Send(std::move(buf));
}

Status CommissioningProxyBleTransport::Scan(System::Clock::Seconds16 scanMaxTime)
{
    if (mScanInProgress)
    {
        ChipLogProgress(AppServer, "Ble::Scan: scan already in progress — returning Busy");
        return Status::Busy;
    }

    // The foreground scan and the background scan share the single BLE scanner. Pause any
    // running background scan so StartScan below does not fail with BUSY; the scan window
    // expiry resumes it when the foreground scan completes.
    PauseBgScan();

    mScanResultCount = 0;
    CHIP_ERROR err   = mAdapter.StartScan(HandleForegroundScanResult, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Ble::Scan: StartScan failed: %" CHIP_ERROR_FORMAT, err.Format());
        ResumeBgScanIfNeeded();
        return Status::Failure;
    }
    mScanInProgress = true;

    CHIP_ERROR timerErr = mTimerDelegate.StartTimer(&mScanTimer, scanMaxTime);
    if (timerErr != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Ble::Scan: StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
        mAdapter.StopScan();
        mScanInProgress = false;
        ResumeBgScanIfNeeded();
        return Status::Failure;
    }

    ChipLogProgress(AppServer, "Ble::Scan: started, scanMaxTime=%us", scanMaxTime.count());
    return Status::Success;
}

Status CommissioningProxyBleTransport::BgScanStart(System::Clock::Seconds16 timeout, BitMask<WiFiBandBitmap> /*wiFiBands*/,
                                                   FabricIndex fabricIndex, NodeId nodeId)
{
    // BLE has no Wi-Fi bands: register with the kBle transport and empty bands. The
    // registry starts or defers the hardware scan (StartScan reports BUSY while a connect
    // or foreground scan holds the single scanner).
    return mBgScan.Start(fabricIndex, nodeId, BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kBle), BitMask<WiFiBandBitmap>{},
                         timeout);
}

Status CommissioningProxyBleTransport::BgScanStop(BitMask<CapabilitiesBitmap> transport, BitMask<WiFiBandBitmap> /*wiFiBands*/,
                                                  FabricIndex fabricIndex, NodeId nodeId)
{
    // BLE ignores Wi-Fi bands; pass the transport through with empty bands. The registry
    // returns NOT_FOUND for an unknown fabric, SUCCESS when the request does not target
    // this transport, and stops the scanner + clears the cache when the last fabric is
    // removed.
    return mBgScan.Stop(fabricIndex, nodeId, transport, BitMask<WiFiBandBitmap>{});
}

void CommissioningProxyBleTransport::OnFabricRemoved(FabricIndex fabricIndex)
{
    // The cluster has already dropped the fabric's sessions; this drops its background
    // scans so nothing survives into a reused FabricIndex.
    mBgScan.RemoveFabric(fabricIndex);
}

void CommissioningProxyBleTransport::OnAllSessionsClosed()
{
    // The proxy uses a one-way peripheral->central role switch on the first
    // ProxyConnectRequest(kBle), so there is no peripheral advertising state to resume.
    // But a background scan paused for the connect must be resumed now that the session
    // has closed and the scanner is free again.
    ResumeBgScanIfNeeded();
}

bool CommissioningProxyBleTransport::IsConnectPending() const
{
    return mPendingConnect.has_value();
}

void CommissioningProxyBleTransport::Shutdown()
{
    // Stop any foreground scan first. Its timer outlives the cluster otherwise, and the
    // window expiry would then hand results to mHost — a cluster that no longer exists —
    // and call ResumeBgScanIfNeeded(), restarting the radio after teardown.
    mTimerDelegate.CancelTimer(&mScanTimer);
    if (mScanInProgress)
    {
        mAdapter.StopScan();
        mScanInProgress = false;
    }
    mScanResultCount = 0;

    // Take the endpoints out first so the slots are free when Close() fires
    // OnEndPointConnectionClosed synchronously — FindSlot returns nullptr and the
    // callbacks are no-ops rather than mutating a slot mid-iteration.
    BLEEndPoint * closing[kMaxSessions] = {};
    size_t closingCount                 = 0;
    for (auto & slot : mEndpoints)
    {
        if (slot.inUse && slot.endpoint != nullptr)
        {
            closing[closingCount++] = slot.endpoint;
        }
        slot = EndpointSlot{};
    }
    for (size_t i = 0; i < closingCount; i++)
    {
        closing[i]->Close();
    }

    // If a ProxyConnectRequest was in flight, cancel its timeout and fail the exchange.
    // Do not call FailPendingConnect() here — that would invoke ResumeBgScanIfNeeded(),
    // which would restart the HW scan immediately before the cleanup below stops and
    // clears it.
    if (mPendingConnect.has_value())
    {
        mTimerDelegate.CancelTimer(&mConnectTimer);

        ConnectCtx ctx = std::move(*mPendingConnect);
        mPendingConnect.reset();

        // Stop the BleLayer's in-flight scan/connect, so no later OnConnectFound /
        // OnConnectError reaches the torn-down transport.
        if (auto * layer = GetBleLayer())
        {
            CHIP_ERROR cancelErr = layer->CancelBleIncompleteConnection();
            if (cancelErr != CHIP_NO_ERROR)
            {
                ChipLogDetail(AppServer, "Shutdown: CancelBleIncompleteConnection: %" CHIP_ERROR_FORMAT, cancelErr.Format());
            }
        }

        // A BTP handshake in flight has an endpoint that was not in a slot, so the loop
        // above did not close it. Leave mBtpHandshakeEndpoint set as in OnConnectTimeout:
        // OnEndPointConnectionClosed matches it to suppress forwarding the close to
        // mOriginalTransport, and clears it.
        if (ctx.endpoint != nullptr)
        {
            ctx.endpoint->Close();
        }
        else
        {
            mBtpHandshakeEndpoint = nullptr;
        }

        if (app::CommandHandler * cmd = ctx.handle.Get())
        {
            cmd->AddStatus(ctx.path, Status::Failure);
        }
    }

    // Tear down the background scan: cancel every per-fabric lifetime timer and stop the
    // hardware scan if the registry currently owns it.
    mBgScan.Shutdown();

    // Restore BleLayer::mBleTransport so a later BLE event reaches the application's own
    // delegate rather than this object after the cluster is gone.
    mProxyDelegate.Uninstall();

    // Last: the cluster is going away, so drop the pointer to it. Every callback above
    // null-checks mHost, and any that still reaches the platform (a scan result already
    // queued through ScheduleWork) must find null rather than a destroyed cluster.
    mHost = nullptr;
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
