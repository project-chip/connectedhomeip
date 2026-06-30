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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyDelegate.h>
#include <app/server/Server.h>
#include <ble/BleConfig.h>
#include <clusters/CommissioningProxy/Commands.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/PlatformManager.h>

#include "CommissioningProxyBgScanCache.h"

#include <deque>
#include <map>
#include <vector>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#include "CommissioningProxyPafTransport.h"
#endif
#if CONFIG_NETWORK_LAYER_BLE
#include "CommissioningProxyBleTransport.h"
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

// ------------------------------------------------------------------
// ProxyScanRequest aggregation across transports
//
// A single ProxyScanRequest may target multiple transports.  Each transport
// module scans independently and reports its results via
// ProxyDispatcher::ContributeScanResults; this aggregator owns the command
// handle and emits one combined ProxyScanResponse once the last started
// sub-scan reports.  Only one aggregate scan runs at a time.
// ------------------------------------------------------------------

using ScanResultEntry = Structs::ScanResultStruct::Type;

// The per-transport modules build transient ScanResultStruct values whose
// ByteSpans dangle after ContributeScanResults returns, so the aggregator owns
// the backing bytes.  std::deque keeps element addresses stable across growth,
// so the ScanResultStruct values in `results` can hold spans straight into
// addressStore/extStore — no second rebuild at emit time.
struct ScanAggregator
{
    chip::app::CommandHandler::Handle handle;
    chip::app::ConcreteCommandPath path;
    uint8_t pending = 0;
    std::deque<std::vector<uint8_t>> addressStore;
    std::deque<std::vector<uint8_t>> extStore;
    std::vector<ScanResultEntry> results;
};

static ScanAggregator * sScanAgg = nullptr;

// Fallback so a sub-scan whose completion callback never fires cannot wedge the
// aggregator (and thus every future ProxyScanRequest) permanently.
constexpr uint16_t kScanWatchdogMarginSecs = 5;
static void OnScanWatchdog(chip::System::Layer *, void *);

static void EmitCombinedScanResponse()
{
    chip::DeviceLayer::SystemLayer().CancelTimer(OnScanWatchdog, nullptr);

    auto * agg = sScanAgg;
    sScanAgg   = nullptr;
    if (agg == nullptr)
        return;

    chip::app::CommandHandler * cmd = agg->handle.Get();
    if (cmd != nullptr)
    {
        Commands::ProxyScanResponse::Type response;
        response.proxyScanResult = chip::app::DataModel::List<const ScanResultEntry>(
            chip::Span<const ScanResultEntry>(agg->results.data(), agg->results.size()));
        response.numberOfResults = static_cast<uint8_t>(agg->results.size());
        cmd->AddResponse(agg->path, response);
        ChipLogProgress(AppServer, "ProxyScanRequest: combined scan complete, %u result(s)",
                        static_cast<unsigned>(agg->results.size()));
    }
    delete agg;
}

static void OnScanWatchdog(chip::System::Layer *, void *)
{
    if (sScanAgg != nullptr)
    {
        ChipLogError(AppServer, "ProxyScanRequest: watchdog fired (a sub-scan never completed); emitting partial results");
        EmitCombinedScanResponse();
    }
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
    response.sessionID = sessionId;
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

void ContributeScanResults(chip::Span<const ScanResultEntry> results)
{
    if (sScanAgg == nullptr)
    {
        ChipLogError(AppServer, "ContributeScanResults: no aggregate scan active; dropping %u result(s)",
                     static_cast<unsigned>(results.size()));
        return;
    }

    for (const auto & e : results)
    {
        // Copy scalar fields and (transient) spans, then rebind the address /
        // extendedData spans to point into the aggregator's stable storage so
        // they survive until the combined AddResponse.
        ScanResultEntry r = e;
        if (!e.address.IsNull())
        {
            auto span = e.address.Value();
            sScanAgg->addressStore.emplace_back(span.data(), span.data() + span.size());
            auto & addr = sScanAgg->addressStore.back();
            r.address.SetNonNull(chip::ByteSpan(addr.data(), addr.size()));
        }
        if (!e.extendedData.IsNull())
        {
            auto span = e.extendedData.Value();
            sScanAgg->extStore.emplace_back(span.data(), span.data() + span.size());
            auto & ext = sScanAgg->extStore.back();
            r.extendedData.SetNonNull(chip::ByteSpan(ext.data(), ext.size()));
        }
        sScanAgg->results.push_back(r);
    }

    if (sScanAgg->pending > 0 && --sScanAgg->pending == 0)
    {
        EmitCombinedScanResponse();
    }
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
    DataModel::Nullable<chip::ByteSpan> address, chip::BitMask<CapabilitiesBitmap> transportBits, uint16_t discriminator,
    chip::VendorId vendorid, uint16_t productid, uint16_t timeout, chip::BitMask<WiFiBandBitmap> wiFiBand,
    app::CommandHandler * commandObj, const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "ProxyConnectRequest transport:%d wiFiBand:%d timeout:%u discriminator:%u", transportBits.Raw(),
                    wiFiBand.Raw(), timeout, discriminator);

    // The cluster guarantees exactly one transport bit is set for a connect.
    const CapabilitiesBitmap transport = static_cast<CapabilitiesBitmap>(transportBits.Raw());

    switch (transport)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    case CapabilitiesBitmap::kWiFiPAF:
        return Paf::Connect(commandObj, request, discriminator, timeout, mServer);
#endif

#if CONFIG_NETWORK_LAYER_BLE
    case CapabilitiesBitmap::kBle:
        return Ble::Connect(commandObj, request, discriminator, timeout, mServer);
#endif

    default:
        ChipLogError(AppServer, "ProxyConnectRequest: unsupported transport 0x%x", static_cast<uint8_t>(transport));
        return chip::Protocols::InteractionModel::Status::InvalidTransportType;
    }
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyScanRequest(
    chip::BitMask<CapabilitiesBitmap> transport, chip::BitMask<WiFiBandBitmap> wiFiBands, app::CommandHandler * commandObj,
    const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "ProxyScanRequest: transport:0x%x wiFiBands:0x%x", transport.Raw(), wiFiBands.Raw());

    // A ProxyScanRequest MAY select multiple transports (CommissioningProxy
    // cluster spec, ProxyScanRequest Transport field).  Scan every requested
    // transport in parallel and aggregate the results into a single
    // ProxyScanResponse.  The cluster has already verified the request is
    // non-empty and that every requested bit is supported by this instance.
    if (sScanAgg != nullptr)
    {
        ChipLogProgress(AppServer, "ProxyScanRequest: an aggregate scan is already in progress — returning Busy");
        return chip::Protocols::InteractionModel::Status::Busy;
    }

    [[maybe_unused]] const chip::BitMask<CapabilitiesBitmap> requested(transport);
    const uint8_t scanMaxTime = GetScanMaxTime();

    auto * agg   = new ScanAggregator();
    agg->handle  = chip::app::CommandHandler::Handle(commandObj);
    agg->path    = request.path;
    agg->pending = 0;
    sScanAgg     = agg;

    // First non-success status from a sub-scan that could not be started; only
    // returned to the caller if no sub-scan starts at all.  Initialised to
    // Success so the first real failure (from either branch) is recorded.
    auto firstError = chip::Protocols::InteractionModel::Status::Success;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    if (requested.Has(CapabilitiesBitmap::kWiFiPAF))
    {
        auto s = Paf::Scan(scanMaxTime);
        if (s == chip::Protocols::InteractionModel::Status::Success)
            agg->pending++;
        else if (firstError == chip::Protocols::InteractionModel::Status::Success)
            firstError = s;
    }
#endif
#if CONFIG_NETWORK_LAYER_BLE
    if (requested.Has(CapabilitiesBitmap::kBle))
    {
        auto s = Ble::Scan(scanMaxTime);
        if (s == chip::Protocols::InteractionModel::Status::Success)
            agg->pending++;
        else if (firstError == chip::Protocols::InteractionModel::Status::Success)
            firstError = s;
    }
#endif

    if (agg->pending == 0)
    {
        ChipLogError(AppServer, "ProxyScanRequest: no requested transport scan could be started");
        sScanAgg = nullptr;
        delete agg;
        return (firstError == chip::Protocols::InteractionModel::Status::Success)
            ? chip::Protocols::InteractionModel::Status::Failure
            : firstError;
    }

    commandObj->FlushAcksRightAwayOnSlowCommand();

    // The dispatcher owns the exchange now; keep it alive for the full scan window.
    if (auto * exchange = commandObj->GetExchangeContext())
    {
        exchange->SetResponseTimeout(chip::System::Clock::Seconds16(static_cast<uint16_t>(scanMaxTime) + 5));
    }

    // Watchdog: force-emit if some started sub-scan never reports, so a stalled
    // transport cannot wedge sScanAgg (and every future ProxyScanRequest).
    CHIP_ERROR wdErr = chip::DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Seconds16(static_cast<uint16_t>(scanMaxTime) + kScanWatchdogMarginSecs), OnScanWatchdog, nullptr);
    if (wdErr != CHIP_NO_ERROR)
        ChipLogError(AppServer, "ProxyScanRequest: failed to arm scan watchdog: %" CHIP_ERROR_FORMAT, wdErr.Format());

    return chip::Protocols::InteractionModel::Status::Success;
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
        pollResponse.sessionID = sessionId;
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

    const CapabilitiesBitmap sessTransport = it->second.transport;

    // Per spec ResponseTimeout=0: forward best-effort; respond immediately.
    if (responseTimeout == 0)
    {
        CHIP_ERROR sendErr = CHIP_ERROR_INCORRECT_STATE;
        switch (sessTransport)
        {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        case CapabilitiesBitmap::kWiFiPAF:
            sendErr = Paf::SendMessage(sessionId, std::move(buf));
            break;
#endif
#if CONFIG_NETWORK_LAYER_BLE
        case CapabilitiesBitmap::kBle:
            sendErr = Ble::SendMessage(sessionId, std::move(buf));
            break;
#endif
        default:
            break;
        }
        if (sendErr != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "ProxyMessageRequest(ResponseTimeout=0): SendMessage failed: %" CHIP_ERROR_FORMAT,
                          sendErr.Format());
        }
        chip::app::Clusters::CommissioningProxy::Commands::ProxyMessageResponse::Type immediateResp;
        immediateResp.sessionID = sessionId;
        immediateResp.message.SetNull();
        commandObj->AddResponse(request.path, immediateResp);
        return chip::Protocols::InteractionModel::Status::Success;
    }

    // Keep the invoke handler alive until the commissionee replies.
    auto * ctx = new ProxyMsgCtx{ chip::app::CommandHandler::Handle(commandObj), request.path, sessionId };
    commandObj->FlushAcksRightAwayOnSlowCommand();
    sPendingProxyMsgCtx[sessionId] = ctx;

    if (auto * exchange = commandObj->GetExchangeContext())
    {
        exchange->SetResponseTimeout(chip::System::Clock::Seconds16(responseTimeout));
    }

    CHIP_ERROR err = CHIP_ERROR_INCORRECT_STATE;
    switch (sessTransport)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    case CapabilitiesBitmap::kWiFiPAF:
        err = Paf::SendMessage(sessionId, std::move(buf));
        break;
#endif
#if CONFIG_NETWORK_LAYER_BLE
    case CapabilitiesBitmap::kBle:
        err = Ble::SendMessage(sessionId, std::move(buf));
        break;
#endif
    default:
        break;
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

    const CapabilitiesBitmap sessTransport = it->second.transport;
    sProxySessions.erase(it);

    switch (sessTransport)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    case CapabilitiesBitmap::kWiFiPAF:
        Paf::Disconnect(sessionId);
        if (sProxySessions.empty())
            Paf::OnAllSessionsClosed();
        break;
#endif
#if CONFIG_NETWORK_LAYER_BLE
    case CapabilitiesBitmap::kBle:
        Ble::Disconnect(sessionId);
        if (sProxySessions.empty())
            Ble::OnAllSessionsClosed();
        break;
#endif
    default:
        break;
    }

    ChipLogProgress(AppServer, "ProxyDisconnectRequest: session %u cleaned up", sessionId);
    return chip::Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::CancelPendingConnect(chip::FabricIndex fabricIndex)
{
    // A null-SessionID ProxyDisconnectRequest cancels the invoking fabric's own
    // pending connect(s). Each transport returns:
    //   Success       — a connect owned by this fabric was cancelled
    //   NotFound      — a connect is pending but owned by a different fabric
    //   InvalidInState — no connect pending on that transport
    // Every transport is checked (no early return): the spec says null cancels
    // *any* ongoing ProxyConnectRequest for the fabric, so a connect pending on
    // a second transport must also be cancelled, and a foreign connect on one
    // transport must not mask this fabric's connect on another. Cancellation
    // stays strictly same-fabric — a foreign connect is never touched; it only
    // affects the returned status.
    bool cancelledOwn      = false;
    bool sawForeignPending = false;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    auto pafStatus = Paf::CancelPendingConnect(fabricIndex);
    if (pafStatus == chip::Protocols::InteractionModel::Status::Success)
        cancelledOwn = true;
    else if (pafStatus == chip::Protocols::InteractionModel::Status::NotFound)
        sawForeignPending = true;
#endif
#if CONFIG_NETWORK_LAYER_BLE
    auto bleStatus = Ble::CancelPendingConnect(fabricIndex);
    if (bleStatus == chip::Protocols::InteractionModel::Status::Success)
        cancelledOwn = true;
    else if (bleStatus == chip::Protocols::InteractionModel::Status::NotFound)
        sawForeignPending = true;
#endif
    if (cancelledOwn)
        return chip::Protocols::InteractionModel::Status::Success;
    return sawForeignPending ? chip::Protocols::InteractionModel::Status::NotFound
                             : chip::Protocols::InteractionModel::Status::InvalidInState;
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyBackgroundScanStartRequest(
    chip::BitMask<CapabilitiesBitmap> transport, uint16_t timeout, chip::BitMask<WiFiBandBitmap> wiFiBandsMask,
    chip::FabricIndex fabricIndex, chip::NodeId nodeId, app::CommandHandler * commandObj, const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "ProxyBackgroundScanStartRequest transport:%u timeout:%u fabricIndex:%u nodeId:0x" ChipLogFormatX64,
                    transport.Raw(), timeout, fabricIndex, ChipLogValueX64(nodeId));

    [[maybe_unused]] const WiFiBandBitmap wiFiBands = static_cast<WiFiBandBitmap>(wiFiBandsMask.Raw());
    const uint8_t tbits                             = transport.Raw();
    const bool needsPaf                             = (tbits & static_cast<uint8_t>(CapabilitiesBitmap::kWiFiPAF)) != 0;
    const bool needsBle                             = (tbits & static_cast<uint8_t>(CapabilitiesBitmap::kBle)) != 0;

    // A background scan may select multiple transports (spec: "Multiple
    // transports can be selected for the scan").  Start each requested transport
    // independently — each gets only its own transport bit so its per-fabric
    // record and stop-overlap accounting stay transport-local.  The cluster has
    // already verified every requested bit is supported.
    bool anyHandled = false;
    auto result     = chip::Protocols::InteractionModel::Status::Success;

    if (needsPaf)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        anyHandled = true;
        auto s     = Paf::BgScanStart(CapabilitiesBitmap::kWiFiPAF, timeout, wiFiBands, fabricIndex, nodeId, mServer);
        if (s != chip::Protocols::InteractionModel::Status::Success)
            result = s;
#else
        return chip::Protocols::InteractionModel::Status::InvalidTransportType;
#endif
    }

    if (needsBle)
    {
#if CONFIG_NETWORK_LAYER_BLE
        anyHandled = true;
        auto s     = Ble::BgScanStart(CapabilitiesBitmap::kBle, timeout, wiFiBands, fabricIndex, nodeId, mServer);
        if (s != chip::Protocols::InteractionModel::Status::Success && result == chip::Protocols::InteractionModel::Status::Success)
            result = s;
#else
        return chip::Protocols::InteractionModel::Status::InvalidTransportType;
#endif
    }

    if (!anyHandled)
        return chip::Protocols::InteractionModel::Status::InvalidTransportType;
    return result;
}

Protocols::InteractionModel::Status
Clusters::CommissioningProxy::MyCPDelegate::ProxyBackgroundScanStopRequest(chip::BitMask<CapabilitiesBitmap> transportMask,
                                                                           chip::BitMask<WiFiBandBitmap> wiFiBandsMask,
                                                                           chip::FabricIndex fabricIndex, chip::NodeId nodeId)
{
    ChipLogProgress(AppServer,
                    "ProxyBackgroundScanStopRequest transport:0x%x wiFiBands:0x%x fabricIndex:%u nodeId:0x" ChipLogFormatX64,
                    transportMask.Raw(), wiFiBandsMask.Raw(), fabricIndex, ChipLogValueX64(nodeId));

    [[maybe_unused]] const CapabilitiesBitmap transport = static_cast<CapabilitiesBitmap>(transportMask.Raw());
    [[maybe_unused]] const WiFiBandBitmap wiFiBands     = static_cast<WiFiBandBitmap>(wiFiBandsMask.Raw());

    // Fan the stop out to every transport; each matches the request against its
    // own per-fabric record.  NotFound is returned only if no transport had a
    // matching fabric record at all.
    bool matched = false;
    auto err     = chip::Protocols::InteractionModel::Status::Success;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    {
        auto pafStatus = Paf::BgScanStop(transport, wiFiBands, fabricIndex, nodeId);
        if (pafStatus != chip::Protocols::InteractionModel::Status::NotFound)
        {
            matched = true;
            if (pafStatus != chip::Protocols::InteractionModel::Status::Success)
                err = pafStatus;
        }
    }
#endif
#if CONFIG_NETWORK_LAYER_BLE
    {
        auto bleStatus = Ble::BgScanStop(transport, wiFiBands, fabricIndex, nodeId);
        if (bleStatus != chip::Protocols::InteractionModel::Status::NotFound)
        {
            matched = true;
            if (bleStatus != chip::Protocols::InteractionModel::Status::Success)
                err = bleStatus;
        }
    }
#endif
    return matched ? err : chip::Protocols::InteractionModel::Status::NotFound;
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
    if (Ble::IsConnectPending())
        ++count;
#endif
    return count;
}

uint8_t Clusters::CommissioningProxy::MyCPDelegate::GetNumCachedResults()
{
    // CachedResults is a single cluster-wide list; the shared cache holds the
    // combined view across transports (and enforces the combined MaxCachedResults cap).
    return BgScanCache::Count();
}

CHIP_ERROR Clusters::CommissioningProxy::MyCPDelegate::EncodeCachedResults(app::AttributeValueEncoder & encoder)
{
    using ScanResultT = chip::app::Clusters::CommissioningProxy::Structs::ScanResultStruct::Type;
    std::vector<ScanResultT> all;
    BgScanCache::Collect(all);

    if (all.empty())
    {
        chip::app::DataModel::Nullable<chip::app::DataModel::List<const ScanResultT>> nullValue;
        return encoder.Encode(nullValue);
    }

    return encoder.EncodeList([&all](const auto & listEncoder) -> CHIP_ERROR {
        for (const auto & r : all)
        {
            ReturnErrorOnFailure(listEncoder.Encode(r));
        }
        return CHIP_NO_ERROR;
    });
}
