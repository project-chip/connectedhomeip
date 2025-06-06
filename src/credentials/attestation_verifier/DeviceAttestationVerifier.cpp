/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "DeviceAttestationVerifier.h"

#include <credentials/DeviceAttestationConstructor.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CHIPMem.h>

using namespace chip::Crypto;

namespace chip {
namespace Credentials {

namespace {

// Version to have a default placeholder so the getter never
// returns `nullptr` by default.
class UnimplementedDACVerifier : public DeviceAttestationVerifier
{
public:
    void VerifyAttestationInformation(const AttestationInfo & info,
                                      Callback::Callback<OnAttestationInformationVerification> * onCompletion) override
    {
        (void) info;
        (void) onCompletion;
    }

    AttestationVerificationResult ValidateCertificationDeclarationSignature(const ByteSpan & cmsEnvelopeBuffer,
                                                                            ByteSpan & certDeclBuffer) override
    {
        (void) cmsEnvelopeBuffer;
        (void) certDeclBuffer;
        return AttestationVerificationResult::kNotImplemented;
    }

    AttestationVerificationResult ValidateCertificateDeclarationPayload(const ByteSpan & certDeclBuffer,
                                                                        const ByteSpan & firmwareInfo,
                                                                        const DeviceInfoForAttestation & deviceInfo) override
    {
        (void) certDeclBuffer;
        (void) firmwareInfo;
        (void) deviceInfo;
        return AttestationVerificationResult::kNotImplemented;
    }

    CHIP_ERROR VerifyNodeOperationalCSRInformation(const ByteSpan & nocsrElementsBuffer,
                                                   const ByteSpan & attestationChallengeBuffer,
                                                   const ByteSpan & attestationSignatureBuffer,
                                                   const Crypto::P256PublicKey & dacPublicKey, const ByteSpan & csrNonce) override
    {
        (void) nocsrElementsBuffer;
        (void) attestationChallengeBuffer;
        (void) attestationSignatureBuffer;
        (void) dacPublicKey;
        (void) csrNonce;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    void CheckForRevokedDACChain(const AttestationInfo & info,
                                 Callback::Callback<OnAttestationInformationVerification> * onCompletion) override
    {
        (void) info;
        (void) onCompletion;
        VerifyOrDie(false);
    }
};

// Default to avoid nullptr on getter and cleanly handle new products/clients before
// they provide their own.
UnimplementedDACVerifier gDefaultDACVerifier;

DeviceAttestationVerifier * gDacVerifier = &gDefaultDACVerifier;

} // namespace

struct AttestationVerificationResultDescription
{
    AttestationVerificationResultDescription(AttestationVerificationResult theResultCode, const char * theDescription) :
        resultCode(theResultCode), description(theDescription)
    {}

    AttestationVerificationResult resultCode;
    const char * description;
};

const char * GetAttestationResultDescription(AttestationVerificationResult resultCode)
{
    static const AttestationVerificationResultDescription kAttestationVerificationResultDescriptions[] = {
        { AttestationVerificationResult::kSuccess, "Success" },

        { AttestationVerificationResult::kPaaUntrusted, "PAA is untrusted (OBSOLETE: consider using a different error)" },
        { AttestationVerificationResult::kPaaNotFound, "PAA not found in DCL and/or local PAA trust store" },
        { AttestationVerificationResult::kPaaExpired, "PAA is expired" },
        { AttestationVerificationResult::kPaaSignatureInvalid, "PAA signature is invalid" },
        { AttestationVerificationResult::kPaaRevoked, "PAA is revoked (consider removing from DCL or PAA trust store!)" },
        { AttestationVerificationResult::kPaaFormatInvalid, "PAA format is invalid" },
        { AttestationVerificationResult::kPaaArgumentInvalid, "PAA argument is invalid in some way according to X.509 backend" },

        { AttestationVerificationResult::kPaiExpired, "PAI is expired" },
        { AttestationVerificationResult::kPaiSignatureInvalid, "PAI signature is invalid" },
        { AttestationVerificationResult::kPaiRevoked, "PAI is revoked" },
        { AttestationVerificationResult::kPaiFormatInvalid, "PAI format is invalid" },
        { AttestationVerificationResult::kPaiArgumentInvalid, "PAI argument is invalid in some way according to X.509 backend" },
        { AttestationVerificationResult::kPaiVendorIdMismatch, "PAI vendor ID mismatch (did not match VID present in PAA)" },
        { AttestationVerificationResult::kPaiAuthorityNotFound,
          "PAI authority not found (OBSOLETE: consider using a different error)" },
        { AttestationVerificationResult::kPaiMissing, "PAI is missing/empty from attestation information data" },
        { AttestationVerificationResult::kPaiAndDacRevoked, "Both PAI and DAC are revoked" },

        { AttestationVerificationResult::kDacExpired, "DAC is expired" },
        { AttestationVerificationResult::kDacSignatureInvalid, "DAC signature is invalid" },
        { AttestationVerificationResult::kDacRevoked, "DAC is revoked" },
        { AttestationVerificationResult::kDacFormatInvalid, "DAC format is invalid" },
        { AttestationVerificationResult::kDacArgumentInvalid, "DAC  is invalid in some way according to X.509 backend" },
        { AttestationVerificationResult::kDacVendorIdMismatch,
          "DAC vendor ID mismatch (either between DAC and PAI, or between DAC and Basic Information cluster)" },
        { AttestationVerificationResult::kDacProductIdMismatch,
          "DAC product ID mismatch (either between DAC and PAI, or between DAC and Basic Information cluster)" },
        { AttestationVerificationResult::kDacAuthorityNotFound,
          "DAC authority not found (OBSOLETE: consider using a different error)" },

        { AttestationVerificationResult::kFirmwareInformationMismatch, "Firmware information mismatch" },
        { AttestationVerificationResult::kFirmwareInformationMissing, "Firmware information missing" },

        { AttestationVerificationResult::kAttestationSignatureInvalid,
          "Attestation signature failed to validate against DAC subject public key" },
        { AttestationVerificationResult::kAttestationElementsMalformed, "Attestation elements payload is malformed" },
        { AttestationVerificationResult::kAttestationNonceMismatch, "Attestation nonce mismatches one from Attestation Request" },
        { AttestationVerificationResult::kAttestationSignatureInvalidFormat,
          "Attestation signature format is invalid (likely wrong signature algorithm in certificate)" },

        { AttestationVerificationResult::kCertificationDeclarationNoKeyId,
          "Certification declaration missing the required key ID in CMS envelope" },
        { AttestationVerificationResult::kCertificationDeclarationNoCertificateFound,
          "Could not find matching trusted verification certificate for the certification declaration's key ID" },
        { AttestationVerificationResult::kCertificationDeclarationInvalidSignature,
          "Certification declaration signature failed to validate against the verification certificate" },
        { AttestationVerificationResult::kCertificationDeclarationInvalidFormat, "Certification declaration format is invalid" },
        { AttestationVerificationResult::kCertificationDeclarationInvalidVendorId,
          "Certification declaration vendor ID failed to cross-reference with DAC and/or PAI and/or Basic Information cluster" },
        { AttestationVerificationResult::kCertificationDeclarationInvalidProductId,
          "Certification declaration product ID failed to cross-reference with DAC and/or PAI and/or Basic Information cluster" },
        { AttestationVerificationResult::kCertificationDeclarationInvalidPAA,
          "Certification declaration required a fixed allowed PAA which mismatches the final PAA found" },

        { AttestationVerificationResult::kNoMemory, "Failed to allocate memory to process attestation verification" },
        { AttestationVerificationResult::kInvalidArgument,
          "Some unexpected invalid argument was provided internally to the device attestation procedure (likely malformed input "
          "data from candidate device)" },
        { AttestationVerificationResult::kInternalError,
          "An internal error error arose in the device attestation procedure (likely malformed input data from candidate device)" },
        { AttestationVerificationResult::kNotImplemented,
          "Reached a critical-but-unimplemented part of the device attestation procedure!" }
    };

    for (const auto & desc : kAttestationVerificationResultDescriptions)
    {
        if (desc.resultCode == resultCode)
        {
            return desc.description;
        }
    }
    return "<AttestationVerificationResult does not have a description!>";
}

CHIP_ERROR DeviceAttestationVerifier::ValidateAttestationSignature(const P256PublicKey & pubkey,
                                                                   const ByteSpan & attestationElements,
                                                                   const ByteSpan & attestationChallenge,
                                                                   const P256ECDSASignature & signature)
{
    Hash_SHA256_stream hashStream;
    uint8_t md[kSHA256_Hash_Length];
    MutableByteSpan messageDigestSpan(md);

    ReturnErrorOnFailure(hashStream.Begin());
    ReturnErrorOnFailure(hashStream.AddData(attestationElements));
    ReturnErrorOnFailure(hashStream.AddData(attestationChallenge));
    ReturnErrorOnFailure(hashStream.Finish(messageDigestSpan));

    ReturnErrorOnFailure(pubkey.ECDSA_validate_hash_signature(messageDigestSpan.data(), messageDigestSpan.size(), signature));

    return CHIP_NO_ERROR;
}

DeviceAttestationVerifier * GetDeviceAttestationVerifier()
{
    return gDacVerifier;
}

void SetDeviceAttestationVerifier(DeviceAttestationVerifier * verifier)
{
    if (verifier == nullptr)
    {
        return;
    }

    gDacVerifier = verifier;
}

static inline Platform::ScopedMemoryBufferWithSize<uint8_t> CopyByteSpanHelper(const ByteSpan & span_to_copy)
{
    Platform::ScopedMemoryBufferWithSize<uint8_t> bufferCopy;
    if (bufferCopy.Alloc(span_to_copy.size()))
    {
        memcpy(bufferCopy.Get(), span_to_copy.data(), span_to_copy.size());
    }
    return bufferCopy;
}

DeviceAttestationVerifier::AttestationDeviceInfo::AttestationDeviceInfo(const AttestationInfo & attestationInfo) :
    mPaiDerBuffer(CopyByteSpanHelper(attestationInfo.paiDerBuffer)),
    mDacDerBuffer(CopyByteSpanHelper(attestationInfo.dacDerBuffer)), mBasicInformationVendorId(attestationInfo.vendorId),
    mBasicInformationProductId(attestationInfo.productId)
{
    ByteSpan certificationDeclarationSpan;
    ByteSpan attestationNonceSpan;
    uint32_t timestampDeconstructed;
    ByteSpan firmwareInfoSpan;
    DeviceAttestationVendorReservedDeconstructor vendorReserved;

    if (DeconstructAttestationElements(attestationInfo.attestationElementsBuffer, certificationDeclarationSpan,
                                       attestationNonceSpan, timestampDeconstructed, firmwareInfoSpan,
                                       vendorReserved) == CHIP_NO_ERROR)
    {
        mCdBuffer = CopyByteSpanHelper(certificationDeclarationSpan);
    }
}

} // namespace Credentials
} // namespace chip
