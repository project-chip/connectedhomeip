#include "pigweed/rpc_services/JointFabric.h"

#include "JFAResponseSignal.h"

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

namespace joint_fabric_service {

constexpr uint32_t kRpcTimeoutMs = 1000;

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

    JFASignalResponse();

    return pw::OkStatus();
}

CHIP_ERROR JointFabric::GetICACCSRForJF(MutableByteSpan & icacCSR)
{
    ::pw::Status status;

    JFAResetResponseSignal();

    // JFA requests an ICAC CSR from JFC
    RequestOptions requestOptions{ TransactionType::TransactionType_ICAC_CSR };
    status = rpcGetStream.Write(requestOptions);

    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "Writing to GetStream failed");
        return CHIP_ERROR_SHUT_DOWN;
    }

    // wait for the ICAC CSR from JFC
    if (JFAWaitResponse(kRpcTimeoutMs))
    {
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(icacCSRSpan.data(), icacCSRSpan.size()), icacCSR));

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TIMEOUT;
}

void JointFabric::CloseStreams()
{
    rpcGetStream.Finish();
}

} // namespace joint_fabric_service
