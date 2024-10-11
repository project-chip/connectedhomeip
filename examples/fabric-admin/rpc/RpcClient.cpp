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

#include "fabric_bridge_service/fabric_bridge_service.pb.h"
#include "fabric_bridge_service/fabric_bridge_service.rpc.pb.h"

using namespace chip;

namespace {

// Constants
constexpr uint32_t kRpcTimeoutMs     = 1000;
constexpr uint32_t kDefaultChannelId = 1;

// Fabric Bridge Client
rpc::pw_rpc::nanopb::FabricBridge::Client fabricBridgeClient(rpc::client::GetDefaultRpcClient(), kDefaultChannelId);

std::mutex responseMutex;
std::condition_variable responseCv;
bool responseReceived    = false;
CHIP_ERROR responseError = CHIP_NO_ERROR;

// By passing the `call` parameter into WaitForResponse we are explicitly trying to insure the caller takes into consideration that
// the lifetime of the `call` object when calling WaitForResponse
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

void RpcCompletedWithEmptyResponse(const pw_protobuf_Empty & response, pw::Status status)
{
    std::lock_guard<std::mutex> lock(responseMutex);
    responseReceived = true;
    responseError    = status.ok() ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    responseCv.notify_one();

    if (status.ok())
    {
        ChipLogProgress(NotSpecified, "RPC call succeeded!");
    }
    else
    {
        ChipLogProgress(NotSpecified, "RPC call failed with status: %d", status.code());
    }
}

} // namespace

void SetRpcRemoteServerPort(uint16_t port)
{
    rpc::client::SetRpcServerPort(port);
}

CHIP_ERROR StartRpcClient()
{
    return rpc::client::StartPacketProcessing();
}

CHIP_ERROR AddSynchronizedDevice(const chip_rpc_SynchronizedDevice & data)
{
    ChipLogProgress(NotSpecified, "AddSynchronizedDevice");

    // The RPC call is kept alive until it completes. When a response is received, it will be logged by the handler
    // function and the call will complete.
    auto call = fabricBridgeClient.AddSynchronizedDevice(data, OnAddDeviceResponseCompleted);

    if (!call.active())
    {
        // The RPC call was not sent. This could occur due to, for example, an invalid channel ID. Handle if necessary.
        return CHIP_ERROR_INTERNAL;
    }

    return WaitForResponse(call);
}

CHIP_ERROR RemoveSynchronizedDevice(ScopedNodeId scopedNodeId)
{
    ChipLogProgress(NotSpecified, "RemoveSynchronizedDevice");

    chip_rpc_SynchronizedDevice device = chip_rpc_SynchronizedDevice_init_default;
    device.has_id                      = true;
    device.id.node_id                  = scopedNodeId.GetNodeId();
    device.id.fabric_index             = scopedNodeId.GetFabricIndex();

    // The RPC call is kept alive until it completes. When a response is received, it will be logged by the handler
    // function and the call will complete.
    auto call = fabricBridgeClient.RemoveSynchronizedDevice(device, OnRemoveDeviceResponseCompleted);

    if (!call.active())
    {
        // The RPC call was not sent. This could occur due to, for example, an invalid channel ID. Handle if necessary.
        return CHIP_ERROR_INTERNAL;
    }

    return WaitForResponse(call);
}

CHIP_ERROR ActiveChanged(ScopedNodeId scopedNodeId, uint32_t promisedActiveDurationMs)
{
    ChipLogProgress(NotSpecified, "ActiveChanged");

    chip_rpc_KeepActiveChanged parameters;
    parameters.has_id                      = true;
    parameters.id.node_id                  = scopedNodeId.GetNodeId();
    parameters.id.fabric_index             = scopedNodeId.GetFabricIndex();
    parameters.promised_active_duration_ms = promisedActiveDurationMs;

    // The RPC call is kept alive until it completes. When a response is received, it will be logged by the handler
    // function and the call will complete.
    auto call = fabricBridgeClient.ActiveChanged(parameters, RpcCompletedWithEmptyResponse);

    if (!call.active())
    {
        // The RPC call was not sent. This could occur due to, for example, an invalid channel ID. Handle if necessary.
        return CHIP_ERROR_INTERNAL;
    }

    return WaitForResponse(call);
}

CHIP_ERROR AdminCommissioningAttributeChanged(const chip_rpc_AdministratorCommissioningChanged & data)
{
    ChipLogProgress(NotSpecified, "AdminCommissioningAttributeChanged");

    // The RPC call is kept alive until it completes. When a response is received, it will be logged by the handler
    // function and the call will complete.
    auto call = fabricBridgeClient.AdminCommissioningAttributeChanged(data, RpcCompletedWithEmptyResponse);

    if (!call.active())
    {
        // The RPC call was not sent. This could occur due to, for example, an invalid channel ID. Handle if necessary.
        return CHIP_ERROR_INTERNAL;
    }

    return WaitForResponse(call);
}
