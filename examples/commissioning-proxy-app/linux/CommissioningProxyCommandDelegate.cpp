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

using namespace chip;
using namespace chip::app;
using namespace Clusters::CommissioningProxy;

namespace {

using ScanResultT = chip::app::Clusters::CommissioningProxy::Structs::ScanResultStruct::Type;

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

    // TODO: initiate the actual transport connection here using the transport-specific
    //       layer (BLE/WiFiPAF/NTL), honouring the Timeout field per spec [10.124].

    // Send ProxyConnectResponse with a sessionId per spec [10.125-10.127].
    // TODO: allocate a real per-session ID when multiple sessions are supported.
    Commands::ProxyConnectResponse::Type response;
    response.sessionId = 1;
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
