/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include "CloudDeviceAttestationVerifier.h"

#include <controller/OperationalCredentialsDelegate.h>
#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationVendorReserved.h>
#include <crypto/CHIPCryptoPAL.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

using namespace chip::Crypto;

namespace chip {
namespace Credentials {

// As per specifications section 11.22.5.1. Constant RESP_MAX
constexpr size_t kMaxResponseLength = 900;

AttestationVerificationResult CloudDACVerifier::CheckPAA(const DeviceAttestationVerifier::AttestationInfo & info,
                                                         DeviceInfoForAttestation & deviceInfo,
                                                         Platform::ScopedMemoryBuffer<uint8_t> & paaCert,
                                                         MutableByteSpan & paaDerBuffer, AttestationCertVidPid & paaVidPid,
                                                         AttestationCertVidPid & paiVidPid)
{
    AttestationVerificationResult attestationError = AttestationVerificationResult::kSuccess;
    MutableByteSpan akid(deviceInfo.paaSKID);

    VerifyOrExit(ExtractAKIDFromX509Cert(info.paiDerBuffer, akid) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kPaiFormatInvalid);

    ChipLogProgress(Support, "CloudDACVerifier::CheckPAA skipping vid-scoped PAA check - PAARootStore disabled");

exit:
    return attestationError;
}

AttestationVerificationResult CloudDACVerifier::CheckCertTimes(const DeviceAttestationVerifier::AttestationInfo & info,
                                                               MutableByteSpan & paaDerBuffer)
{
    AttestationVerificationResult attestationError = AttestationVerificationResult::kSuccess;

#if !defined(CURRENT_TIME_NOT_IMPLEMENTED)
    VerifyOrExit(IsCertificateValidAtCurrentTime(info.dacDerBuffer) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kDacExpired);
#endif

    VerifyOrExit(IsCertificateValidAtIssuance(info.dacDerBuffer, info.paiDerBuffer) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kPaiExpired);

    ChipLogProgress(Support, "CloudDACVerifier::CheckCertTimes skipping PAA expiry check - PAARootStore disabled");

exit:
    return attestationError;
}

AttestationVerificationResult CloudDACVerifier::CheckCertChain(const DeviceAttestationVerifier::AttestationInfo & info,
                                                               MutableByteSpan & paaDerBuffer)
{
    ChipLogProgress(Support, "CloudDACVerifier::CheckCertChain skipping cert chain check - PAARootStore disabled");

    return AttestationVerificationResult::kSuccess;
}

AttestationVerificationResult
CloudDACVerifier::CheckCertDeclaration(const DeviceAttestationVerifier::AttestationInfo & info, MutableByteSpan & paaDerBuffer,
                                       AttestationCertVidPid & dacVidPid, AttestationCertVidPid & paiVidPid,
                                       AttestationCertVidPid & paaVidPid, DeviceInfoForAttestation & deviceInfo)
{
    AttestationVerificationResult attestationError = AttestationVerificationResult::kSuccess;

    ByteSpan certificationDeclarationSpan;
    ByteSpan attestationNonceSpan;
    uint32_t timestampDeconstructed;
    ByteSpan firmwareInfoSpan;
    DeviceAttestationVendorReservedDeconstructor vendorReserved;
    ByteSpan certificationDeclarationPayload;

    deviceInfo.dacVendorId  = dacVidPid.mVendorId.Value();
    deviceInfo.dacProductId = dacVidPid.mProductId.Value();
    deviceInfo.paiVendorId  = paiVidPid.mVendorId.Value();
    deviceInfo.paiProductId = paiVidPid.mProductId.ValueOr(0);
    deviceInfo.paaVendorId  = paaVidPid.mVendorId.ValueOr(VendorId::NotSpecified);

    VerifyOrExit(DeconstructAttestationElements(info.attestationElementsBuffer, certificationDeclarationSpan, attestationNonceSpan,
                                                timestampDeconstructed, firmwareInfoSpan, vendorReserved) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kAttestationElementsMalformed);

    // Verify that Nonce matches with what we sent
    VerifyOrExit(attestationNonceSpan.data_equal(info.attestationNonceBuffer),
                 attestationError = AttestationVerificationResult::kAttestationNonceMismatch);

    ChipLogProgress(Support, "CloudDACVerifier::VerifyAttestationInformation skipping CD signature check - LocalCSAStore disabled");
    VerifyOrExit(CMS_ExtractCDContent(certificationDeclarationSpan, certificationDeclarationPayload) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kPaaFormatInvalid);

    attestationError = ValidateCertificateDeclarationPayload(certificationDeclarationPayload, firmwareInfoSpan, deviceInfo);
    VerifyOrExit(attestationError == AttestationVerificationResult::kSuccess, attestationError = attestationError);
exit:
    return attestationError;
}

void CloudDACVerifier::VerifyAttestationInformation(const DeviceAttestationVerifier::AttestationInfo & info,
                                                    Callback::Callback<OnAttestationInformationVerification> * onCompletion)
{
    AttestationVerificationResult attestationError = AttestationVerificationResult::kSuccess;

    Platform::ScopedMemoryBuffer<uint8_t> paaCert;
    MutableByteSpan paaDerBuffer;
    AttestationCertVidPid dacVidPid;
    AttestationCertVidPid paiVidPid;
    AttestationCertVidPid paaVidPid;

    DeviceInfoForAttestation deviceInfo{
        .vendorId  = info.vendorId,
        .productId = info.productId,
    };

    VerifyOrExit(!info.attestationElementsBuffer.empty() && !info.attestationChallengeBuffer.empty() &&
                     !info.attestationSignatureBuffer.empty() && !info.paiDerBuffer.empty() && !info.dacDerBuffer.empty() &&
                     !info.attestationNonceBuffer.empty() && onCompletion != nullptr,
                 attestationError = AttestationVerificationResult::kInvalidArgument);

    VerifyOrExit(info.attestationElementsBuffer.size() <= kMaxResponseLength,
                 attestationError = AttestationVerificationResult::kInvalidArgument);

    attestationError = CheckDacPaiVidPids(info, dacVidPid, paiVidPid);
    VerifyOrExit(attestationError == AttestationVerificationResult::kSuccess, attestationError = attestationError);

    attestationError = CheckAttestationSignature(info);
    VerifyOrExit(attestationError == AttestationVerificationResult::kSuccess, attestationError = attestationError);

    attestationError = CheckPAA(info, deviceInfo, paaCert, paaDerBuffer, paaVidPid, paiVidPid);
    VerifyOrExit(attestationError == AttestationVerificationResult::kSuccess, attestationError = attestationError);

    attestationError = CheckCertTimes(info, paaDerBuffer);
    VerifyOrExit(attestationError == AttestationVerificationResult::kSuccess, attestationError = attestationError);

    attestationError = CheckCertChain(info, paaDerBuffer);
    VerifyOrExit(attestationError == AttestationVerificationResult::kSuccess, attestationError = attestationError);

    attestationError = CheckCertDeclaration(info, paaDerBuffer, dacVidPid, paiVidPid, paaVidPid, deviceInfo);
    VerifyOrExit(attestationError == AttestationVerificationResult::kSuccess, attestationError = attestationError);

exit:
    onCompletion->mCall(onCompletion->mContext, attestationError); // TODO: is this check getting done?
}

} // namespace Credentials
} // namespace chip
