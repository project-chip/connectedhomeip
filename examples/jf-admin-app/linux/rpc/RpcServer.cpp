#include "pigweed/rpc_services/JointFabric.h"

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

namespace joint_fabric_service {

constexpr uint32_t kRpcTimeoutMs = 1000;
std::condition_variable responseCv;
bool responseReceived = false;

uint8_t icacCSRBuf[Crypto::kMIN_CSR_Buffer_Size] = { 0 };
MutableByteSpan icacCSRSpan{ icacCSRBuf };

::pw::Status JointFabric::TransferOwnership(const ::OwnershipContext & request, ::pw_protobuf_Empty & response)
{
    ChipLogProgress(JointFabric, "RPC Ownership Transfer for NodeId: 0x" ChipLogFormatX64 ", jcm=%d",
                    ChipLogValueX64(request.node_id), request.jcm);

    if (request.jcm && (Crypto::kP256_PublicKey_Length != request.trustedIcacPublicKeyB.size))
    {
        return pw::Status::OutOfRange();
        ChipLogError(JointFabric, "Invalid ICAC Public Key Size");
    }

    for (size_t i = 0; i < Crypto::kP256_PublicKey_Length; ++i)
    {
        ChipLogProgress(JointFabric, "trustedIcacPublicKeyB[%li] = %02X", i, request.trustedIcacPublicKeyB.bytes[i]);
    }

    OwnershipTransferContext * data = Platform::New<OwnershipTransferContext>(
        request.node_id, request.jcm, ByteSpan(request.trustedIcacPublicKeyB.bytes, request.trustedIcacPublicKeyB.size));
    VerifyOrReturnValue(data, pw::Status::Internal());
    DeviceLayer::PlatformMgr().ScheduleWork(FinalizeCommissioningWork, reinterpret_cast<intptr_t>(data));

    return pw::OkStatus();
}

void JointFabric::GetICACCSR(const ::pw_protobuf_Empty & request, ServerWriter<::ICACCSROptions> & writer)
{
    ChipLogProgress(JointFabric, "GetICACCSR Stream Opened");
    rpcStreamGetICACCSR = std::move(writer);

    return;
}

::pw::Status JointFabric::ReplyWithICACCSR(const ::ICACCSR & ICACCSRBytes, ::pw_protobuf_Empty & response)
{
    ChipLogProgress(JointFabric, "RPC ReplyWithICACCSR");

    CopySpanToMutableSpan(ByteSpan(ICACCSRBytes.csr.bytes, ICACCSRBytes.csr.size), icacCSRSpan);

    responseReceived = true;
    responseCv.notify_one();

    return pw::OkStatus();
}

CHIP_ERROR JointFabric::GetICACCSRForJF(uint64_t anchorFabricId, MutableByteSpan & icacCSR)
{
    std::mutex responseMutex;
    std::unique_lock<std::mutex> lock(responseMutex);
    ::pw::Status status;

    // JFA requests an ICAC CSR from JFC
    ICACCSROptions icaccsrOptions{ anchorFabricId };
    status = rpcStreamGetICACCSR.Write(icaccsrOptions);

    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "Writing to RPC stream GetICACCSR failed");
        return CHIP_ERROR_SHUT_DOWN;
    }

    // wait for the ICAC CSR from JFC
    if (responseCv.wait_for(lock, std::chrono::milliseconds(kRpcTimeoutMs), [] { return responseReceived; }))
    {
        CopySpanToMutableSpan(ByteSpan(icacCSRSpan.data(), icacCSRSpan.size()), icacCSR);
        responseReceived = false;

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TIMEOUT;
}

void JointFabric::CloseStreams()
{
    rpcStreamGetICACCSR.Finish();
}

} // namespace joint_fabric_service
