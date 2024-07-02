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

#include <lib/core/CHIPError.h>

#include <string>
#include <thread>

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
#include "pigweed/rpc_services/FabricBridge.h"
#endif

#include "Device.h"
#include "DeviceManager.h"

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
};

pw::Status FabricBridge::AddSynchronizedDevice(const chip_rpc_SynchronizedDevice & request, pw_protobuf_Empty & response)
{
    NodeId nodeId = request.node_id;
    ChipLogProgress(NotSpecified, "Received AddSynchronizedDevice: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    Device * device = new Device(nodeId);
    device->SetReachable(true);

    int result = DeviceMgr().AddDeviceEndpoint(device, 1);
    if (result == -1)
    {
        delete device;
        ChipLogError(NotSpecified, "Failed to add device with nodeId=0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
        return pw::Status::Unknown();
    }

    return pw::OkStatus();
}

pw::Status FabricBridge::RemoveSynchronizedDevice(const chip_rpc_SynchronizedDevice & request, pw_protobuf_Empty & response)
{
    NodeId nodeId = request.node_id;
    ChipLogProgress(NotSpecified, "Received RemoveSynchronizedDevice: " ChipLogFormatX64, ChipLogValueX64(nodeId));

    int removed_idx = DeviceMgr().RemoveDeviceByNodeId(nodeId);
    if (removed_idx < 0)
    {
        ChipLogError(NotSpecified, "Failed to remove device with nodeId=0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
        return pw::Status::NotFound();
    }

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
