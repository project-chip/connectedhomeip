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

#include <system/SystemClock.h>
#include <thread>

#if defined(PW_RPC_FABRIC_ADMIN_SERVICE) && PW_RPC_FABRIC_ADMIN_SERVICE
#include "pigweed/rpc_services/FabricAdmin.h"
#endif

namespace {

#if defined(PW_RPC_FABRIC_ADMIN_SERVICE) && PW_RPC_FABRIC_ADMIN_SERVICE
class FabricAdmin final : public chip::rpc::FabricAdmin
{
public:
    pw::Status OpenCommissioningWindow(const chip_rpc_DeviceInfo & request, chip_rpc_OperationStatus & response) override
    {
        chip::NodeId nodeId = request.node_id;
        printf("Received OpenCommissioningWindow request: 0x%llx\n", nodeId);
        response.success = false;

        return pw::OkStatus();
    }
};

FabricAdmin fabric_admin_service;
#endif // defined(PW_RPC_FABRIC_ADMIN_SERVICE) && PW_RPC_FABRIC_ADMIN_SERVICE

void RegisterServices(pw::rpc::Server & server)
{
#if defined(PW_RPC_FABRIC_ADMIN_SERVICE) && PW_RPC_FABRIC_ADMIN_SERVICE
    server.RegisterService(fabric_admin_service);
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
