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
#include "commissioning-proxy-delegate-impl.h"

using namespace chip;
using namespace chip::app;
using namespace Clusters::CommissioningProxy;

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyConnectRequest(
                        DataModel::Nullable<chip::ByteSpan> address,
                        CapabilitiesBitmap transport,
                        uint16_t discriminator,
                        chip::VendorId vendorid,
                        uint16_t productid,
                        WiFiBandBitmap wiFiBand,
                        app::CommandHandler * commandObj,
                        const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "===SHM %s(), transport:%d wiFiBand:%d", __func__, (int)transport, (int)wiFiBand);
    // Cluster state is owned by the cluster; access it via the bound server pointer.
    ChipLogProgress(NotSpecified, "=== %s() State:%u", __func__, (uint8_t)Server().GetCPState());

    // Successfully Connected, change CP state
    CHIP_ERROR err = Server().SetCPState(CommissioningProxyCluster::kState_CPConnected);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Commissioning Proxy SetCPState() Failed");
        return chip::Protocols::InteractionModel::Status::Failure;
    }
    ChipLogProgress(NotSpecified, "=== %s() State:%u", __func__, (uint8_t)Server().GetCPState());
    return chip::Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Clusters::CommissioningProxy::MyCPDelegate::ProxyScanRequest(
    CapabilitiesBitmap transport, WiFiBandBitmap wiFiBands, app::CommandHandler * commandObj,
    const DataModel::InvokeRequest & request)
{
    ChipLogProgress(AppServer, "===SHM %s(), transport:%d wiFiBands:%d", __func__, (int)transport, (int)wiFiBands);
    using ScanResultT = Clusters::CommissioningProxy::Structs::ScanResultStruct::DecodableType;
    std::vector<ScanResultT> results;

    // Start PAF
    CHIP_ERROR err = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().Init(&DeviceLayer::SystemLayer());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Commissioning Proxy PAF Scan Request Failed");
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    ChipLogProgress(NotSpecified, "=== %s() Transport:0x%x WiFiBands:0x%x", __func__,
        static_cast<uint16_t>(transport), static_cast<uint16_t>(wiFiBands));

    // Create a Handle and move it into ConnectivityManagerImpl
    // This keeps the ProxyScanRequest open, so the scan can complete before the ProxyScanResponse is sent
    // Scan results are processed in ConnectivityManagerImpl::ScanDiscoveryResult()
    // scanMaxTime expiry handled in ConnectivityManagerImpl::FinishWiFiPAFScanAndRespond()
    CommandHandler::Handle handle(commandObj);
    uint8_t scanMaxTime = GetScanMaxTime();
    err = chip::DeviceLayer::ConnectivityMgrImpl()._WiFiPAFScan( std::move(handle), request.path, scanMaxTime);
    if (err != CHIP_NO_ERROR) {
        commandObj->AddStatus(request.path, chip::Protocols::InteractionModel::Status::Failure);
        return chip::Protocols::InteractionModel::Status::Failure;
    }

    ChipLogProgress(Controller, "===SHM %s() Before", __func__);
    // Ensure Response Timeout is greater than the ScanMaxTime
    if (auto * responseTimeout = commandObj->GetExchangeContext())
    {
        responseTimeout->SetResponseTimeout(chip::System::Clock::Seconds16(scanMaxTime + 5));
        ChipLogProgress(Controller, "===SHM %s() In", __func__);
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

#if 0
bool emberAfCommissioningProxyClusterProxyBackGroundScanStartRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::CommissioningProxy::Commands::ProxyBackGroundScanStartRequest::DecodableType & commandData)
{
    ChipLogError(NotSpecified, "=== %s() Received ProxyBackGroundScanStartRequest", __func__);

    // Use the NodeId and fabric Index as unique identifiers for the background scan
    FabricIndex fabricIndex = kUndefinedFabricIndex;
    NodeId localNodeId = kUndefinedNodeId;

    fabricIndex = commandObj->GetAccessingFabricIndex();
    if (IsValidFabricIndex(fabricIndex))
    {
        const auto * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
        if (fabricInfo != nullptr)
        {
            localNodeId = fabricInfo->GetNodeId();
        }
    }

    ChipLogProgress(AppServer,
                    "===SHM %s(): fabricIndex=%u localNodeId=0x" ChipLogFormatX64, __func__,
                    static_cast<unsigned>(fabricIndex), ChipLogValueX64(localNodeId));

    return true;
}
#endif