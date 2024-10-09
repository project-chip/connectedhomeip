/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "pw_rpc/server.h"
#include "pw_rpc_system_server/rpc_server.h"
#include "pw_rpc_system_server/socket.h"

#include <app/clusters/ecosystem-information-server/ecosystem-information-server.h>
#include <lib/core/CHIPError.h>

#include <string>
#include <thread>

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
#include "pigweed/rpc_services/FabricBridge.h"
#endif

#include "BridgedDevice.h"
#include "BridgedDeviceManager.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
class FabricBridge final : public chip::rpc::FabricBridge
{
public:
    pw::Status AddSynchronizedDevice(const chip_rpc_SynchronizedDevice & request, pw_protobuf_Empty & response) override;
    pw::Status RemoveSynchronizedDevice(const chip_rpc_SynchronizedDevice & request, pw_protobuf_Empty & response) override;
    pw::Status ActiveChanged(const chip_rpc_KeepActiveChanged & request, pw_protobuf_Empty & response) override;
    pw::Status AdminCommissioningAttributeChanged(const chip_rpc_AdministratorCommissioningChanged & request,
                                                  pw_protobuf_Empty & response) override;
};

pw::Status FabricBridge::AddSynchronizedDevice(const chip_rpc_SynchronizedDevice & request, pw_protobuf_Empty & response)
{
    VerifyOrReturnValue(request.has_id, pw::Status::InvalidArgument());
    ScopedNodeId scopedNodeId(request.id.node_id, request.id.fabric_index);
    ChipLogProgress(NotSpecified, "Received AddSynchronizedDevice: Id=[%d:" ChipLogFormatX64 "]", scopedNodeId.GetFabricIndex(),
                    ChipLogValueX64(scopedNodeId.GetNodeId()));

    auto device = std::make_unique<BridgedDevice>(scopedNodeId);
    device->SetReachable(true);

    BridgedDevice::BridgedAttributes attributes;

    if (request.has_unique_id)
    {
        attributes.uniqueId = request.unique_id;
    }

    if (request.has_vendor_name)
    {
        attributes.vendorName = request.vendor_name;
    }

    if (request.has_vendor_id)
    {
        attributes.vendorId = request.vendor_id;
    }

    if (request.has_product_name)
    {
        attributes.productName = request.product_name;
    }

    if (request.has_product_id)
    {
        attributes.productId = request.product_id;
    }

    if (request.has_node_label)
    {
        attributes.nodeLabel = request.node_label;
    }

    if (request.has_hardware_version)
    {
        attributes.hardwareVersion = request.hardware_version;
    }

    if (request.has_hardware_version_string)
    {
        attributes.hardwareVersionString = request.hardware_version_string;
    }

    if (request.has_software_version)
    {
        attributes.softwareVersion = request.software_version;
    }

    if (request.has_software_version_string)
    {
        attributes.softwareVersionString = request.software_version_string;
    }

    device->SetBridgedAttributes(attributes);
    device->SetIcd(request.has_is_icd && request.is_icd);

    auto result = BridgeDeviceMgr().AddDeviceEndpoint(std::move(device), 1 /* parentEndpointId */);
    if (!result.has_value())
    {
        ChipLogError(NotSpecified, "Failed to add device with Id=[%d:0x" ChipLogFormatX64 "]", scopedNodeId.GetFabricIndex(),
                     ChipLogValueX64(scopedNodeId.GetNodeId()));
        return pw::Status::Unknown();
    }

    BridgedDevice * addedDevice = BridgeDeviceMgr().GetDeviceByScopedNodeId(scopedNodeId);
    VerifyOrDie(addedDevice);

    CHIP_ERROR err = EcosystemInformation::EcosystemInformationServer::Instance().AddEcosystemInformationClusterToEndpoint(
        addedDevice->GetEndpointId());
    VerifyOrDie(err == CHIP_NO_ERROR);

    return pw::OkStatus();
}

pw::Status FabricBridge::RemoveSynchronizedDevice(const chip_rpc_SynchronizedDevice & request, pw_protobuf_Empty & response)
{
    VerifyOrReturnValue(request.has_id, pw::Status::InvalidArgument());
    ScopedNodeId scopedNodeId(request.id.node_id, request.id.fabric_index);
    ChipLogProgress(NotSpecified, "Received RemoveSynchronizedDevice: Id=[%d:" ChipLogFormatX64 "]", scopedNodeId.GetFabricIndex(),
                    ChipLogValueX64(scopedNodeId.GetNodeId()));

    auto removed_idx = BridgeDeviceMgr().RemoveDeviceByScopedNodeId(scopedNodeId);
    if (!removed_idx.has_value())
    {
        ChipLogError(NotSpecified, "Failed to remove device with Id=[%d:0x" ChipLogFormatX64 "]", scopedNodeId.GetFabricIndex(),
                     ChipLogValueX64(scopedNodeId.GetNodeId()));
        return pw::Status::NotFound();
    }

    return pw::OkStatus();
}

pw::Status FabricBridge::ActiveChanged(const chip_rpc_KeepActiveChanged & request, pw_protobuf_Empty & response)
{
    VerifyOrReturnValue(request.has_id, pw::Status::InvalidArgument());
    ScopedNodeId scopedNodeId(request.id.node_id, request.id.fabric_index);
    ChipLogProgress(NotSpecified, "Received ActiveChanged: Id=[%d:" ChipLogFormatX64 "]", scopedNodeId.GetFabricIndex(),
                    ChipLogValueX64(scopedNodeId.GetNodeId()));

    auto * device = BridgeDeviceMgr().GetDeviceByScopedNodeId(scopedNodeId);
    if (device == nullptr)
    {
        ChipLogError(NotSpecified, "Could not find bridged device associated with Id=[%d:0x" ChipLogFormatX64 "]",
                     scopedNodeId.GetFabricIndex(), ChipLogValueX64(scopedNodeId.GetNodeId()));
        return pw::Status::NotFound();
    }

    device->LogActiveChangeEvent(request.promised_active_duration_ms);
    return pw::OkStatus();
}

pw::Status FabricBridge::AdminCommissioningAttributeChanged(const chip_rpc_AdministratorCommissioningChanged & request,
                                                            pw_protobuf_Empty & response)
{
    VerifyOrReturnValue(request.has_id, pw::Status::InvalidArgument());
    ScopedNodeId scopedNodeId(request.id.node_id, request.id.fabric_index);
    ChipLogProgress(NotSpecified, "Received CADMIN attribute change: Id=[%d:" ChipLogFormatX64 "]", scopedNodeId.GetFabricIndex(),
                    ChipLogValueX64(scopedNodeId.GetNodeId()));

    auto * device = BridgeDeviceMgr().GetDeviceByScopedNodeId(scopedNodeId);
    if (device == nullptr)
    {
        ChipLogError(NotSpecified, "Could not find bridged device associated with Id=[%d:0x" ChipLogFormatX64 "]",
                     scopedNodeId.GetFabricIndex(), ChipLogValueX64(scopedNodeId.GetNodeId()));
        return pw::Status::NotFound();
    }

    BridgedDevice::AdminCommissioningAttributes adminCommissioningAttributes;

    uint32_t max_window_status_value =
        static_cast<uint32_t>(chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kUnknownEnumValue);
    VerifyOrReturnValue(request.window_status < max_window_status_value, pw::Status::InvalidArgument());
    adminCommissioningAttributes.commissioningWindowStatus =
        static_cast<chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum>(request.window_status);
    if (request.has_opener_fabric_index)
    {
        VerifyOrReturnValue(request.opener_fabric_index >= chip::kMinValidFabricIndex, pw::Status::InvalidArgument());
        VerifyOrReturnValue(request.opener_fabric_index <= chip::kMaxValidFabricIndex, pw::Status::InvalidArgument());
        adminCommissioningAttributes.openerFabricIndex = static_cast<FabricIndex>(request.opener_fabric_index);
    }

    if (request.has_opener_vendor_id)
    {
        VerifyOrReturnValue(request.opener_vendor_id != chip::VendorId::NotSpecified, pw::Status::InvalidArgument());
        adminCommissioningAttributes.openerVendorId = static_cast<chip::VendorId>(request.opener_vendor_id);
    }

    device->SetAdminCommissioningAttributes(adminCommissioningAttributes);
    return pw::OkStatus();
}

FabricBridge fabric_bridge_service;
#endif // defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE

void RegisterServices(pw::rpc::Server & server)
{
#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
    server.RegisterService(fabric_bridge_service);
#endif
}

} // namespace

void RunRpcService()
{
    pw::rpc::system_server::Init();
    RegisterServices(pw::rpc::system_server::Server());
    pw::rpc::system_server::Start();
}

void InitRpcServer(uint16_t rpcServerPort)
{
    pw::rpc::system_server::set_socket_port(rpcServerPort);
    std::thread rpc_service(RunRpcService);
    rpc_service.detach();
}
