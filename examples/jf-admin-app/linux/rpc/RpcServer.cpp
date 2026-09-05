#include "pigweed/rpc_services/JointFabric.h"

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <credentials/CHIPCert.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::Credentials;

namespace joint_fabric_service {

constexpr uint32_t kRpcTimeoutMs = 1000;
std::condition_variable responseCv;
bool responseReceived = false;

// Buffer for ICAC_CSR transaction responses
uint8_t icacCSRBuf[Crypto::kMIN_CSR_Buffer_Size] = { 0 };
MutableByteSpan icacCSRSpan{ icacCSRBuf };

// Buffer for CROSS_SIGNED_ICAC transaction responses
uint8_t crossSignedICACBuf[Credentials::kMaxDERCertLength] = { 0 };
MutableByteSpan crossSignedICACSpan{ crossSignedICACBuf };

// Track which transaction type the last response belongs to
TransactionType lastResponseTransactionType = TransactionType::TransactionType_ICAC_CSR;

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

::pw::Status JointFabric::ResponseStream(const ::Response & responseBytes, ::pw_protobuf_Empty & response)
{
    ChipLogProgress(JointFabric, "RPC ResponseStream: transaction_type=%d", responseBytes.transaction_type);

    lastResponseTransactionType = responseBytes.transaction_type;

    switch (responseBytes.transaction_type)
    {
    case TransactionType::TransactionType_ICAC_CSR:
        TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(
            ByteSpan(responseBytes.response_bytes.bytes, responseBytes.response_bytes.size), icacCSRSpan);
        break;
    case TransactionType::TransactionType_CROSS_SIGNED_ICAC:
        TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(
            ByteSpan(responseBytes.response_bytes.bytes, responseBytes.response_bytes.size), crossSignedICACSpan);
        break;
    default:
        ChipLogError(JointFabric, "ResponseStream: unknown transaction type %d", responseBytes.transaction_type);
        return pw::Status::InvalidArgument();
    }

    responseReceived = true;
    responseCv.notify_one();

    return pw::OkStatus();
}

CHIP_ERROR JointFabric::GetICACCSRForJF(MutableByteSpan & icacCSR)
{
    std::mutex responseMutex;
    std::unique_lock<std::mutex> lock(responseMutex);
    ::pw::Status status;

    // JFA requests an ICAC CSR from JFC
    RequestOptions requestOptions;
    requestOptions.transaction_type = TransactionType::TransactionType_ICAC_CSR;
    requestOptions.anchor_fabric_id = 0;
    status                          = rpcGetStream.Write(requestOptions);

    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "Writing to GetStream failed");
        return CHIP_ERROR_SHUT_DOWN;
    }

    // wait for the ICAC CSR from JFC
    if (responseCv.wait_for(lock, std::chrono::milliseconds(kRpcTimeoutMs), [] { return responseReceived; }))
    {
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(icacCSRSpan.data(), icacCSRSpan.size()), icacCSR));
        responseReceived = false;

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TIMEOUT;
}

CHIP_ERROR JointFabric::GetCrossSignedICAC(FabricId anchorFabricId, ByteSpan icacCSR, MutableByteSpan & crossSignedICAC)
{
    std::mutex responseMutex;
    std::unique_lock<std::mutex> lock(responseMutex);
    ::pw::Status status;

    // Reset the cross-signed ICAC buffer and copy the CSR into the request context
    // The proto RequestOptions carries anchor_fabric_id; the CSR itself is already known
    // to JFC from the earlier ICAC_CSR exchange (it generated the key pair for it).
    // We pass anchor_fabric_id so JFC can embed the correct matter-fabric-id in the ICAC Subject.
    crossSignedICACSpan = MutableByteSpan(crossSignedICACBuf, sizeof(crossSignedICACBuf));

    RequestOptions requestOptions;
    requestOptions.transaction_type = TransactionType::TransactionType_CROSS_SIGNED_ICAC;
    requestOptions.anchor_fabric_id = anchorFabricId;
    status                          = rpcGetStream.Write(requestOptions);

    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "GetCrossSignedICAC: writing CROSS_SIGNED_ICAC request to GetStream failed");
        return CHIP_ERROR_SHUT_DOWN;
    }

    // Wait for the cross-signed ICAC from JFC
    if (responseCv.wait_for(lock, std::chrono::milliseconds(kRpcTimeoutMs), [] { return responseReceived; }))
    {
        if (lastResponseTransactionType != TransactionType::TransactionType_CROSS_SIGNED_ICAC)
        {
            ChipLogError(JointFabric, "GetCrossSignedICAC: unexpected response transaction type %d", lastResponseTransactionType);
            responseReceived = false;
            return CHIP_ERROR_WRONG_ORDER;
        }

        ReturnErrorOnFailure(
            CopySpanToMutableSpan(ByteSpan(crossSignedICACSpan.data(), crossSignedICACSpan.size()), crossSignedICAC));
        responseReceived = false;

        ChipLogProgress(JointFabric, "GetCrossSignedICAC: received cross-signed ICAC (%u bytes)",
                        static_cast<unsigned>(crossSignedICAC.size()));
        return CHIP_NO_ERROR;
    }

    ChipLogError(JointFabric, "GetCrossSignedICAC: timed out waiting for cross-signed ICAC");
    return CHIP_ERROR_TIMEOUT;
}

void JointFabric::CloseStreams()
{
    rpcGetStream.Finish();
}

} // namespace joint_fabric_service
