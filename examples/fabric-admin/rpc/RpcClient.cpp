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

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "fabric_bridge_service/fabric_bridge_service.rpc.pb.h"
#include "pw_assert/check.h"
#include "pw_hdlc/decoder.h"
#include "pw_hdlc/default_addresses.h"
#include "pw_hdlc/rpc_channel.h"
#include "pw_rpc/client.h"
#include "pw_stream/socket_stream.h"

using namespace chip;

namespace {

// Constants
constexpr uint32_t kRpcTimeoutMs     = 1000;
constexpr uint32_t kDefaultChannelId = 1;

// Fabric Bridge Client
rpc::pw_rpc::nanopb::FabricBridge::Client fabricBridgeClient(rpc::client::GetDefaultRpcClient(), kDefaultChannelId);
pw::rpc::NanopbUnaryReceiver<::pw_protobuf_Empty> addSynchronizedDeviceCall;
pw::rpc::NanopbUnaryReceiver<::pw_protobuf_Empty> removeSynchronizedDeviceCall;

std::mutex responseMutex;
std::condition_variable responseCv;
bool responseReceived    = false;
CHIP_ERROR responseError = CHIP_NO_ERROR;

template <typename CallType>
CHIP_ERROR WaitForResponse(CallType & call)
{
    std::unique_lock<std::mutex> lock(responseMutex);
    responseReceived = false;
    responseError    = CHIP_NO_ERROR;

    if (responseCv.wait_for(lock, std::chrono::milliseconds(kRpcTimeoutMs), [] { return responseReceived; }))
    {
        return responseError;
    }
    else
    {
        fprintf(stderr, "RPC Response timed out!");
        return CHIP_ERROR_TIMEOUT;
    }
}

// Callback function to be called when the RPC response is received
void OnAddDeviceResponseCompleted(const pw_protobuf_Empty & response, pw::Status status)
{
    std::lock_guard<std::mutex> lock(responseMutex);
    responseReceived = true;
    responseError    = status.ok() ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    responseCv.notify_one();

    if (status.ok())
    {
        ChipLogProgress(NotSpecified, "AddSynchronizedDevice RPC call succeeded!");
    }
    else
    {
        ChipLogProgress(NotSpecified, "AddSynchronizedDevice RPC call failed with status: %d\n", status.code());
    }
}

// Callback function to be called when the RPC response is received
void OnRemoveDeviceResponseCompleted(const pw_protobuf_Empty & response, pw::Status status)
{
    std::lock_guard<std::mutex> lock(responseMutex);
    responseReceived = true;
    responseError    = status.ok() ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    responseCv.notify_one();

    if (status.ok())
    {
        ChipLogProgress(NotSpecified, "RemoveSynchronizedDevice RPC call succeeded!");
    }
    else
    {
        ChipLogProgress(NotSpecified, "RemoveSynchronizedDevice RPC call failed with status: %d", status.code());
    }
}

} // namespace

CHIP_ERROR InitRpcClient(uint16_t rpcServerPort)
{
    rpc::client::SetRpcServerPort(rpcServerPort);
    return rpc::client::StartPacketProcessing();
}

CHIP_ERROR AddSynchronizedDevice(chip::NodeId nodeId)
{
    ChipLogProgress(NotSpecified, "AddSynchronizedDevice");

    if (addSynchronizedDeviceCall.active())
    {
        ChipLogError(NotSpecified, "Add Synchronized Device operation is in progress\n");
        return CHIP_ERROR_BUSY;
    }

    chip_rpc_SynchronizedDevice device;
    device.node_id = nodeId;

    // By assigning the returned call to the global 'addSynchronizedDeviceCall', the RPC
    // call is kept alive until it completes. When a response is received, it
    // will be logged by the handler function and the call will complete.
    addSynchronizedDeviceCall = fabricBridgeClient.AddSynchronizedDevice(device, OnAddDeviceResponseCompleted);

    if (!addSynchronizedDeviceCall.active())
    {
        // The RPC call was not sent. This could occur due to, for example, an invalid channel ID. Handle if necessary.
        return CHIP_ERROR_INTERNAL;
    }

    return WaitForResponse(addSynchronizedDeviceCall);
}

CHIP_ERROR RemoveSynchronizedDevice(chip::NodeId nodeId)
{
    ChipLogProgress(NotSpecified, "RemoveSynchronizedDevice");

    if (removeSynchronizedDeviceCall.active())
    {
        ChipLogError(NotSpecified, "Remove Synchronized Device operation is in progress\n");
        return CHIP_ERROR_BUSY;
    }

    chip_rpc_SynchronizedDevice device;
    device.node_id = nodeId;

    // By assigning the returned call to the global 'removeSynchronizedDeviceCall', the RPC
    // call is kept alive until it completes. When a response is received, it
    // will be logged by the handler function and the call will complete.
    removeSynchronizedDeviceCall = fabricBridgeClient.RemoveSynchronizedDevice(device, OnRemoveDeviceResponseCompleted);

    if (!removeSynchronizedDeviceCall.active())
    {
        // The RPC call was not sent. This could occur due to, for example, an invalid channel ID. Handle if necessary.
        return CHIP_ERROR_INTERNAL;
    }

    return WaitForResponse(removeSynchronizedDeviceCall);
}
