/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <AppMain.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include "NamedPipeCommands.h"

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF && CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
#include <platform/Linux/ConnectivityManagerImpl.h>
#include <platform/PlatformManager.h>
#endif

#include <string>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/connectivity.h>
#include <imgui_ui/windows/light.h>
#include <imgui_ui/windows/occupancy_sensing.h>
#include <imgui_ui/windows/qrcode.h>
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
NamedPipeCommands sChipNamedPipeCommands;
} // namespace

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF && CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
static void CancelWiFiPAFPublish()
{
    uint32_t publishId = LinuxDeviceOptions::GetInstance().mPublishId;
    if (publishId == 0)
        return;

    // Stop advertising the proxy as a NAN publisher — it is reached by chip-tool
    // over TCP/IP, not NAN.  Cancel the publish and disconnect the nanreceive
    // signal handler so that a subsequent _WiFiPAFSubscribe (triggered by
    // ProxyConnectRequest) registers exactly one handler and packets are not
    // delivered twice.
    TEMPORARY_RETURN_IGNORED DeviceLayer::ConnectivityMgr().WiFiPAFCancelPublish(publishId);
    DeviceLayer::ConnectivityMgrImpl().WiFiPAFDisconnectPublishReceiveHandler();
    LinuxDeviceOptions::GetInstance().mPublishId = 0;
    ChipLogProgress(AppServer, "CommissioningProxy: cancelled WiFi-PAF publish and disconnected nanreceive handler");
}

static void OnChipDeviceEvent(const DeviceLayer::ChipDeviceEvent * event, intptr_t)
{
    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        ChipLogProgress(AppServer, "CommissioningProxy: commissioning complete, cancelling WiFi-PAF publish");
        CancelWiFiPAFPublish();
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF && CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY

#if 0
void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{

    switch (attributePath.mClusterId)
    {
        case Clusters::CommissioningProxy::Id:
        {
            switch (attributePath.mAttributeId)
            {
                case Clusters::CommissioningProxy::Attributes::ScanMaxTime::Id:
                {
                    CommissioningProxyMgr().InitiateAction(CommissioningProxyManager::DISCONNECT_ACTION);
                    break;
                }

                default:
                {
                    ChipLogDetail(NotSpecified, "%s: Commissioning Proxy attribute %#x (%u) (type %#x size %u) updated",
                        __func__,
                        attributePath.mAttributeId, attributePath.mAttributeId,
                        type, size);
                    break;
                }
            }
            break;
        }

        default:
        {
            ChipLogDetail(NotSpecified, "%s: cluster %#x (%u) attribute %#x (%u) (type %#x size %u) updated",
                __func__, attributePath.mClusterId, attributePath.mClusterId,
                attributePath.mAttributeId, attributePath.mAttributeId, type, size);
            break;
        }
    }
}
#endif

#if 0
#if 0


bool emberAfCommissioningProxyClusterProxyScanRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::CommissioningProxy::Commands::ProxyScanRequest::DecodableType & commandData)
{
    using ScanResultT = chip::app::Clusters::CommissioningProxy::Structs::ScanResultStruct::DecodableType;
    std::vector<ScanResultT> results;
    // ScanResultT r{};

    // Start PAF
    CHIP_ERROR err = WiFiPAF::WiFiPAFLayer::GetWiFiPAFLayer().Init(&DeviceLayer::SystemLayer());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Commissioning Proxy PAF Scan Request Failed");
        return false;
    }

    // TODO: parse these from commandData
    //chip::Optional<uint16_t> srvProtoType;  // set if you want to filter
    //uint32_t windowMs = 1500;

    std::vector<chip::DeviceLayer::NanPeerInfo> peers;
    //err = chip::DeviceLayer::ConnectivityMgrImpl().ScanNanPublishers(
    //    serviceName, srvProtoType, windowMs, peers);

    ChipLogProgress(NotSpecified, "=== %s() Transport:0x%x WiFiBands:0x%x", __func__,
        static_cast<uint16_t>(commandData.transport.Raw()),
        static_cast<uint16_t>((commandData.wiFiBands.HasValue() ? commandData.wiFiBands.Value().Raw() : 0)));

    // Create a Handle and move it into ConnectivityManagerImpl
    // This keeps the ProxyScanRequest open, so the scan can complete before the ProxyScanResponse is sent
    CommandHandler::Handle handle(commandObj);
 bang1;   err = chip::DeviceLayer::ConnectivityMgrImpl()._WiFiPAFScan( std::move(handle), commandPath);
    if (err != CHIP_NO_ERROR) {
        commandObj->AddStatus(commandPath, chip::Protocols::InteractionModel::Status::Failure);
        return true;
    }

    ChipLogProgress(Controller, "===SHM %s() Before", __func__);
    // Ensure Response Timeout is greater than the ScanMaxTime
    if (auto * ec = commandObj->GetExchangeContext())
    {
        ec->SetResponseTimeout(chip::System::Clock::Seconds16(140));
        ChipLogProgress(Controller, "===SHM %s() In", __func__);
    }

    return true;
}

bool emberAfCommissioningProxyClusterProxyMessageRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::CommissioningProxy::Commands::ProxyMessageRequest::DecodableType & commandData)
{
    ChipLogError(NotSpecified, "=== %s() Received ProxyMessageRequest", __func__);
    return true;
}

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
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
#endif // #if 0

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/clusters/device-energy-management-server/CodegenIntegration.h>
#include "commissioning-proxy-delegate-impl.h"
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include "CPAppCommandDelegate.h"

// In a .cpp file
CPAppCommandDelegate sCPAppCommandDelegate;
// CommissioningProxyManager gCommissioningProxyManager;
chip::app::Clusters::CommissioningProxy::MyCPDelegate gMyCPDelegate;

chip::BitMask<chip::app::Clusters::CommissioningProxy::Feature> gFeatures(
    chip::app::Clusters::CommissioningProxy::Feature::kBackgroundScan,
    chip::app::Clusters::CommissioningProxy::Feature::kWiFiNetworkInterface
);

chip::app::RegisteredServerCluster<chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster> gCPCluster(
    chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster::Config(
        CommissioningProxyEndpoint, gFeatures, gMyCPDelegate)
);

void ApplicationInit()
{
    std::string path = std::string(LinuxDeviceOptions::GetInstance().app_pipe);
    if ((!path.empty()) and (sChipNamedPipeCommands.Start(path, &sCPAppCommandDelegate) != CHIP_NO_ERROR))
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        TEMPORARY_RETURN_IGNORED sChipNamedPipeCommands.Stop();
    }

    // Register the Commissioning Proxy Code Driven mechanism
    VerifyOrDie(chip::app::CodegenDataModelProvider::Instance().Registry().Register(gCPCluster.Registration()) == CHIP_NO_ERROR);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF && CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
    // If the proxy is already on a fabric (i.e. it was previously commissioned
    // via PAF and restarted), cancel the WiFi-PAF publish immediately.
    // If it still needs to be commissioned, register an event handler to cancel
    // the publish once commissioning completes.
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() > 0)
    {
        CancelWiFiPAFPublish();
    }
    else
    {
        TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().AddEventHandler(OnChipDeviceEvent, 0);
    }
#endif

    ChipLogProgress(AppServer, "===SHM %s()", __func__);
    ChipLogProgress(AppServer, "%s(): Main function is Proxy Commissioner on endpoint %u",
            __func__, CommissioningProxyEndpoint);
}

void ApplicationShutdown()
{
    if (sChipNamedPipeCommands.Stop() != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to stop CHIP NamedPipeCommands");
    }
}


int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }
    ChipLinuxAppMainLoop();
    return 0;
}
