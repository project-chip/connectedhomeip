/*
 *
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include "pigweed/rpc_services/JointFabric.h"

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

using namespace chip;

namespace joint_fabric_service {

constexpr uint32_t kRpcTimeoutMs = 1000;

// Binary semaphore used to signal that ResponseStream() has delivered the ICAC
// CSR bytes requested by GetICACCSRForJF(). Created on first use.
SemaphoreHandle_t responseSem = nullptr;
bool responseReceived         = false;

uint8_t icacCSRBuf[Crypto::kMIN_CSR_Buffer_Size] = { 0 };
MutableByteSpan icacCSRSpan{ icacCSRBuf };

::pw::Status JointFabric::TransferOwnership(const ::OwnershipContext & request, ::pw_protobuf_Empty & response)
{
    ChipLogProgress(JointFabric, "RPC Ownership Transfer for NodeId: 0x" ChipLogFormatX64 ", jcm=%d",
                    ChipLogValueX64(request.node_id), request.jcm);

    if (request.jcm && (Crypto::kP256_PublicKey_Length != request.trustedIcacPublicKeyB.size))
    {
        ChipLogError(JointFabric, "Invalid ICAC Public Key Size");
        return pw::Status::OutOfRange();
    }

    if (request.jcm && request.peerAdminJFAdminClusterEndpointId == kInvalidEndpointId)
    {
        ChipLogError(JointFabric, "Invalid Peer Admin Endpoint ID for the JF Administrator Cluster");
        return pw::Status::OutOfRange();
    }

    OwnershipTransferContext * data = Platform::New<OwnershipTransferContext>(
        request.node_id, request.jcm, ByteSpan(request.trustedIcacPublicKeyB.bytes, request.trustedIcacPublicKeyB.size),
        request.peerAdminJFAdminClusterEndpointId);
    VerifyOrReturnValue(data, pw::Status::Internal());
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(FinalizeCommissioningWork, reinterpret_cast<intptr_t>(data));

    return pw::OkStatus();
}

void JointFabric::GetStream(const ::pw_protobuf_Empty & request, ServerWriter<::RequestOptions> & writer)
{
    ChipLogProgress(JointFabric, "GetStream Opened");
    rpcGetStream = std::move(writer);

    return;
}

::pw::Status JointFabric::ResponseStream(const ::Response & ICACCSRBytes, ::pw_protobuf_Empty & response)
{
    ChipLogProgress(JointFabric, "RPC ReplyWithICACCSR");

    TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(ByteSpan(ICACCSRBytes.response_bytes.bytes, ICACCSRBytes.response_bytes.size),
                                                   icacCSRSpan);

    responseReceived = true;
    if (responseSem != nullptr)
    {
        xSemaphoreGive(responseSem);
    }

    return pw::OkStatus();
}

CHIP_ERROR JointFabric::GetICACCSRForJF(MutableByteSpan & icacCSR)
{
    if (responseSem == nullptr)
    {
        responseSem = xSemaphoreCreateBinary();
        VerifyOrReturnError(responseSem != nullptr, CHIP_ERROR_NO_MEMORY);
    }

    ::pw::Status status;

    // Drain any stale signal/response left over from a previous request that
    // timed out here but was answered late by ResponseStream(). Without this,
    // the xSemaphoreTake() below could return immediately with a CSR that
    // belongs to the earlier request (potentially a different key).
    responseReceived = false;
    while (xSemaphoreTake(responseSem, 0) == pdTRUE)
    {
        // Discard pending gives from late responses.
    }

    // JFA requests an ICAC CSR from JFC
    RequestOptions requestOptions{ TransactionType::TransactionType_ICAC_CSR };
    status = rpcGetStream.Write(requestOptions);

    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "Writing to GetStream failed");

        return CHIP_ERROR_SHUT_DOWN;
    }

    // wait for the ICAC CSR from JFC
    if (xSemaphoreTake(responseSem, pdMS_TO_TICKS(kRpcTimeoutMs)) == pdTRUE && responseReceived)
    {
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(icacCSRSpan.data(), icacCSRSpan.size()), icacCSR));
        responseReceived = false;

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TIMEOUT;
}

void JointFabric::CloseStreams()
{
    rpcGetStream.Finish();
}

} // namespace joint_fabric_service
