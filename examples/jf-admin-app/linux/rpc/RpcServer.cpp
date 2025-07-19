#include "pigweed/rpc_services/JointFabric.h"

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

namespace joint_fabric_service {

constexpr uint32_t kRpcTimeoutMs = 1000;
std::condition_variable responseCv;
bool responseReceived = false;

uint8_t replyBuf[Credentials::kMaxDERCertLength] = { 0 };
MutableByteSpan replySpan{ replyBuf };

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
        return pw::Status::OutOfRange();
        ChipLogError(JointFabric, "Invalid Peer Admin Endpoint ID for the JF Administrator Cluster");
    }

    for (size_t i = 0; i < Crypto::kP256_PublicKey_Length; ++i)
    {
        ChipLogProgress(JointFabric, "trustedIcacPublicKeyB[%li] = %02X", i, request.trustedIcacPublicKeyB.bytes[i]);
    }

    OwnershipTransferContext * data = Platform::New<OwnershipTransferContext>(
        request.node_id, request.jcm, ByteSpan(request.trustedIcacPublicKeyB.bytes, request.trustedIcacPublicKeyB.size),
        request.peerAdminJFAdminClusterEndpointId);
    VerifyOrReturnValue(data, pw::Status::Internal());
    DeviceLayer::PlatformMgr().ScheduleWork(FinalizeCommissioningWork, reinterpret_cast<intptr_t>(data));

    return pw::OkStatus();
}

void JointFabric::GetStream(const ::pw_protobuf_Empty & request, ServerWriter<::RequestOptions> & writer)
{
    ChipLogProgress(JointFabric, "GetStream Opened");
    rpcGetStream = std::move(writer);

    return;
}

::pw::Status JointFabric::ResponseStream(const ::Response & responseBytes, ::pw_protobuf_Empty & response)
{
    ChipLogProgress(JointFabric, "RPC ReplyWithICACCSR");

    CopySpanToMutableSpan(ByteSpan{ responseBytes.response_bytes.bytes, responseBytes.response_bytes.size }, replySpan);

    responseReceived = true;
    responseCv.notify_one();

    return pw::OkStatus();
}

CHIP_ERROR JointFabric::GetICACCSRForJF(MutableByteSpan & icacCSR)
{
    // JFA requests an ICAC CSR from JFC
    RequestOptions requestOptions{ TransactionType::TransactionType_ICAC_CSR };

    return GetReplyInternal(icacCSR, requestOptions);
}

CHIP_ERROR JointFabric::GetReplyInternal(MutableByteSpan & icacCSR, RequestOptions & requestOptions)
{
    std::mutex responseMutex;
    std::unique_lock<std::mutex> lock(responseMutex);
    ::pw::Status status;

    status = rpcGetStream.Write(requestOptions);

    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "Writing to GetStream failed");
        return CHIP_ERROR_SHUT_DOWN;
    }

    // wait for the ICAC CSR from JFC
    if (responseCv.wait_for(lock, std::chrono::milliseconds(kRpcTimeoutMs), [] { return responseReceived; }))
    {
        CopySpanToMutableSpan(ByteSpan(replySpan.data(), replySpan.size()), icacCSR);
        responseReceived = false;

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TIMEOUT;
}

CHIP_ERROR JointFabric::GetCrossSignedIcacForJF(uint64_t anchorFabricId, ByteSpan & icacCSR, MutableByteSpan & crossSignedICAC)
{

    RequestOptions requestOptions;
    requestOptions.transaction_type = TransactionType::TransactionType_CROSS_SIGNED_ICAC;
    requestOptions.anchor_fabric_id = anchorFabricId;

    if (sizeof(requestOptions.request_bytes.bytes) < icacCSR.size())
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    memcpy(requestOptions.request_bytes.bytes, icacCSR.data(), icacCSR.size());
    requestOptions.request_bytes.size = icacCSR.size();

    return GetReplyInternal(crossSignedICAC, requestOptions);
}

void JointFabric::CloseStreams()
{
    rpcGetStream.Finish();
}

} // namespace joint_fabric_service
