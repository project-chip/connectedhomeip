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
#include <wifipaf/WiFiPAFRole.h>

using namespace chip;
using namespace chip::app;
using namespace Clusters::CommissioningProxy;

namespace {

using ScanResultT = chip::app::Clusters::CommissioningProxy::Structs::ScanResultStruct::Type;

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
static uint16_t sNextProxySessionId = 1;

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

    cmd->AddResponse(ctx->path, response);
    cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Success);

    delete ctx;
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
    ChipLogProgress(AppServer, "===SHM %s(), transport:%d wiFiBand:%d timeout:%u",
                    __func__, (int)transport, (int)wiFiBand, timeout);

    // Allocate a new proxy session ID.
    uint16_t sessionId = sNextProxySessionId++;

    // Populate a WiFiPAF session info record.
    // The MAC address is provided by the 'address' field when the device was
    // previously discovered via ProxyScanRequest.  If not provided, leave zero.
    ProxySessionInfo info{};
    info.pafSession.role          = chip::WiFiPAF::kWiFiPafRole_Subscriber;
    info.pafSession.id            = sessionId;
    info.pafSession.discriminator = discriminator;
    info.pafSession.nodeId        = chip::kUndefinedNodeId;
    memset(info.pafSession.peer_addr, 0, sizeof(info.pafSession.peer_addr));

    if (!address.IsNull() && address.Value().size() == sizeof(info.pafSession.peer_addr))
    {
        memcpy(info.pafSession.peer_addr, address.Value().data(), sizeof(info.pafSession.peer_addr));
    }

    sProxySessions[sessionId] = std::move(info);

    ChipLogProgress(AppServer, "===SHM %s(): allocated proxy session %u for discriminator %u",
                    __func__, sessionId, discriminator);

    // TODO: initiate the actual WiFiPAF transport connection here, honouring
    //       the Timeout field per spec [10.124].  For now we assume the
    //       connection was already established by the scan/connect flow and
    //       the peer MAC is known.

    // Send ProxyConnectResponse with the allocated sessionId.
    Commands::ProxyConnectResponse::Type response;
    response.sessionId = sessionId;
    commandObj->AddResponse(request.path, response);

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
        commandObj->AddStatus(request.path, chip::Protocols::InteractionModel::Status::Failure);
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

    if (!message.HasValue() || message.Value().empty())
    {
        // Nothing to forward.
        commandObj->AddStatus(request.path, chip::Protocols::InteractionModel::Status::Success);
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
    // TODO: The receive path (commissionee → proxy → chip-tool) requires
    //       intercepting WiFiPAF incoming packets for this session and sending
    //       them back as ProxyMessageResponse.  See the WiFiPAF platform receive
    //       callback (ConnectivityManagerImpl) for the integration point.
    CHIP_ERROR err = chip::DeviceLayer::ConnectivityMgrImpl().WiFiPAFSend(
        it->second.pafSession, std::move(buf));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyMessageRequest: WiFiPAFSend failed: %" CHIP_ERROR_FORMAT, err.Format());
        sPendingProxyMsgCtx.erase(sessionId);
        delete ctx;
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    // Response will be sent asynchronously via OnProxyWiFiPAFMessageReceived()
    // when the commissionee's reply arrives.
    return chip::Protocols::InteractionModel::Status::Success;
}

// Called by the platform layer when a WiFiPAF packet arrives for a proxy session.
// Sends the raw bytes back to chip-tool as ProxyMessageResponse.
void OnProxyWiFiPAFMessageReceived(uint16_t sessionId, const uint8_t * data, size_t length)
{
    auto it = sPendingProxyMsgCtx.find(sessionId);
    if (it == sPendingProxyMsgCtx.end())
    {
        ChipLogDetail(AppServer, "OnProxyWiFiPAFMessageReceived: no pending ctx for session %u", sessionId);
        return;
    }

    ProxyMsgCtx * ctx = it->second;
    sPendingProxyMsgCtx.erase(it);

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd == nullptr)
    {
        delete ctx;
        return;
    }

    chip::app::Clusters::CommissioningProxy::Commands::ProxyMessageResponse::Type response;
    response.sessionId = sessionId;
    response.message.SetNonNull(chip::ByteSpan(data, length));
    cmd->AddResponse(ctx->path, response);

    delete ctx;
}
