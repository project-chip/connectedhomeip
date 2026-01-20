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

#include <string>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/connectivity.h>
#include <imgui_ui/windows/light.h>
#include <imgui_ui/windows/occupancy_sensing.h>
#include <imgui_ui/windows/qrcode.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
#include "CommissioningProxyCommandDelegate.h"
#include "CommissioningProxyManager.h"
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

NamedPipeCommands sChipNamedPipeCommands;

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
CommissioningProxyAppCommandDelegate sCommissioningProxyAppCommandDelegate;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
} // namespace

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

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
void MatterCommissioningProxyPluginServerInitCallback()
{
    ChipLogError(NotSpecified, "=== %s() Entered", __func__);
    // Needed for linking
}

bool emberAfCommissioningProxyClusterProxyConnectRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::CommissioningProxy::Commands::ProxyConnectRequest::DecodableType & commandData)
{
    ChipLogError(NotSpecified, "=== %s() Received ProxyConnectRequest", __func__);
    return true;
}

bool emberAfCommissioningProxyClusterProxyDisconnectRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::CommissioningProxy::Commands::ProxyDisconnectRequest::DecodableType & commandData)
{
    ChipLogError(NotSpecified, "=== %s() Received ProxyDisconnectRequest", __func__);
    return true;
}

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
    err = chip::DeviceLayer::ConnectivityMgrImpl()._WiFiPAFScan( std::move(handle), commandPath);
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

#if 0
    // Convert each NanPeerInfo -> ScanResultStruct::Type
    for (const auto & p : peers)
    {
        ScanResultT r{};
        r.address.SetNonNull(chip::ByteSpan(p.mac, 6));  // 6-byte MAC

        // If you want to include SSI as extended data:
        if (!p.ssi.empty()) {
            r.extendedData.SetNonNull(chip::ByteSpan(p.ssi.data(), p.ssi.size()));
        } else {
            r.extendedData.SetNull();
        }

        // Fill anything else you can/need. Examples:
        r.transport = chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap::kWiFiPAF;
        r.discriminator = 0; // unknown if you don't have it yet
        r.vendorId = static_cast<chip::VendorId>(0);
        r.productId = 0;

        results.push_back(r);
    }

    ChipLogProgress(NotSpecified, "=== %s() Received ProxyScanRequest", __func__);
    Clusters::CommissioningProxy::Commands::ProxyScanResponse::Type response;
    List<const ScanResultT> list{ Span<const ScanResultT>(results.data(), results.size()) };
    response.proxyScanResult = list;

    // response.numberOfResults = results.size();
    response.numberOfResults = static_cast<uint8_t>(response.proxyScanResult.size());

    commandObj->AddResponse(commandPath, response);
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);

//    commandObj->AddClusterSpecificFailure(commandPath,
  //                                        to_underlying(Clusters::CommissioningProxy::ProxyErrorEnum::kProxyBusy));
#endif
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

void ApplicationInit()
{
    std::string path = std::string(LinuxDeviceOptions::GetInstance().app_pipe);

    if ((!path.empty()) and (sChipNamedPipeCommands.Start(path, &sCommissioningProxyAppCommandDelegate) != CHIP_NO_ERROR))
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        TEMPORARY_RETURN_IGNORED sChipNamedPipeCommands.Stop();
    }

    ChipLogProgress(AppServer, "%s: Main function is Proxy Commissioner on endpoint %u",
            __func__, CommissioningProxyEndpoint);
}

void ApplicationShutdown()
{
    if (sChipNamedPipeCommands.Stop() != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to stop CHIP NamedPipeCommands");
    }
}

#ifdef __NuttX__
// NuttX requires the main function to be defined with C-linkage. However, marking
// the main as extern "C" is not strictly conformant with the C++ standard. Since
// clang >= 20 such code triggers -Wmain warning.
extern "C" {
#endif

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    CHIP_ERROR err = CommissioningProxyMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to initialize Commissioning Proxy manager: %" CHIP_ERROR_FORMAT, err.Format());
        chip::DeviceLayer::PlatformMgr().Shutdown();
        return -1;
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    example::Ui::ImguiUi ui;

    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::Connectivity>());
    ui.AddWindow(std::make_unique<example::Ui::Windows::OccupancySensing>(chip::EndpointId(1), "Occupancy"));
    ui.AddWindow(std::make_unique<example::Ui::Windows::Light>(chip::EndpointId(1)));

    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}

#ifdef __NuttX__
}
#endif
