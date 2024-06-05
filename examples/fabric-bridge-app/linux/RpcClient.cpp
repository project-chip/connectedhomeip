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

#include "RpcClient.h"
#include "RpcClientProcessor.h"

#include <string>
#include <thread>
#include <unistd.h>

#include "fabric_admin_service/fabric_admin_service.rpc.pb.h"
#include "pw_assert/check.h"
#include "pw_hdlc/decoder.h"
#include "pw_hdlc/default_addresses.h"
#include "pw_hdlc/rpc_channel.h"
#include "pw_rpc/client.h"
#include "pw_stream/socket_stream.h"

using namespace chip;

namespace {

// Constants
constexpr uint32_t kDefaultChannelId = 1;

// Fabric Admin Client
rpc::pw_rpc::nanopb::FabricAdmin::Client fabricAdminClient(rpc::client::GetDefaultRpcClient(), kDefaultChannelId);
pw::rpc::NanopbUnaryReceiver<::chip_rpc_OperationStatus> openCommissioningWindowCall;

// Callback function to be called when the RPC response is received
void OnOpenCommissioningWindowCompleted(const chip_rpc_OperationStatus & response, pw::Status status)
{
    if (status.ok())
    {
        ChipLogProgress(NotSpecified, "OpenCommissioningWindow received operation status: %d", response.success);
    }
    else
    {
        ChipLogProgress(NotSpecified, "OpenCommissioningWindow RPC call failed with status: %d\n", status.code());
    }
}

} // namespace

CHIP_ERROR InitRpcClient(uint16_t rpcServerPort)
{
    rpc::client::SetRpcServerPort(rpcServerPort);
    return rpc::client::StartPacketProcessing();
}

CHIP_ERROR OpenCommissioningWindow(NodeId nodeId)
{
    ChipLogProgress(NotSpecified, "OpenCommissioningWindow\n");

    if (openCommissioningWindowCall.active())
    {
        ChipLogError(NotSpecified, "OpenCommissioningWindow is in progress\n");
        return CHIP_ERROR_BUSY;
    }

    chip_rpc_DeviceInfo device;
    device.node_id = nodeId;

    // The RPC will remain active as long as `openCommissioningWindowCall` is alive.
    openCommissioningWindowCall = fabricAdminClient.OpenCommissioningWindow(device, OnOpenCommissioningWindowCompleted);

    if (!openCommissioningWindowCall.active())
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}
