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
};

// Default to avoid nullptr on getter and cleanly handle new products/clients before
// they provide their own.
UnimplementedDACVerifier gDefaultDACVerifier;

DeviceAttestationVerifier * gDacVerifier = &gDefaultDACVerifier;

} // namespace

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
    mPaiDerBuffer(CopyByteSpanHelper(attestationInfo.paiDerBuffer)), mDacDerBuffer(CopyByteSpanHelper(attestationInfo.dacDerBuffer))
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
