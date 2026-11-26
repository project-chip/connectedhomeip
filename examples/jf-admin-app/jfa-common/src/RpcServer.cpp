#include "pigweed/rpc_services/JointFabric.h"

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

namespace joint_fabric_service {

constexpr uint32_t kRpcTimeoutMs                = 1000;
constexpr uint32_t kCrossSignedICACRpcTimeoutMs = 10000;

std::mutex icacCSRMutex;
std::condition_variable icacCSRCv;
bool icacCSRReceived = false;
uint8_t icacCSRBuf[Crypto::kMIN_CSR_Buffer_Size] = { 0 };
MutableByteSpan icacCSRSpan{ icacCSRBuf };

std::mutex crossSignedICACMutex;
std::condition_variable crossSignedICACCv;
bool crossSignedICACReceived                              = false;
uint8_t crossSignedICACBuf[Credentials::kMaxDERCertLength] = { 0 };
MutableByteSpan crossSignedICACSpan{ crossSignedICACBuf };

std::mutex deviceNOCMutex;
std::condition_variable deviceNOCCv;
bool deviceNOCReceived                              = false;
uint8_t deviceNOCBuf[Credentials::kMaxDERCertLength] = { 0 };
MutableByteSpan deviceNOCSpan{ deviceNOCBuf };
// JF materials returned by JFC-B alongside the device NOC (DEVICE_NOC transaction).
uint8_t deviceICACBuf[Credentials::kMaxDERCertLength] = { 0 };
MutableByteSpan deviceICACSpan{ deviceICACBuf };
uint8_t deviceRCACBuf[Credentials::kMaxDERCertLength] = { 0 };
MutableByteSpan deviceRCACSpan{ deviceRCACBuf };
uint8_t deviceIPKBuf[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0 };
MutableByteSpan deviceIPKSpan{ deviceIPKBuf };


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

::pw::Status JointFabric::ResponseStream(const ::Response & responseBytes, ::pw_protobuf_Empty & response)
{
    ChipLogProgress(JointFabric, "RPC ResponseStream, transaction_type=%d", responseBytes.transaction_type);

    switch (responseBytes.transaction_type)
    {
    case TransactionType::TransactionType_CROSS_SIGNED_ICAC: {
        std::lock_guard<std::mutex> lock(crossSignedICACMutex);
        // Reset the span to full buffer capacity before copying.
        crossSignedICACSpan = MutableByteSpan(crossSignedICACBuf, sizeof(crossSignedICACBuf));
        TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(
            ByteSpan(responseBytes.response_bytes.bytes, responseBytes.response_bytes.size), crossSignedICACSpan);
        crossSignedICACReceived = true;
        crossSignedICACCv.notify_one();
        break;
    }
    case TransactionType::TransactionType_DEVICE_NOC: {
        std::lock_guard<std::mutex> lock(deviceNOCMutex);
        // Reset the spans to full buffer capacity before copying.
        deviceNOCSpan = MutableByteSpan(deviceNOCBuf, sizeof(deviceNOCBuf));
        TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(
            ByteSpan(responseBytes.response_bytes.bytes, responseBytes.response_bytes.size), deviceNOCSpan);
        // JFC-B also returns the JF materials it holds (cross-signed ICAC, Fabric-A RCAC, IPK)
        // so JFA-B can populate the AddTrustedRootCertificate / AddNOC commands.
        deviceICACSpan = MutableByteSpan(deviceICACBuf, sizeof(deviceICACBuf));
        TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(
            ByteSpan(responseBytes.icac_bytes.bytes, responseBytes.icac_bytes.size), deviceICACSpan);
        deviceRCACSpan = MutableByteSpan(deviceRCACBuf, sizeof(deviceRCACBuf));
        TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(
            ByteSpan(responseBytes.rcac_bytes.bytes, responseBytes.rcac_bytes.size), deviceRCACSpan);
        deviceIPKSpan = MutableByteSpan(deviceIPKBuf, sizeof(deviceIPKBuf));
        TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(
            ByteSpan(responseBytes.ipk_bytes.bytes, responseBytes.ipk_bytes.size), deviceIPKSpan);
        deviceNOCReceived = true;
        deviceNOCCv.notify_one();
        break;
    }
    case TransactionType::TransactionType_ICAC_CSR:
    default: {
        std::lock_guard<std::mutex> lock(icacCSRMutex);
        // Reset the span to full buffer capacity before copying.
        icacCSRSpan = MutableByteSpan(icacCSRBuf, sizeof(icacCSRBuf));
        TEMPORARY_RETURN_IGNORED CopySpanToMutableSpan(
            ByteSpan(responseBytes.response_bytes.bytes, responseBytes.response_bytes.size), icacCSRSpan);
        icacCSRReceived = true;
        icacCSRCv.notify_one();
        break;
    }
    }

    return pw::OkStatus();
}

CHIP_ERROR JointFabric::GetICACCSRForJF(MutableByteSpan & icacCSR)
{
    std::unique_lock<std::mutex> lock(icacCSRMutex);
    ::pw::Status status;

    icacCSRReceived = false;

    // JFA requests an ICAC CSR from JFC.
    RequestOptions requestOptions{};
    requestOptions.transaction_type = TransactionType::TransactionType_ICAC_CSR;
    status                          = rpcGetStream.Write(requestOptions);

    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "GetICACCSRForJF: Writing to GetStream failed");
        return CHIP_ERROR_SHUT_DOWN;
    }

    // Wait for the ICAC CSR from JFC.
    if (icacCSRCv.wait_for(lock, std::chrono::milliseconds(kRpcTimeoutMs), [&] { return icacCSRReceived; }))
    {
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(icacCSRSpan.data(), icacCSRSpan.size()), icacCSR));
        icacCSRReceived = false;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TIMEOUT;
}

CHIP_ERROR JointFabric::GetCrossSignedICAC(chip::FabricId anchorFabricId, chip::ByteSpan csr, chip::MutableByteSpan & icac)
{
    std::unique_lock<std::mutex> lock(crossSignedICACMutex);
    ::pw::Status status;

    crossSignedICACReceived = false;

    // Send the commissionee CSR to the JFC for cross-signing.
    RequestOptions requestOptions{};
    requestOptions.transaction_type    = TransactionType::TransactionType_CROSS_SIGNED_ICAC;
    requestOptions.anchor_fabric_id    = anchorFabricId;
    requestOptions.csr_bytes.size      = static_cast<uint16_t>(csr.size());
    memcpy(requestOptions.csr_bytes.bytes, csr.data(), csr.size());
    status = rpcGetStream.Write(requestOptions);

    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "GetCrossSignedICAC: Writing to GetStream failed");
        return CHIP_ERROR_SHUT_DOWN;
    }

    // Wait for the cross-signed ICAC from JFC.
    if (crossSignedICACCv.wait_for(lock, std::chrono::milliseconds(kCrossSignedICACRpcTimeoutMs), [&] { return crossSignedICACReceived; }))
    {
        ReturnErrorOnFailure(
            CopySpanToMutableSpan(ByteSpan(crossSignedICACSpan.data(), crossSignedICACSpan.size()), icac));
        crossSignedICACReceived = false;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TIMEOUT;
}

CHIP_ERROR JointFabric::PushJFMaterials(chip::ByteSpan crossSignedICAC, chip::ByteSpan anchorRCAC, chip::ByteSpan anchorIPK)
{
    VerifyOrReturnError(crossSignedICAC.size() <= sizeof(RequestOptions{}.icac_bytes.bytes), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(anchorRCAC.size() <= sizeof(RequestOptions{}.rcac_bytes.bytes), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(anchorIPK.size() <= sizeof(RequestOptions{}.ipk_bytes.bytes), CHIP_ERROR_BUFFER_TOO_SMALL);

    RequestOptions requestOptions{};
    requestOptions.transaction_type   = TransactionType::TransactionType_JF_MATERIALS;
    requestOptions.icac_bytes.size    = static_cast<uint16_t>(crossSignedICAC.size());
    memcpy(requestOptions.icac_bytes.bytes, crossSignedICAC.data(), crossSignedICAC.size());
    requestOptions.rcac_bytes.size    = static_cast<uint16_t>(anchorRCAC.size());
    memcpy(requestOptions.rcac_bytes.bytes, anchorRCAC.data(), anchorRCAC.size());
    requestOptions.ipk_bytes.size     = static_cast<uint16_t>(anchorIPK.size());
    memcpy(requestOptions.ipk_bytes.bytes, anchorIPK.data(), anchorIPK.size());

    ::pw::Status status = rpcGetStream.Write(requestOptions);
    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "PushJFMaterials: Writing to GetStream failed");
        return CHIP_ERROR_SHUT_DOWN;
    }

    ChipLogProgress(JointFabric, "PushJFMaterials: sent ICAC (%u bytes) + RCAC (%u bytes) + IPK (%u bytes) to JFC-B",
                    static_cast<unsigned>(crossSignedICAC.size()), static_cast<unsigned>(anchorRCAC.size()),
                    static_cast<unsigned>(anchorIPK.size()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabric::ClearJFMaterials()
{
    RequestOptions requestOptions{};
    requestOptions.transaction_type = TransactionType::TransactionType_CLEAR_JF_MATERIALS;
    ::pw::Status status             = rpcGetStream.Write(requestOptions);
    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "ClearJFMaterials: Writing to GetStream failed");
        return CHIP_ERROR_SHUT_DOWN;
    }
    ChipLogProgress(JointFabric, "ClearJFMaterials: sent CLEAR_JF_MATERIALS to JFC-B");
    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabric::GetNOCForDevice(chip::NodeId deviceNodeId, chip::ByteSpan csr, chip::MutableByteSpan & noc,
                                        chip::MutableByteSpan & icac, chip::MutableByteSpan & rcac, chip::MutableByteSpan & ipk)
{
    std::unique_lock<std::mutex> lock(deviceNOCMutex);
    ::pw::Status status;

    deviceNOCReceived = false;

    // Send the device CSR to JFC-B so it can issue a Fabric-A NOC for migrating the device into Fabric A.
    // No fabric id is transmitted; JFC-B derives it internally from the cross-signed ICAC it holds.
    RequestOptions requestOptions{};
    requestOptions.transaction_type = TransactionType::TransactionType_DEVICE_NOC;
    requestOptions.node_id          = deviceNodeId;
    requestOptions.csr_bytes.size   = static_cast<uint16_t>(csr.size());
    memcpy(requestOptions.csr_bytes.bytes, csr.data(), csr.size());
    status = rpcGetStream.Write(requestOptions);

    if (pw::OkStatus() != status)
    {
        ChipLogError(JointFabric, "GetNOCForDevice: Writing to GetStream failed");
        return CHIP_ERROR_SHUT_DOWN;
    }

    // Wait for the device NOC (plus the JF materials) from JFC-B.
    if (deviceNOCCv.wait_for(lock, std::chrono::milliseconds(kCrossSignedICACRpcTimeoutMs), [&] { return deviceNOCReceived; }))
    {
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(deviceNOCSpan.data(), deviceNOCSpan.size()), noc));
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(deviceICACSpan.data(), deviceICACSpan.size()), icac));
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(deviceRCACSpan.data(), deviceRCACSpan.size()), rcac));
        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(deviceIPKSpan.data(), deviceIPKSpan.size()), ipk));
        deviceNOCReceived = false;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_TIMEOUT;
}

void JointFabric::CloseStreams()
{
    rpcGetStream.Finish();
}


} // namespace joint_fabric_service
