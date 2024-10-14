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
constexpr uint32_t kRpcTimeoutMs     = 1000;
constexpr uint32_t kDefaultChannelId = 1;

// Fabric Admin Client
rpc::pw_rpc::nanopb::FabricAdmin::Client fabricAdminClient(rpc::client::GetDefaultRpcClient(), kDefaultChannelId);

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
        ChipLogError(NotSpecified, "RPC Response timed out!");
        return CHIP_ERROR_TIMEOUT;
    }
}

// Callback function to be called when the RPC response is received
void OnOpenCommissioningWindowCompleted(const chip_rpc_OperationStatus & response, pw::Status status)
{
    std::lock_guard<std::mutex> lock(responseMutex);
    responseReceived = true;
    responseError    = status.ok() ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    responseCv.notify_one();

    if (status.ok())
    {
        ChipLogProgress(NotSpecified, "OpenCommissioningWindow received operation status: %d", response.success);
    }
    else
    {
        ChipLogProgress(NotSpecified, "OpenCommissioningWindow RPC call failed with status: %d\n", status.code());
    }
}

// Callback function to be called when the RPC response is received for generic empty response.
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

CHIP_ERROR OpenCommissioningWindow(chip_rpc_DeviceCommissioningWindowInfo device)
{
    ChipLogProgress(NotSpecified, "OpenCommissioningWindow with Id=[%d:0x" ChipLogFormatX64 "]", device.id.fabric_index,
                    ChipLogValueX64(device.id.node_id));

    // The RPC call is kept alive until it completes. When a response is received, it will be logged by the handler
    // function and the call will complete.
    auto call = fabricAdminClient.OpenCommissioningWindow(device, OnOpenCommissioningWindowCompleted);

    if (!call.active())
    {
        // The RPC call was not sent. This could occur due to, for example, an invalid channel ID. Handle if necessary.
        return CHIP_ERROR_INTERNAL;
    }

    return WaitForResponse(call);
}

CHIP_ERROR
OpenCommissioningWindow(chip::Controller::CommissioningWindowVerifierParams params, chip::FabricIndex fabricIndex)
{
    chip_rpc_DeviceCommissioningWindowInfo device;
    device.has_id                = true;
    device.id.node_id            = params.GetNodeId();
    device.id.fabric_index       = fabricIndex;
    device.commissioning_timeout = params.GetTimeout().count();
    device.discriminator         = params.GetDiscriminator();
    device.iterations            = params.GetIteration();

    VerifyOrReturnError(params.GetSalt().size() <= sizeof(device.salt.bytes), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(device.salt.bytes, params.GetSalt().data(), params.GetSalt().size());
    device.salt.size = static_cast<size_t>(params.GetSalt().size());

    VerifyOrReturnError(params.GetVerifier().size() <= sizeof(device.verifier.bytes), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(device.verifier.bytes, params.GetVerifier().data(), params.GetVerifier().size());
    device.verifier.size = static_cast<size_t>(params.GetVerifier().size());

    return OpenCommissioningWindow(device);
}

CHIP_ERROR
CommissionNode(chip::Controller::CommissioningWindowPasscodeParams params, VendorId vendorId, uint16_t productId)
{
    chip_rpc_DeviceCommissioningInfo device;
    device.setup_pin     = params.GetSetupPIN();
    device.discriminator = params.GetDiscriminator();
    device.iterations    = params.GetIteration();
    device.vendor_id     = vendorId;
    device.product_id    = productId;

    VerifyOrReturnError(params.GetSalt().size() <= sizeof(device.salt.bytes), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(device.salt.bytes, params.GetSalt().data(), params.GetSalt().size());
    device.salt.size = static_cast<size_t>(params.GetSalt().size());

    // The RPC call is kept alive until it completes. When a response is received, it will be logged by the handler
    // function and the call will complete.
    auto call = fabricAdminClient.CommissionNode(device, RpcCompletedWithEmptyResponse);

    if (!call.active())
    {
        // The RPC call was not sent. This could occur due to, for example, an invalid channel ID. Handle if necessary.
        return CHIP_ERROR_INTERNAL;
    }

    return WaitForResponse(call);
}

CHIP_ERROR KeepActive(chip::ScopedNodeId scopedNodeId, uint32_t stayActiveDurationMs, uint32_t timeoutMs)
{
    chip_rpc_KeepActiveParameters params;
    params.has_id                  = true;
    params.id.node_id              = scopedNodeId.GetNodeId();
    params.id.fabric_index         = scopedNodeId.GetFabricIndex();
    params.stay_active_duration_ms = stayActiveDurationMs;
    params.timeout_ms              = timeoutMs;

    // The RPC call is kept alive until it completes. When a response is received, it will be logged by the handler
    // function and the call will complete.
    auto call = fabricAdminClient.KeepActive(params, RpcCompletedWithEmptyResponse);

    if (!call.active())
    {
        // The RPC call was not sent. This could occur due to, for example, an invalid channel ID. Handle if necessary.
        return CHIP_ERROR_INTERNAL;
    }

    return WaitForResponse(call);
}
