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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyPafTransport.h>

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <wifipaf/WiFiPAFLayer.h>

#include <algorithm>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

using Protocols::InteractionModel::Status;

CommissioningProxyPafTransport::CommissioningProxyPafTransport(CommissioningProxyPafAdapter & adapter,
                                                               TimerDelegate & timerDelegate, FabricTable * fabricTable) :
    mAdapter(adapter), mTimerDelegate(timerDelegate), mBgScan(mBgScanHardware, timerDelegate), mFabricTable(fabricTable)
{}

CommissioningProxyPafTransport::~CommissioningProxyPafTransport()
{
    // Safe to run twice, and required if the owner never called it: the connect timer and
    // the installed WiFiPAF delegate would otherwise outlive this object.
    Shutdown();
}

// ==================================================================
// Small helpers
// ==================================================================

CommissioningProxyPafTransport::SessionSlot * CommissioningProxyPafTransport::FindSlot(uint16_t sessionId)
{
    for (auto & slot : mSessions)
    {
        if (slot.inUse && slot.sessionId == sessionId)
        {
            return &slot;
        }
    }
    return nullptr;
}

CommissioningProxyPafTransport::SessionSlot * CommissioningProxyPafTransport::FindSlotByPeer(uint32_t peerId)
{
    for (auto & slot : mSessions)
    {
        if (slot.inUse && slot.pafSession.peer_id == peerId)
        {
            return &slot;
        }
    }
    return nullptr;
}

CommissioningProxyPafTransport::SessionSlot * CommissioningProxyPafTransport::ClaimSlot()
{
    for (auto & slot : mSessions)
    {
        if (!slot.inUse)
        {
            return &slot;
        }
    }
    return nullptr;
}

bool CommissioningProxyPafTransport::AnySessionOpen() const
{
    for (const auto & slot : mSessions)
    {
        if (slot.inUse)
        {
            return true;
        }
    }
    return false;
}

CommissioningProxyPafTransport::ScanResultT CommissioningProxyPafTransport::MakeScanResult(ByteSpan address, uint16_t discriminator,
                                                                                           uint16_t vendorId, uint16_t productId,
                                                                                           ByteSpan extendedData, uint16_t wiFiBand)
{
    ScanResultT r{};
    r.address.SetNonNull(address);
    r.transport     = BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kWiFiPAF);
    r.discriminator = discriminator;
    r.vendorID      = static_cast<VendorId>(vendorId);
    r.productID     = productId;
    if (!extendedData.empty())
    {
        r.extendedData.SetNonNull(extendedData);
    }
    else
    {
        r.extendedData.SetNull();
    }
    if (wiFiBand != 0)
    {
        r.wiFiBand.SetValue(static_cast<WiFiBandBitmap>(wiFiBand));
    }
    else
    {
        r.wiFiBand.ClearValue();
    }
    return r;
}

// ==================================================================
// ProxyPafDelegate
// ==================================================================

void CommissioningProxyPafTransport::ProxyPafDelegate::Install()
{
    auto & layer = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    if (layer.mWiFiPAFTransport == this)
    {
        return;
    }
    mOriginalTransport      = layer.mWiFiPAFTransport;
    layer.mWiFiPAFTransport = this;
    ChipLogProgress(AppServer, "ProxyPafDelegate: installed (original=%p)", (void *) mOriginalTransport);
}

void CommissioningProxyPafTransport::ProxyPafDelegate::Uninstall()
{
    auto & layer = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    if (layer.mWiFiPAFTransport == this)
    {
        layer.mWiFiPAFTransport = mOriginalTransport;
        ChipLogProgress(AppServer, "ProxyPafDelegate: uninstalled");
    }
    mOriginalTransport = nullptr;
}

CHIP_ERROR CommissioningProxyPafTransport::ProxyPafDelegate::WiFiPAFMessageReceived(WiFiPAF::WiFiPAFSession & rxInfo,
                                                                                    System::PacketBufferHandle && msg)
{
    if (SessionSlot * slot = mOwner.FindSlotByPeer(rxInfo.peer_id))
    {
        // DispatchMessageResponse copies the payload into the IM ProxyMessageResponse
        // synchronously, so msg can be released when this scope returns.
        if (mOwner.mHost != nullptr)
        {
            mOwner.mHost->Sessions().DispatchMessageResponse(slot->sessionId, msg->Start(), msg->DataLength());
        }
        return CHIP_NO_ERROR;
    }
    if (mOriginalTransport != nullptr)
    {
        return mOriginalTransport->WiFiPAFMessageReceived(rxInfo, std::move(msg));
    }
    ChipLogError(AppServer, "ProxyPafDelegate: no original transport for non-proxy session");
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR CommissioningProxyPafTransport::ProxyPafDelegate::WiFiPAFMessageSend(WiFiPAF::WiFiPAFSession & txInfo,
                                                                                System::PacketBufferHandle && msg)
{
    if (mOriginalTransport != nullptr)
    {
        return mOriginalTransport->WiFiPAFMessageSend(txInfo, std::move(msg));
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR CommissioningProxyPafTransport::ProxyPafDelegate::WiFiPAFCloseSession(WiFiPAF::WiFiPAFSession & sessionInfo)
{
    // The PAF session closed on its own — e.g. the ED dropped NAN when ConnectNetwork
    // switched it to infrastructure Wi-Fi, so the PAFTP ack-received timer fired and the
    // layer is closing the session here. There is no ProxyDisconnectRequest, so this
    // handler runs the same teardown the cluster would on disconnect: fail any in-flight
    // ProxyMessage, drop the session, terminate the NAN subscribe, and resume a
    // background scan that was paused for the connect. (Mirrors the BLE transport's
    // OnEndPointConnectionClosed handler.)
    if (SessionSlot * slot = mOwner.FindSlotByPeer(sessionInfo.peer_id))
    {
        const uint16_t sid                 = slot->sessionId;
        WiFiPAF::WiFiPAFSession pafSession = slot->pafSession;

        ChipLogError(AppServer, "WiFiPAFCloseSession: PAF session for proxy session %u closed by peer — cleaning up", sid);

        // Free the slot before notifying, so a re-entrant callback cannot find it.
        *slot = SessionSlot{};

        // Resolve any in-flight ProxyMessageRequest so the commissioner gets a timely
        // error instead of waiting out its 30 s timeout.
        if (mOwner.mHost != nullptr)
        {
            mOwner.mHost->Sessions().DispatchMessageFailure(sid, Status::Failure);
            // Only the last session leaving makes the proxy disconnected: the slot is
            // already freed above, so AnySessionOpen() reflects what remains. The
            // ProxyDisconnectRequest path is not handled here - the cluster sets the state
            // itself once it has torn the session down.
            if (!mOwner.AnySessionOpen())
            {
                CHIP_ERROR stateErr = mOwner.mHost->SetCPState(CommissioningProxyCluster::kState_CPDisconnected);
                if (stateErr != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "WiFiPAFCloseSession: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
                }
            }
        }

        WiFiPAF::WiFiPAFLayer & layer = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
        (void) layer.RmPafSession(WiFiPAF::PafInfoAccess::kAccSessionId, pafSession);
        // Terminate the NAN subscribe (for a subscriber session the WiFiPAFSession id IS
        // the subscribe_id) so it does not linger in wpa_supplicant — the same leak the
        // ProxyDisconnect path guards against. Do NOT CloseEndPoint here: the layer is
        // already closing the endpoint, which is why we were called.
        if (pafSession.id != 0)
        {
            (void) DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(pafSession.id);
        }
        if (mOwner.mHost != nullptr)
        {
            mOwner.mHost->Sessions().RemoveSession(sid);
        }

        // This path does not run through the cluster's OnAllSessionsClosed(), so resume a
        // background scan paused for the connect once the last session is gone and no
        // connect is mid-flight.
        if (!mOwner.AnySessionOpen() && !mOwner.mPendingConnect.has_value())
        {
            mOwner.OnAllSessionsClosed();
        }
    }
    if (mOriginalTransport != nullptr)
    {
        return mOriginalTransport->WiFiPAFCloseSession(sessionInfo);
    }
    return CHIP_NO_ERROR;
}

bool CommissioningProxyPafTransport::ProxyPafDelegate::WiFiPAFResourceAvailable()
{
    if (mOriginalTransport != nullptr)
    {
        return mOriginalTransport->WiFiPAFResourceAvailable();
    }
    return true;
}

// ==================================================================
// Background-scan hardware hooks
// ==================================================================

CHIP_ERROR CommissioningProxyPafTransport::BgScanHardware::StartHardwareScan()
{
    if (mOwner.mPendingConnect.has_value())
    {
        // A ProxyConnect owns the single NAN subscribe slot; defer.
        return CHIP_ERROR_BUSY;
    }
    return mOwner.mAdapter.StartBackgroundScan(HandleBackgroundScanResult, &mOwner);
}

void CommissioningProxyPafTransport::BgScanHardware::StopHardwareScan()
{
    mOwner.mAdapter.StopBackgroundScan();
}

void CommissioningProxyPafTransport::BgScanHardware::ClearCachedResults(BitMask<WiFiBandBitmap> bands)
{
    // bands == 0 means the transport itself stopped; otherwise only those bands did, and
    // only their results go. PAFTP results are the only ones carrying a band.
    if (mOwner.mHost != nullptr)
    {
        mOwner.mHost->ScanCache().ClearTransport(CapabilitiesBitmap::kWiFiPAF, bands);
    }
}

// ==================================================================
// Adapter discovery callbacks
// ==================================================================

void CommissioningProxyPafTransport::HandleBackgroundScanResult(void * context, ByteSpan address, uint16_t discriminator,
                                                                uint16_t vendorId, uint16_t productId, ByteSpan extendedData,
                                                                uint16_t wiFiBand)
{
    auto * self = static_cast<CommissioningProxyPafTransport *>(context);
    // Result cache, TTL and the combined MaxCachedResults cap all live in the cluster's
    // scan cache; this just builds the kWiFiPAF result and reports it.
    if (self != nullptr && self->mHost != nullptr)
    {
        self->mHost->ScanCache().Report(MakeScanResult(address, discriminator, vendorId, productId, extendedData, wiFiBand));
    }
}

void CommissioningProxyPafTransport::HandleForegroundScanResult(void * context, ByteSpan address, uint16_t discriminator,
                                                                uint16_t vendorId, uint16_t productId, ByteSpan extendedData,
                                                                uint16_t wiFiBand)
{
    auto * self = static_cast<CommissioningProxyPafTransport *>(context);
    VerifyOrReturn(self != nullptr);

    // ProxyScanResponse spec rule (CommissioningProxy.adoc, ProxyScanResult field):
    // "Each found device SHALL be reported once based on discriminator/VendorID/
    // ProductID per transport." Transport is fixed to kWiFiPAF here, so dedupe on the
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
        ChipLogProgress(AppServer, "Paf::Scan: result store full (%u); dropping discriminator=%u",
                        static_cast<unsigned>(kMaxScanResults), discriminator);
        return;
    }

    ScanRecord & r = self->mScanResults[self->mScanResultCount];
    r              = ScanRecord{};
    // Copy the variable-length fields: the adapter's spans die when this returns, while
    // the record lives until the scan completes.
    if (!address.empty() && address.size() <= sizeof(r.address))
    {
        memcpy(r.address, address.data(), address.size());
        r.addressLen = static_cast<uint8_t>(address.size());
    }
    if (!extendedData.empty() && extendedData.size() <= sizeof(r.extendedData))
    {
        memcpy(r.extendedData, extendedData.data(), extendedData.size());
        r.extendedDataLen = static_cast<uint8_t>(extendedData.size());
    }
    else if (extendedData.size() > sizeof(r.extendedData))
    {
        // Dropped rather than truncated: a partial blob reported as complete would be
        // worse than none. The device itself is still reported.
        ChipLogProgress(AppServer, "Paf::Scan: discriminator=%u ExtendedData is %u bytes, max %u - omitting it", discriminator,
                        static_cast<unsigned>(extendedData.size()), static_cast<unsigned>(sizeof(r.extendedData)));
    }
    r.discriminator = discriminator;
    r.vendorId      = vendorId;
    r.productId     = productId;
    r.wiFiBand      = wiFiBand;
    self->mScanResultCount++;

    ChipLogProgress(AppServer, "Paf::Scan: discovered discriminator=%u vid=0x%04x pid=0x%04x band=%u", discriminator, vendorId,
                    productId, wiFiBand);
}

void CommissioningProxyPafTransport::HandleForegroundScanDone(void * context)
{
    auto * self = static_cast<CommissioningProxyPafTransport *>(context);
    VerifyOrReturn(self != nullptr);

    self->mScanInProgress = false;

    // Hand the results to the cluster's scan aggregator; it owns the command handle and
    // emits the combined ProxyScanResponse once every transport's sub-scan has reported.
    if (self->mHost != nullptr)
    {
        // Clamp to the cluster's advertised cap as well as the store's own bound, so the
        // aggregator's numberOfResults field (uint8_t) cannot overflow even if the two
        // ever diverge.
        const size_t count = std::min<size_t>(self->mScanResultCount, self->mHost->GetMaxCachedResults());
        ScanResultT out[kMaxScanResults];
        for (size_t i = 0; i < count; i++)
        {
            const ScanRecord & d = self->mScanResults[i];
            out[i]               = MakeScanResult(ByteSpan(d.address, d.addressLen), d.discriminator, d.vendorId, d.productId,
                                                  ByteSpan(d.extendedData, d.extendedDataLen), d.wiFiBand);
        }
        self->mHost->ScanAggregator().Contribute(Span<const ScanResultT>(out, count));
    }
    self->mScanResultCount = 0;

    // The one-shot scan has released the NAN subscribe slot; resume a background scan
    // that was paused to make room for it.
    self->OnAllSessionsClosed();
}

// ==================================================================
// Connect
// ==================================================================

void CommissioningProxyPafTransport::FailPendingConnect(Status status, bool cancelTimer)
{
    // Every caller has already verified a connect is pending; the check is repeated here
    // because bugprone-unchecked-optional-access does not reason across functions, and
    // the dereference below would otherwise fail the clang-tidy gate.
    VerifyOrReturn(mPendingConnect.has_value());

    if (cancelTimer)
    {
        mTimerDelegate.CancelTimer(&mConnectTimer);
    }

    // Move out and reset before answering, so a re-entrant callback sees no pending
    // connect.
    ConnectCtx ctx = std::move(*mPendingConnect);
    mPendingConnect.reset();

    // Null the platform connect callbacks before cancelling the subscribe so the
    // cancel-connect event posted by the platform does not re-enter HandleConnectError.
    CHIP_ERROR cancelIncompleteErr = DeviceLayer::ConnectivityMgr().WiFiPAFCancelIncompleteSubscribe();
    if (cancelIncompleteErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "FailPendingConnect: WiFiPAFCancelIncompleteSubscribe: %" CHIP_ERROR_FORMAT,
                      cancelIncompleteErr.Format());
    }

    WiFiPAF::WiFiPAFLayer & pafLayer = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    WiFiPAF::WiFiPAFSession key{};
    key.nodeId        = static_cast<NodeId>(ctx.discriminator);
    key.discriminator = ctx.discriminator;

    // Capture the session before RmPafSession clears the slot so we can close any PAFTP
    // endpoint the handshake created; otherwise it leaks from the 2-slot pool until its
    // own timer self-closes. CloseEndPoint is a no-op if there is none.
    WiFiPAF::WiFiPAFSession endpointSession{};
    bool haveEndpoint               = false;
    WiFiPAF::WiFiPAFSession * pInfo = pafLayer.GetPAFInfo(WiFiPAF::PafInfoAccess::kAccDisc, key);
    if (pInfo != nullptr)
    {
        endpointSession = *pInfo;
        haveEndpoint    = true;
    }

    CHIP_ERROR rmErr = pafLayer.RmPafSession(WiFiPAF::PafInfoAccess::kAccNodeInfo, key);
    if (rmErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "FailPendingConnect: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr.Format());
    }
    if (haveEndpoint)
    {
        pafLayer.CloseEndPoint(endpointSession);
    }

    if (ctx.subscribeId != 0)
    {
        CHIP_ERROR cancelErr = DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(ctx.subscribeId);
        if (cancelErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "FailPendingConnect: WiFiPAFCancelSubscribe(%u): %" CHIP_ERROR_FORMAT, ctx.subscribeId,
                          cancelErr.Format());
        }
    }

    if (app::CommandHandler * cmd = ctx.handle.Get())
    {
        cmd->AddStatus(ctx.path, status);
    }

    // The connect freed the NAN subscribe slot; resume a background scan that was
    // deferred or paused for it (a failed connect registers no session, so the cluster's
    // OnAllSessionsClosed path does not run).
    OnAllSessionsClosed();
}

void CommissioningProxyPafTransport::OnConnectTimeout()
{
    if (!mPendingConnect.has_value())
    {
        return; // Success or error callback already fired first; nothing to do.
    }
    ChipLogError(AppServer, "ProxyConnectRequest: timeout waiting for WiFiPAF connect (disc %u)", mPendingConnect->discriminator);
    FailPendingConnect(Status::Timeout, /*cancelTimer=*/false);
}

void CommissioningProxyPafTransport::HandleConnectSuccess(void * context)
{
    auto * self = static_cast<CommissioningProxyPafTransport *>(context);
    VerifyOrReturn(self != nullptr);

    if (!self->mPendingConnect.has_value())
    {
        ChipLogProgress(AppServer, "OnConnectSuccess: no pending connect ctx; ignoring stale callback");
        return;
    }
    self->mTimerDelegate.CancelTimer(&self->mConnectTimer);

    const uint16_t discriminator = self->mPendingConnect->discriminator;

    WiFiPAF::WiFiPAFSession keyInfo{};
    keyInfo.discriminator              = discriminator;
    WiFiPAF::WiFiPAFLayer & layer      = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer();
    WiFiPAF::WiFiPAFSession * pPafInfo = layer.GetPAFInfo(WiFiPAF::PafInfoAccess::kAccDisc, keyInfo);

    app::CommandHandler * cmd = self->mPendingConnect->handle.Get();
    if (cmd == nullptr || pPafInfo == nullptr)
    {
        ChipLogError(AppServer, "OnConnectSuccess: cmd=%p pPafInfo=%p for disc %u", (void *) cmd, (void *) pPafInfo, discriminator);
        // The connect completed but the session cannot be handed to the commissioner
        // (originating exchange gone, or the PAF session vanished). Run the full teardown
        // so an established-but-untracked session does not leak its PAF pool slot, PAFTP
        // endpoint and NAN subscribe. The timeout timer was already cancelled above.
        self->FailPendingConnect(Status::Failure, /*cancelTimer=*/false);
        return;
    }

    if (self->mPendingConnect->cluster == nullptr)
    {
        ChipLogError(AppServer, "OnConnectSuccess: cluster gone at connect complete; tearing down PAF session");
        self->FailPendingConnect(Status::Failure, /*cancelTimer=*/false);
        return;
    }

    SessionSlot * slot = self->ClaimSlot();
    if (slot == nullptr)
    {
        // The cluster's MaxSessions gate should have rejected the ProxyConnectRequest
        // before it reached the driver, so this means the two disagree.
        ChipLogError(AppServer, "OnConnectSuccess: no free session slot; tearing down PAF session");
        self->FailPendingConnect(Status::ResourceExhausted, /*cancelTimer=*/false);
        return;
    }

    ConnectCtx ctx = std::move(*self->mPendingConnect);
    self->mPendingConnect.reset();

    const uint16_t sessionId = ctx.cluster->Sessions().AllocSessionId();
    slot->inUse              = true;
    slot->sessionId          = sessionId;
    slot->pafSession         = *pPafInfo;
    ctx.cluster->Sessions().RegisterSession(sessionId, CapabilitiesBitmap::kWiFiPAF, ctx.fabricIndex);

    ChipLogProgress(AppServer, "ProxyConnectRequest: WiFiPAF connected, proxy session %u (disc %u peer_id %u)", sessionId,
                    ctx.discriminator, pPafInfo->peer_id);

    CHIP_ERROR stateErr = ctx.cluster->SetCPState(CommissioningProxyCluster::kState_CPConnected);
    if (stateErr != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "OnConnectSuccess: SetCPState failed: %" CHIP_ERROR_FORMAT, stateErr.Format());
    }

    Commands::ProxyConnectResponse::Type response;
    response.sessionID = sessionId;
    cmd->AddResponse(ctx.path, response);
}

void CommissioningProxyPafTransport::HandleConnectError(void * context, CHIP_ERROR err)
{
    auto * self = static_cast<CommissioningProxyPafTransport *>(context);
    VerifyOrReturn(self != nullptr);

    if (!self->mPendingConnect.has_value())
    {
        ChipLogProgress(AppServer,
                        "OnConnectError: ignoring stale callback after successful connect "
                        "(err: %" CHIP_ERROR_FORMAT ")",
                        err.Format());
        return;
    }
    ChipLogError(AppServer, "ProxyConnectRequest: WiFiPAF connect failed: %" CHIP_ERROR_FORMAT, err.Format());
    self->FailPendingConnect(Status::Failure, /*cancelTimer=*/true);
}

// ==================================================================
// CommissioningProxyTransport
// ==================================================================

void CommissioningProxyPafTransport::SetHost(CommissioningProxyCluster * host)
{
    mHost = host;

    // Teardown passes nullptr, and Shutdown() has already dropped the handler by then.
    VerifyOrReturn(host != nullptr);

    // A proxy publishes over NAN so it can be commissioned onto a fabric itself. That
    // publish receive handler has to go once it is commissioned, or a later subscribe
    // leaves the platform with two handlers for the same traffic. With no fabric table
    // there is nothing to watch, which is how the unit tests run.
    VerifyOrReturn(mFabricTable != nullptr);

    if (mFabricTable->FabricCount() > 0)
    {
        // Already commissioned, so it will never publish again.
        mAdapter.DisconnectPublishReceiveHandler();
        return;
    }

    LogErrorOnFailure(DeviceLayer::PlatformMgr().AddEventHandler(OnDeviceEvent, reinterpret_cast<intptr_t>(this)));
    mPublishHandlerArmed = true;
}

void CommissioningProxyPafTransport::OnDeviceEvent(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    VerifyOrReturn(event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete);

    auto * self = reinterpret_cast<CommissioningProxyPafTransport *>(arg);
    VerifyOrReturn(self != nullptr);

    ChipLogProgress(AppServer, "CommissioningProxy: commissioning complete, disconnecting publish receive handler");
    self->mAdapter.DisconnectPublishReceiveHandler();
}

Status CommissioningProxyPafTransport::Connect(app::CommandHandler * commandObj, const DataModel::InvokeRequest & request,
                                               uint16_t discriminator, System::Clock::Seconds16 timeout)
{
    // Only one PAF connect can be in flight at a time. Reject a second one up front,
    // before pausing the background scan or touching the PAF session pool, so a refused
    // connect leaves no side effects behind.
    if (mPendingConnect.has_value())
    {
        ChipLogError(AppServer, "ProxyConnectRequest: a PAF connect is already in progress");
        return Status::Busy;
    }

    // Pause any background scan so the NAN subscribe slot can be reused for the PAF
    // connect subscribe; the registry resumes it on OnAllSessionsClosed().
    mBgScan.Pause();

    // The WiFiPAF layer is initialized once by the platform ConnectivityManager at
    // startup. Do NOT call Init() here: it memset()s the shared endpoint pool, which
    // would wipe a live PAFTP session while leaving its scheduled ack/retransmit timers
    // pointing at the zeroed slot.

    // Install the proxy receive delegate so messages arriving from the commissionee over
    // WiFiPAF are forwarded to the commissioner as ProxyMessageResponse rather than being
    // injected into the application's own Matter transport stack.
    mProxyDelegate.Install();

    // Register a PAF session so the platform's discovery result can locate the right
    // entry by discriminator.
    WiFiPAF::WiFiPAFSession pafSessionInfo{};
    pafSessionInfo.role          = WiFiPAF::kWiFiPafRole_Subscriber;
    pafSessionInfo.nodeId        = static_cast<NodeId>(discriminator);
    pafSessionInfo.discriminator = discriminator;
    CHIP_ERROR addErr =
        WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().AddPafSession(WiFiPAF::PafInfoAccess::kAccNodeInfo, pafSessionInfo);
    if (addErr != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: AddPafSession failed: %" CHIP_ERROR_FORMAT, addErr.Format());
        OnAllSessionsClosed();
        return (addErr == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED) ? Status::ResourceExhausted : Status::Failure;
    }

    // Per spec a Timeout of 0 indicates no timeout: the connect runs until it succeeds,
    // fails, or is cancelled via ProxyDisconnectRequest(null).
    const bool hasTimeout = (timeout.count() > 0);

    ConnectCtx & ctx  = mPendingConnect.emplace();
    ctx.handle        = app::CommandHandler::Handle(commandObj);
    ctx.path          = request.path;
    ctx.discriminator = discriminator;
    ctx.subscribeId   = 0;
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

    CHIP_ERROR err = DeviceLayer::ConnectivityMgr().WiFiPAFSubscribe(discriminator, this, HandleConnectSuccess, HandleConnectError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyConnectRequest: WiFiPAFSubscribe failed: %" CHIP_ERROR_FORMAT, err.Format());
        CHIP_ERROR rmErr =
            WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(WiFiPAF::PafInfoAccess::kAccNodeInfo, pafSessionInfo);
        if (rmErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "ProxyConnectRequest cleanup: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr.Format());
        }
        mPendingConnect.reset();
        OnAllSessionsClosed();
        return Status::Failure;
    }

    // WiFiPAFSubscribe does not return the id the platform assigned, so recover it from
    // the adapter; the transport needs it to cancel this specific subscribe.
    mPendingConnect->subscribeId = mAdapter.PendingConnectSubscribeId();

    if (hasTimeout)
    {
        CHIP_ERROR timerErr = mTimerDelegate.StartTimer(&mConnectTimer, timeout);
        if (timerErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ProxyConnectRequest: StartTimer failed: %" CHIP_ERROR_FORMAT, timerErr.Format());
            CHIP_ERROR rmErr2 =
                WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(WiFiPAF::PafInfoAccess::kAccNodeInfo, pafSessionInfo);
            if (rmErr2 != CHIP_NO_ERROR)
            {
                ChipLogDetail(AppServer, "ProxyConnectRequest cleanup: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr2.Format());
            }
            if (mPendingConnect->subscribeId != 0)
            {
                (void) DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(mPendingConnect->subscribeId);
            }
            mPendingConnect.reset();
            OnAllSessionsClosed();
            return Status::Failure;
        }
    }

    ChipLogProgress(AppServer, "ProxyConnectRequest: WiFiPAFSubscribe started for discriminator %u (subscribe_id %u)",
                    discriminator, mPendingConnect->subscribeId);

    return Status::Success;
}

Status CommissioningProxyPafTransport::CancelPendingConnect(FabricIndex fabricIndex)
{
    if (!mPendingConnect.has_value())
    {
        return Status::InvalidInState;
    }

    if (fabricIndex != mPendingConnect->fabricIndex)
    {
        ChipLogProgress(AppServer, "CancelPendingConnect: pending PAF connect owned by fabric %u, rejected fabric %u",
                        mPendingConnect->fabricIndex, fabricIndex);
        return Status::NotFound;
    }

    ChipLogProgress(AppServer, "CancelPendingConnect: cancelling pending PAF connect for fabric %u", fabricIndex);
    FailPendingConnect(Status::Failure, /*cancelTimer=*/true);

    return Status::Success;
}

Status CommissioningProxyPafTransport::Disconnect(uint16_t sessionId)
{
    SessionSlot * slot = FindSlot(sessionId);
    if (slot == nullptr)
    {
        return Status::NotFound;
    }

    WiFiPAF::WiFiPAFSession pafSession = slot->pafSession;
    *slot                              = SessionSlot{};

    CHIP_ERROR rmErr = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(WiFiPAF::PafInfoAccess::kAccSessionId, pafSession);
    if (rmErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "ProxyDisconnectRequest: RmPafSession for session %u: %" CHIP_ERROR_FORMAT, sessionId,
                      rmErr.Format());
    }

    // Explicitly close the PAFTP endpoint so its ack-received timer does not fire 30 s
    // later.
    WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().CloseEndPoint(pafSession);

    // Terminate the NAN subscribe instance that backed this session. For a subscriber
    // session the WiFiPAFSession id IS the wpa_supplicant subscribe_id, so cancelling it
    // tears down the PAFTP session. Without this the subscribe is left registered in the
    // long-lived wpa_supplicant and keeps firing discovery callbacks for the rest of the
    // proxy's lifetime.
    if (pafSession.id != 0)
    {
        CHIP_ERROR cancelErr = DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(pafSession.id);
        if (cancelErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "ProxyDisconnectRequest: WiFiPAFCancelSubscribe(%u): %" CHIP_ERROR_FORMAT, pafSession.id,
                          cancelErr.Format());
        }
    }

    return Status::Success;
}

CHIP_ERROR CommissioningProxyPafTransport::SendMessage(uint16_t sessionId, System::PacketBufferHandle && buf)
{
    SessionSlot * slot = FindSlot(sessionId);
    if (slot == nullptr)
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    // MUST use WiFiPAFLayer::SendMessage (not the platform's raw WiFiPAFSend):
    // WiFiPAFSend is the low-level NAN transmit used internally by PAFTP; calling it
    // directly bypasses PAFTP framing and the end device cannot parse the frame.
    // SendMessage goes through the PAFTP endpoint, which fragments and frames the
    // message before calling WiFiPAFSend.
    return WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().SendMessage(slot->pafSession, std::move(buf));
}

Status CommissioningProxyPafTransport::Scan(System::Clock::Seconds16 scanMaxTime)
{
    if (mScanInProgress)
    {
        ChipLogProgress(AppServer, "ProxyScanRequest: PAF scan already in progress — returning Busy");
        return Status::Busy;
    }

    // The one-shot scan and the background scan share the single NAN subscribe slot.
    // Pause any running background scan so the scan below does not fail with BUSY; the
    // scan-done callback resumes it when the foreground scan completes.
    mBgScan.Pause();

    mScanResultCount = 0;
    mScanInProgress  = true;

    // The platform owns the scan window here, so unlike BLE there is no timer to arm:
    // HandleForegroundScanDone closes the scan out.
    CHIP_ERROR err = mAdapter.StartForegroundScan(scanMaxTime, HandleForegroundScanResult, HandleForegroundScanDone, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Paf::Scan: StartForegroundScan failed: %" CHIP_ERROR_FORMAT, err.Format());
        mScanInProgress = false;
        OnAllSessionsClosed();
        return Status::Failure;
    }

    ChipLogProgress(AppServer, "Paf::Scan: started, scanMaxTime=%us", scanMaxTime.count());
    return Status::Success;
}

Status CommissioningProxyPafTransport::BgScanStart(System::Clock::Seconds16 timeout, BitMask<WiFiBandBitmap> wiFiBands,
                                                   FabricIndex fabricIndex, NodeId nodeId)
{
    // This driver services the kWiFiPAF transport. The registry starts or defers the
    // hardware scan (StartHardwareScan reports BUSY while a ProxyConnect holds the single
    // NAN subscribe slot) and records the requested bands per fabric.
    return mBgScan.Start(fabricIndex, nodeId, BitMask<CapabilitiesBitmap>(CapabilitiesBitmap::kWiFiPAF), wiFiBands, timeout);
}

Status CommissioningProxyPafTransport::BgScanStop(BitMask<CapabilitiesBitmap> transport, BitMask<WiFiBandBitmap> wiFiBands,
                                                  FabricIndex fabricIndex, NodeId nodeId)
{
    // The registry applies the spec transport/band overlap semantics (NOT_FOUND for an
    // unknown fabric, SUCCESS for a non-overlapping request, keep-scanning while any
    // fabric still covers the transport, stop+clear on the last fabric).
    return mBgScan.Stop(fabricIndex, nodeId, transport, wiFiBands);
}

void CommissioningProxyPafTransport::OnFabricRemoved(FabricIndex fabricIndex)
{
    // The cluster has already dropped the fabric's sessions; this drops its background
    // scans so nothing survives into a reused FabricIndex.
    mBgScan.RemoveFabric(fabricIndex);
}

void CommissioningProxyPafTransport::OnAllSessionsClosed()
{
    if (mBgScan.IsPaused() && !mBgScan.IsEmpty())
    {
        // Zero delay: still dispatched from the event loop rather than inline, but
        // cancellable in Shutdown() so it cannot fire against a destroyed transport. The
        // registry re-checks whether a connect is now pending and stays paused if so.
        mTimerDelegate.CancelTimer(&mResumeBgScanTimer);
        CHIP_ERROR err = mTimerDelegate.StartTimer(&mResumeBgScanTimer, System::Clock::Timeout(0));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "OnAllSessionsClosed: could not defer bg-scan resume: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
}

bool CommissioningProxyPafTransport::IsConnectPending() const
{
    return mPendingConnect.has_value();
}

void CommissioningProxyPafTransport::Shutdown()
{
    if (mPublishHandlerArmed)
    {
        DeviceLayer::PlatformMgr().RemoveEventHandler(OnDeviceEvent, reinterpret_cast<intptr_t>(this));
        mPublishHandlerArmed = false;
    }

    // Close any active PAF sessions: fail in-flight messages, close PAFTP endpoints,
    // cancel NAN subscribes, and remove them from the cluster's session manager. Take the
    // sessions out first so the slots are free if a close fires a callback synchronously.
    WiFiPAF::WiFiPAFSession closing[kMaxSessions] = {};
    uint16_t closingIds[kMaxSessions]             = {};
    size_t closingCount                           = 0;
    for (auto & slot : mSessions)
    {
        if (slot.inUse)
        {
            closingIds[closingCount] = slot.sessionId;
            closing[closingCount]    = slot.pafSession;
            closingCount++;
        }
        slot = SessionSlot{};
    }
    for (size_t i = 0; i < closingCount; i++)
    {
        if (mHost != nullptr)
        {
            mHost->Sessions().DispatchMessageFailure(closingIds[i], Status::Failure);
            mHost->Sessions().RemoveSession(closingIds[i]);
        }
        (void) WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(WiFiPAF::PafInfoAccess::kAccSessionId, closing[i]);
        WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().CloseEndPoint(closing[i]);
        if (closing[i].id != 0)
        {
            (void) DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(closing[i].id);
        }
    }

    // Detach an in-flight foreground scan so its completion cannot reach this object
    // after it is gone; the platform may keep scanning, but nothing is reported.
    mAdapter.StopForegroundScan();
    mScanInProgress  = false;
    mScanResultCount = 0;

    // Drop the deferred bg-scan resume, or it fires against a destroyed transport.
    mTimerDelegate.CancelTimer(&mResumeBgScanTimer);

    // Tear down an in-flight ProxyConnect: cancel its timeout timer and the NAN
    // subscribe, and drop the PAF session, so nothing outlives the cluster. Do not call
    // FailPendingConnect() here — it would invoke OnAllSessionsClosed(), which would
    // schedule a scan restart just as the cleanup below stops everything.
    if (mPendingConnect.has_value())
    {
        mTimerDelegate.CancelTimer(&mConnectTimer);

        ConnectCtx ctx = std::move(*mPendingConnect);
        mPendingConnect.reset();

        (void) DeviceLayer::ConnectivityMgr().WiFiPAFCancelIncompleteSubscribe();
        if (ctx.subscribeId != 0)
        {
            (void) DeviceLayer::ConnectivityMgr().WiFiPAFCancelSubscribe(ctx.subscribeId);
        }

        WiFiPAF::WiFiPAFSession keyInfo{};
        keyInfo.nodeId        = static_cast<NodeId>(ctx.discriminator);
        keyInfo.discriminator = ctx.discriminator;
        (void) WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().RmPafSession(WiFiPAF::PafInfoAccess::kAccNodeInfo, keyInfo);

        if (app::CommandHandler * cmd = ctx.handle.Get())
        {
            cmd->AddStatus(ctx.path, Status::Failure);
        }
    }

    // Tear down the background scan: cancel every per-fabric lifetime timer and stop the
    // hardware scan if the registry currently owns it.
    mBgScan.Shutdown();

    // Restore WiFiPAFLayer::mWiFiPAFTransport so a later PAF event reaches the
    // application's own delegate rather than this object after the cluster is gone.
    mProxyDelegate.Uninstall();

    // Last: the cluster is going away, so drop the pointer to it. Every callback above
    // null-checks mHost.
    mHost = nullptr;
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
