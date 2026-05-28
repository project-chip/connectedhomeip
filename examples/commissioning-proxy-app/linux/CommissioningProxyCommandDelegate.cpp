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
#include "commissioning-proxy-delegate-impl.h"

#include "CommissioningProxyDispatcher.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyDelegate.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <ble/BleConfig.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/PlatformManager.h>

#include <map>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#include "CommissioningProxyPafTransport.h"
#endif

using namespace chip;
using namespace chip::app;
using namespace Clusters::CommissioningProxy;

namespace {

// Maximum simultaneous proxy sessions (spec §10.5.6.3 MaxSessions).
// This implementation supports one device at a time across all transports.
constexpr uint8_t kMaxProxySessions = 1;

// ------------------------------------------------------------------
// Proxy session tracking (transport-agnostic)
//
// Maps a proxy sessionId to its owning transport + fabric.  Per-transport
// state (PAF: WiFiPAFSession; BLE: BLEEndPoint *) lives in the corresponding
// per-transport module's own map.
// ------------------------------------------------------------------

struct ProxySessionInfo
{
    CapabilitiesBitmap transport{};
    chip::FabricIndex fabricIndex = chip::kUndefinedFabricIndex;
};

static std::map<uint16_t, ProxySessionInfo> sProxySessions;

// Session ID counter.  Starts at 1; 0 is never a valid session ID.
static uint16_t sNextProxySessionId = 1;

// ------------------------------------------------------------------
// ProxyMessageRequest response routing
//
// One in-flight ProxyMessageRequest per session: keeps the IM exchange open
// until the commissionee replies (over PAF or BLE) and the per-transport
// module hands the bytes back via ProxyDispatcher::DispatchMessageResponse.
// ------------------------------------------------------------------

struct ProxyMsgCtx
{
    chip::app::CommandHandler::Handle handle;
    chip::app::ConcreteCommandPath path;
    uint16_t sessionId;
};

static std::map<uint16_t, ProxyMsgCtx *> sPendingProxyMsgCtx;

// Fired when the proxy's responseTimeout deadline elapses before the ED replies.
static void ProxyMessageResponseTimeoutCallback(chip::System::Layer *, void * appState)
{
    uint16_t sessionId = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(appState));
    auto it            = sPendingProxyMsgCtx.find(sessionId);
    if (it == sPendingProxyMsgCtx.end())
        return; // Already resolved (ED replied or session closed).
    ProxyMsgCtx * ctx = it->second;
    sPendingProxyMsgCtx.erase(it);
    ChipLogProgress(AppServer, "ProxyMessageRequest: responseTimeout expired for session %u", sessionId);
    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
        cmd->AddStatus(ctx->path, chip::Protocols::InteractionModel::Status::Failure);
    delete ctx;
}

static void CancelProxyMessageResponseTimer(uint16_t sessionId)
{
    chip::DeviceLayer::SystemLayer().CancelTimer(ProxyMessageResponseTimeoutCallback,
                                                 reinterpret_cast<void *>(static_cast<uintptr_t>(sessionId)));
}

} // namespace

// ==================================================================
// Dispatcher API — internal interface called by per-transport modules.
// ==================================================================
namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {
namespace ProxyDispatcher {

uint16_t AllocSessionId()
{
    uint16_t id;
    do
    {
        if (sNextProxySessionId == 0)
            sNextProxySessionId = 1;
        id = sNextProxySessionId++;
    } while (sProxySessions.count(id) > 0);
    return id;
}

void RegisterSession(uint16_t sessionId, CapabilitiesBitmap transport, chip::FabricIndex fabricIndex)
{
    sProxySessions[sessionId] = ProxySessionInfo{ transport, fabricIndex };
}

void RemoveSession(uint16_t sessionId)
{
    sProxySessions.erase(sessionId);
}

void DispatchMessageResponse(uint16_t sessionId, const uint8_t * data, size_t length)
{
    auto it = sPendingProxyMsgCtx.find(sessionId);
    if (it == sPendingProxyMsgCtx.end())
    {
        ChipLogDetail(AppServer,
                      "DispatchMessageResponse: no pending ctx for session %u — "
                      "commissionee sent unsolicited data; dropping",
                      sessionId);
        return;
    }

    ProxyMsgCtx * ctx = it->second;
    sPendingProxyMsgCtx.erase(it);
    CancelProxyMessageResponseTimer(sessionId);

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd == nullptr)
    {
        ChipLogDetail(AppServer, "DispatchMessageResponse: exchange for session %u already expired", sessionId);
        delete ctx;
        return;
    }

    chip::app::Clusters::CommissioningProxy::Commands::ProxyMessageResponse::Type response;
    response.sessionId = sessionId;
    response.message.SetNonNull(chip::ByteSpan(data, length));
    cmd->AddResponse(ctx->path, response);
    delete ctx;
}

void DispatchMessageFailure(uint16_t sessionId, chip::Protocols::InteractionModel::Status status)
{
    auto it = sPendingProxyMsgCtx.find(sessionId);
    if (it == sPendingProxyMsgCtx.end())
        return;

    ProxyMsgCtx * ctx = it->second;
    sPendingProxyMsgCtx.erase(it);
    CancelProxyMessageResponseTimer(sessionId);

    chip::app::CommandHandler * cmd = ctx->handle.Get();
    if (cmd != nullptr)
        cmd->AddStatus(ctx->path, status);
    delete ctx;
}

} // namespace ProxyDispatcher
} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip

// ==================================================================
// MyCPDelegate handlers — dispatch on transport to the per-transport module.
// ==================================================================

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyConnectRequest(
    DataModel::Nullable<chip::ByteSpan> address, CapabilitiesBitmap transport, uint16_t discriminator, chip::VendorId vendorid,
    uint16_t productid, uint16_t timeout, WiFiBandBitmap wiFiBand, app::CommandHandler * commandObj,
    const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "ProxyConnectRequest transport:%d wiFiBand:%d timeout:%u discriminator:%u", (int) transport,
                    (int) wiFiBand, timeout, discriminator);

    const uint8_t tbits = static_cast<uint8_t>(transport);

    if (tbits == static_cast<uint8_t>(CapabilitiesBitmap::kWiFiPAF))
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        return Paf::Connect(commandObj, request, discriminator, timeout, mServer);
#else
        return chip::Protocols::InteractionModel::Status::InvalidTransportType;
#endif
    }

    if (tbits == static_cast<uint8_t>(CapabilitiesBitmap::kBle))
    {
#if CONFIG_NETWORK_LAYER_BLE
        // TODO(Phase 2B): forward to Ble::Connect.
        ChipLogError(AppServer, "ProxyConnectRequest: BLE transport not yet implemented (Phase 2B)");
        return chip::Protocols::InteractionModel::Status::Failure;
#else
        return chip::Protocols::InteractionModel::Status::InvalidTransportType;
#endif
    }

    ChipLogError(AppServer, "ProxyConnectRequest: unsupported transport 0x%x", tbits);
    return chip::Protocols::InteractionModel::Status::InvalidTransportType;
}

Protocols::InteractionModel::Status
Clusters::CommissioningProxy::MyCPDelegate::ProxyScanRequest(CapabilitiesBitmap transport, WiFiBandBitmap wiFiBands,
                                                             app::CommandHandler * commandObj,
                                                             const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "ProxyScanRequest: transport:0x%x wiFiBands:0x%x", (int) transport, (int) wiFiBands);

    const uint8_t tbits = static_cast<uint8_t>(transport);

    if (tbits == static_cast<uint8_t>(CapabilitiesBitmap::kWiFiPAF))
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        return Paf::Scan(commandObj, request, GetScanMaxTime());
#else
        return chip::Protocols::InteractionModel::Status::InvalidTransportType;
#endif
    }

    if (tbits == static_cast<uint8_t>(CapabilitiesBitmap::kBle))
    {
#if CONFIG_NETWORK_LAYER_BLE
        // TODO(Phase 2B): forward to Ble::Scan.
        ChipLogError(AppServer, "ProxyScanRequest: BLE transport not yet implemented (Phase 2B)");
        return chip::Protocols::InteractionModel::Status::Failure;
#else
        return chip::Protocols::InteractionModel::Status::InvalidTransportType;
#endif
    }

    ChipLogError(AppServer, "ProxyScanRequest: unsupported transport 0x%x", tbits);
    return chip::Protocols::InteractionModel::Status::InvalidTransportType;
}

Protocols::InteractionModel::Status
Clusters::CommissioningProxy::MyCPDelegate::ProxyMessageRequest(uint16_t sessionId, chip::Optional<chip::ByteSpan> message,
                                                                uint8_t responseTimeout, app::CommandHandler * commandObj,
                                                                const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "ProxyMessageRequest: sessionId=%u msgLen=%zu", sessionId,
                    message.HasValue() ? message.Value().size() : 0);

    auto it = sProxySessions.find(sessionId);
    if (it == sProxySessions.end())
    {
        ChipLogError(AppServer, "ProxyMessageRequest: unknown sessionId %u", sessionId);
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    if (request.subjectDescriptor.fabricIndex != it->second.fabricIndex)
    {
        ChipLogProgress(AppServer, "ProxyMessageRequest: sessionId %u owned by fabric %u, rejected fabric %u", sessionId,
                        it->second.fabricIndex, request.subjectDescriptor.fabricIndex);
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    // Per spec reject if another ProxyMessageRequest for this session
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
        ChipLogProgress(AppServer, "ProxyMessageRequest: cleaning up expired pending ctx for session %u", sessionId);
        CancelProxyMessageResponseTimer(sessionId);
        delete existing;
        sPendingProxyMsgCtx.erase(pendingIt);
    }

    if (!message.HasValue() || message.Value().empty())
    {
        // Per spec §10.161: null/empty message means the Commissioner is polling
        // for a queued response from the Commissionee.  Respond immediately.
        chip::app::Clusters::CommissioningProxy::Commands::ProxyMessageResponse::Type pollResponse;
        pollResponse.sessionId = sessionId;
        pollResponse.message.SetNull();
        commandObj->AddResponse(request.path, pollResponse);
        return chip::Protocols::InteractionModel::Status::Success;
    }

    chip::System::PacketBufferHandle buf =
        chip::System::PacketBufferHandle::NewWithData(message.Value().data(), message.Value().size());
    if (buf.IsNull())
    {
        ChipLogError(AppServer, "ProxyMessageRequest: out of memory");
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    const uint8_t sessTransportBits = static_cast<uint8_t>(it->second.transport);

    // Per spec ResponseTimeout=0: forward best-effort; respond immediately.
    if (responseTimeout == 0)
    {
        CHIP_ERROR sendErr = CHIP_ERROR_INCORRECT_STATE;
        if (sessTransportBits == static_cast<uint8_t>(CapabilitiesBitmap::kWiFiPAF))
        {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
            sendErr = Paf::SendMessage(sessionId, std::move(buf));
#endif
        }
        else if (sessTransportBits == static_cast<uint8_t>(CapabilitiesBitmap::kBle))
        {
#if CONFIG_NETWORK_LAYER_BLE
            // TODO(Phase 2B): sendErr = Ble::SendMessage(sessionId, std::move(buf));
            sendErr = CHIP_ERROR_NOT_IMPLEMENTED;
#endif
        }
        if (sendErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "ProxyMessageRequest(ResponseTimeout=0): SendMessage failed: %" CHIP_ERROR_FORMAT,
                          sendErr.Format());
        }
        chip::app::Clusters::CommissioningProxy::Commands::ProxyMessageResponse::Type immediateResp;
        immediateResp.sessionId = sessionId;
        immediateResp.message.SetNull();
        commandObj->AddResponse(request.path, immediateResp);
        return chip::Protocols::InteractionModel::Status::Success;
    }

    // Keep the invoke handler alive until the commissionee replies.
    auto * ctx                     = new ProxyMsgCtx{ chip::app::CommandHandler::Handle(commandObj), request.path, sessionId };
    sPendingProxyMsgCtx[sessionId] = ctx;

    if (auto * exchange = commandObj->GetExchangeContext())
    {
        exchange->SetResponseTimeout(chip::System::Clock::Seconds16(responseTimeout));
    }

    CHIP_ERROR err = CHIP_ERROR_INCORRECT_STATE;
    if (sessTransportBits == static_cast<uint8_t>(CapabilitiesBitmap::kWiFiPAF))
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        err = Paf::SendMessage(sessionId, std::move(buf));
#endif
    }
    else if (sessTransportBits == static_cast<uint8_t>(CapabilitiesBitmap::kBle))
    {
#if CONFIG_NETWORK_LAYER_BLE
        // TODO(Phase 2B): err = Ble::SendMessage(sessionId, std::move(buf));
        err = CHIP_ERROR_NOT_IMPLEMENTED;
#endif
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ProxyMessageRequest: SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
        sPendingProxyMsgCtx.erase(sessionId);
        delete ctx;
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    // Start a hard deadline timer.  If the ED does not reply within
    // responseTimeout seconds, ProxyMessageResponseTimeoutCallback fires and
    // returns Failure to the commissioner.
    CHIP_ERROR timerErr = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(responseTimeout),
                                                                      ProxyMessageResponseTimeoutCallback,
                                                                      reinterpret_cast<void *>(static_cast<uintptr_t>(sessionId)));
    if (timerErr != CHIP_NO_ERROR)
        ChipLogError(AppServer, "ProxyMessageRequest: failed to start response timer: %" CHIP_ERROR_FORMAT, timerErr.Format());

    return chip::Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
Clusters::CommissioningProxy::MyCPDelegate::ProxyDisconnectRequest(uint16_t sessionId, chip::FabricIndex fabricIndex)
{
    ChipLogProgress(AppServer, "ProxyDisconnectRequest: sessionId=%u fabric=%u", sessionId, fabricIndex);

    auto it = sProxySessions.find(sessionId);
    if (it == sProxySessions.end())
    {
        ChipLogError(AppServer, "ProxyDisconnectRequest: unknown sessionId %u", sessionId);
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    if (fabricIndex != it->second.fabricIndex)
    {
        ChipLogProgress(AppServer, "ProxyDisconnectRequest: sessionId %u owned by fabric %u, rejected fabric %u", sessionId,
                        it->second.fabricIndex, fabricIndex);
        return chip::Protocols::InteractionModel::Status::NotFound;
    }

    // Release any outstanding ProxyMessageRequest context for this session.
    // Do NOT call AddStatus() here: if the commissioner has already closed the
    // TCP connection, AddStatus() triggers synchronous session teardown which
    // evicts the ProxyDisconnectRequest exchange before this handler returns.
    auto pendingIt = sPendingProxyMsgCtx.find(sessionId);
    if (pendingIt != sPendingProxyMsgCtx.end())
    {
        CancelProxyMessageResponseTimer(sessionId);
        delete pendingIt->second;
        sPendingProxyMsgCtx.erase(pendingIt);
    }

    const uint8_t sessTransportBits = static_cast<uint8_t>(it->second.transport);
    sProxySessions.erase(it);

    if (sessTransportBits == static_cast<uint8_t>(CapabilitiesBitmap::kWiFiPAF))
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        Paf::Disconnect(sessionId);
        if (sProxySessions.empty())
            Paf::OnAllSessionsClosed();
#endif
    }
    else if (sessTransportBits == static_cast<uint8_t>(CapabilitiesBitmap::kBle))
    {
#if CONFIG_NETWORK_LAYER_BLE
        // TODO(Phase 2B): Ble::Disconnect(sessionId).
        ChipLogError(AppServer, "ProxyDisconnectRequest: BLE disconnect not yet implemented (Phase 2B)");
#endif
    }

    ChipLogProgress(AppServer, "ProxyDisconnectRequest: session %u cleaned up", sessionId);
    return chip::Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::CancelPendingConnect(chip::FabricIndex fabricIndex)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    auto s = Paf::CancelPendingConnect(fabricIndex);
    if (s != chip::Protocols::InteractionModel::Status::InvalidInState)
        return s;
#endif
#if CONFIG_NETWORK_LAYER_BLE
    // TODO(Phase 2B): Ble::CancelPendingConnect(fabricIndex).
#endif
    return chip::Protocols::InteractionModel::Status::InvalidInState;
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyBackgroundScanStartRequest(
    CapabilitiesBitmap transport, uint16_t timeout, WiFiBandBitmap wiFiBands, chip::FabricIndex fabricIndex, chip::NodeId nodeId,
    app::CommandHandler * commandObj, const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "ProxyBackgroundScanStartRequest transport:%u timeout:%u fabricIndex:%u nodeId:0x" ChipLogFormatX64,
                    static_cast<uint8_t>(transport), timeout, fabricIndex, ChipLogValueX64(nodeId));

    const uint8_t tbits = static_cast<uint8_t>(transport);
    const bool needsPaf = (tbits & static_cast<uint8_t>(CapabilitiesBitmap::kWiFiPAF)) != 0;
    const bool needsBle = (tbits & static_cast<uint8_t>(CapabilitiesBitmap::kBle)) != 0;

    if (needsBle && !needsPaf)
    {
#if CONFIG_NETWORK_LAYER_BLE
        // TODO(Phase 2B): forward to Ble::BgScanStart.
        ChipLogError(AppServer, "ProxyBackgroundScanStartRequest: BLE transport not yet implemented (Phase 2B)");
        return chip::Protocols::InteractionModel::Status::Failure;
#else
        return chip::Protocols::InteractionModel::Status::InvalidTransportType;
#endif
    }

    if (needsPaf)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        return Paf::BgScanStart(transport, timeout, wiFiBands, fabricIndex, nodeId, mServer);
#else
        return chip::Protocols::InteractionModel::Status::InvalidTransportType;
#endif
    }

    return chip::Protocols::InteractionModel::Status::InvalidTransportType;
}

Protocols::InteractionModel::Status
Clusters::CommissioningProxy::MyCPDelegate::ProxyBackgroundScanStopRequest(CapabilitiesBitmap transport, WiFiBandBitmap wiFiBands,
                                                                           chip::FabricIndex fabricIndex, chip::NodeId nodeId)
{
    ChipLogProgress(AppServer,
                    "ProxyBackgroundScanStopRequest transport:0x%x wiFiBands:0x%x fabricIndex:%u nodeId:0x" ChipLogFormatX64,
                    static_cast<uint8_t>(transport), static_cast<uint16_t>(wiFiBands), fabricIndex, ChipLogValueX64(nodeId));

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    auto s = Paf::BgScanStop(transport, wiFiBands, fabricIndex, nodeId);
    if (s != chip::Protocols::InteractionModel::Status::NotFound)
        return s;
#endif
#if CONFIG_NETWORK_LAYER_BLE
    // TODO(Phase 2B): forward to Ble::BgScanStop.
#endif
    return chip::Protocols::InteractionModel::Status::NotFound;
}

uint8_t Clusters::CommissioningProxy::MyCPDelegate::GetMaxSessions()
{
    return kMaxProxySessions;
}

uint8_t Clusters::CommissioningProxy::MyCPDelegate::GetActiveSessionCount()
{
    // Count established sessions AND any in-flight connect per transport so
    // the cluster's MaxSessions gate covers both established and pending paths.
    uint8_t count = static_cast<uint8_t>(sProxySessions.size());
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    if (Paf::IsConnectPending())
        ++count;
#endif
#if CONFIG_NETWORK_LAYER_BLE
    // TODO(Phase 2B): if (Ble::IsConnectPending()) ++count;
#endif
    return count;
}

uint8_t Clusters::CommissioningProxy::MyCPDelegate::GetNumCachedResults()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    return Paf::GetNumCachedResults();
#else
    return 0;
#endif
}

CHIP_ERROR Clusters::CommissioningProxy::MyCPDelegate::EncodeCachedResults(app::AttributeValueEncoder & encoder)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    return Paf::EncodeCachedResults(encoder);
#else
    chip::app::DataModel::Nullable<chip::app::DataModel::List<
        const chip::app::Clusters::CommissioningProxy::Structs::ScanResultStruct::Type>>
        nullValue;
    return encoder.Encode(nullValue);
#endif
}
