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
#include "DacOnlyPartialAttestationVerifier.h"

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

/**
 * The implementation should track DefaultDACVerifier::VerifyAttestationInformation but with the checks
 * disabled that are outlined at the top of DacOnlyPartialAttestationVerifier.h.
 */
void PartialDACVerifier::VerifyAttestationInformation(const DeviceAttestationVerifier::AttestationInfo & info,
                                                      Callback::Callback<OnAttestationInformationVerification> * onCompletion)
{
    AttestationVerificationResult attestationError = AttestationVerificationResult::kSuccess;

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

    // match DAC and PAI VIDs
    {
        VerifyOrExit(ExtractVIDPIDFromX509Cert(info.dacDerBuffer, dacVidPid) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kDacFormatInvalid);
        VerifyOrExit(ExtractVIDPIDFromX509Cert(info.paiDerBuffer, paiVidPid) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kPaiFormatInvalid);
        VerifyOrExit(paiVidPid.mVendorId.HasValue() && paiVidPid.mVendorId == dacVidPid.mVendorId,
                     attestationError = AttestationVerificationResult::kDacVendorIdMismatch);
        VerifyOrExit(dacVidPid.mProductId.HasValue(), attestationError = AttestationVerificationResult::kDacProductIdMismatch);
        if (paiVidPid.mProductId.HasValue())
        {
            VerifyOrExit(paiVidPid.mProductId == dacVidPid.mProductId,
                         attestationError = AttestationVerificationResult::kDacProductIdMismatch);
        }
    }

    {
        P256PublicKey remoteManufacturerPubkey;
        P256ECDSASignature deviceSignature;

        VerifyOrExit(ExtractPubkeyFromX509Cert(info.dacDerBuffer, remoteManufacturerPubkey) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kDacFormatInvalid);

        // Validate overall attestation signature on attestation information
        // SetLength will fail if signature doesn't fit
        VerifyOrExit(deviceSignature.SetLength(info.attestationSignatureBuffer.size()) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kAttestationSignatureInvalidFormat);
        memcpy(deviceSignature.Bytes(), info.attestationSignatureBuffer.data(), info.attestationSignatureBuffer.size());
        VerifyOrExit(ValidateAttestationSignature(remoteManufacturerPubkey, info.attestationElementsBuffer,
                                                  info.attestationChallengeBuffer, deviceSignature) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kAttestationSignatureInvalid);
    }

    {
        MutableByteSpan akid(deviceInfo.paaSKID);

        VerifyOrExit(ExtractAKIDFromX509Cert(info.paiDerBuffer, akid) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kPaiFormatInvalid);

        ChipLogProgress(Support, "PartialDACVerifier::CheckPAA skipping vid-scoped PAA check - PAARootStore disabled");
    }

#if !defined(CURRENT_TIME_NOT_IMPLEMENTED)
    VerifyOrExit(IsCertificateValidAtCurrentTime(info.dacDerBuffer) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kDacExpired);
#endif

    ChipLogProgress(Support, "PartialDACVerifier::CheckCertChain skipping cert chain check - PAARootStore disabled");

    {
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

        ChipLogProgress(
            Support,
            "PartialDACVerifier::VerifyAttestationInformation skipping PAA subject key id extraction - PAARootStore disabled");

        VerifyOrExit(DeconstructAttestationElements(info.attestationElementsBuffer, certificationDeclarationSpan,
                                                    attestationNonceSpan, timestampDeconstructed, firmwareInfoSpan,
                                                    vendorReserved) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kAttestationElementsMalformed);

        // Verify that Nonce matches with what we sent
        VerifyOrExit(attestationNonceSpan.data_equal(info.attestationNonceBuffer),
                     attestationError = AttestationVerificationResult::kAttestationNonceMismatch);

        ChipLogProgress(Support,
                        "PartialDACVerifier::VerifyAttestationInformation skipping CD signature check - LocalCSAStore disabled");
        VerifyOrExit(CMS_ExtractCDContent(certificationDeclarationSpan, certificationDeclarationPayload) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kPaaFormatInvalid);

        attestationError = ValidateCertificateDeclarationPayload(certificationDeclarationPayload, firmwareInfoSpan, deviceInfo);
        VerifyOrExit(attestationError == AttestationVerificationResult::kSuccess, attestationError = attestationError);
    }

exit:
    onCompletion->mCall(onCompletion->mContext, info, attestationError);
}

} // namespace Credentials
} // namespace chip
