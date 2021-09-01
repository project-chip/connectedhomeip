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

#include <crypto/CHIPCryptoPAL.h>

using namespace chip::Crypto;

namespace chip {
namespace Credentials {

namespace {

// Version to have a default placeholder so the getter never
// returns `nullptr` by default.
class UnimplementedDACVerifier : public DeviceAttestationVerifier
{
public:
    AttestationVerificationResult
    VerifyAttestationInformation(const ByteSpan & attestation_info_buffer, const ByteSpan & attestation_challenge_buffer,
                                 const ByteSpan & attestation_signature_buffer, const ByteSpan & pai_cert_der_buffer,
                                 const ByteSpan & dac_cert_der_buffer, const ByteSpan & attestation_nonce) override
    {
        (void) attestation_info_buffer;
        (void) attestation_challenge_buffer;
        (void) attestation_signature_buffer;
        (void) pai_cert_der_buffer;
        (void) dac_cert_der_buffer;
        (void) attestation_nonce;
        return AttestationVerificationResult::kNotImplemented;
    }
};

// Default to avoid nullptr on getter and cleanly handle new products/clients before
// they provide their own.
UnimplementedDACVerifier gDefaultDACVerifier;

DeviceAttestationVerifier * gDacVerifier = &gDefaultDACVerifier;

} // namespace

CHIP_ERROR DeviceAttestationVerifier::ValidateAttestationData(const P256PublicKey & pubkey, const ByteSpan & attestationElements,
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

} // namespace Credentials
} // namespace chip
