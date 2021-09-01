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
#include "DeviceAttestationVerifierExample.h"

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <support/ScopedBuffer.h>

using namespace chip::Crypto;

namespace chip {
namespace Credentials {
namespace Examples {

namespace {

// Helper to do common logic to all providers
CHIP_ERROR CopySpanToMutableSpan(ByteSpan span_to_copy, MutableByteSpan & out_buf)
{
    VerifyOrReturnError(IsSpanUsable(span_to_copy), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buf.size() >= span_to_copy.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(out_buf.data(), span_to_copy.data(), span_to_copy.size());
    out_buf.reduce_size(span_to_copy.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR GetProductAttestationAuthorityCert(const ByteSpan & skid, MutableByteSpan & out_dac_buffer)
{
    /*
    credentials/test/attestation/Chip-Test-PAA-FFF1-Cert.pem
    -----BEGIN CERTIFICATE-----
    MIIBmTCCAT+gAwIBAgIIaDhPq7kZ/N8wCgYIKoZIzj0EAwIwHzEdMBsGA1UEAwwU
    TWF0dGVyIFRlc3QgUEFBIEZGRjEwIBcNMjEwNjI4MTQyMzQzWhgPOTk5OTEyMzEy
    MzU5NTlaMB8xHTAbBgNVBAMMFE1hdHRlciBUZXN0IFBBQSBGRkYxMFkwEwYHKoZI
    zj0CAQYIKoZIzj0DAQcDQgAEG5isW7wR3GoXVaBbCsXha6AsRu5vwrvnb/fPbKeq
    Tp/R15jcvvtP6uIl03c8kTSMwm1JMTHjCWMtXp7zHRLek6NjMGEwDwYDVR0TAQH/
    BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFO8Y4OzUZgQ03w28kR7U
    UhaZZoOfMB8GA1UdIwQYMBaAFO8Y4OzUZgQ03w28kR7UUhaZZoOfMAoGCCqGSM49
    BAMCA0gAMEUCIQCn+l+nZv/3tf0VjNNPYl1IkSAOBYUO8SX23udWVPmXNgIgI7Ub
    bkJTKCjbCZIDNwUNcPC2tyzNPLeB5nGsIl31Rys=
    -----END CERTIFICATE-----
    */
    struct PAALookupTable
    {
        const uint8_t mPAACertificate[kMax_x509_Certificate_Length];
        const size_t mPAACertificateLen;
        const uint8_t mSKID[kKeyIdentifierLength];
    };

    static PAALookupTable
        sPAALookupTable[] = {
            { { 0x30, 0x82, 0x01, 0x99, 0x30, 0x82, 0x01, 0x3F, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x68, 0x38, 0x4F, 0xAB,
                0xB9, 0x19, 0xFC, 0xDF, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x1F, 0x31,
                0x1D, 0x30, 0x1B, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x14, 0x4D, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65,
                0x73, 0x74, 0x20, 0x50, 0x41, 0x41, 0x20, 0x46, 0x46, 0x46, 0x31, 0x30, 0x20, 0x17, 0x0D, 0x32, 0x31, 0x30, 0x36,
                0x32, 0x38, 0x31, 0x34, 0x32, 0x33, 0x34, 0x33, 0x5A, 0x18, 0x0F, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32, 0x33, 0x31,
                0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5A, 0x30, 0x1F, 0x31, 0x1D, 0x30, 0x1B, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C,
                0x14, 0x4D, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x50, 0x41, 0x41, 0x20, 0x46, 0x46,
                0x46, 0x31, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86,
                0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x1B, 0x98, 0xAC, 0x5B, 0xBC, 0x11, 0xDC, 0x6A, 0x17,
                0x55, 0xA0, 0x5B, 0x0A, 0xC5, 0xE1, 0x6B, 0xA0, 0x2C, 0x46, 0xEE, 0x6F, 0xC2, 0xBB, 0xE7, 0x6F, 0xF7, 0xCF, 0x6C,
                0xA7, 0xAA, 0x4E, 0x9F, 0xD1, 0xD7, 0x98, 0xDC, 0xBE, 0xFB, 0x4F, 0xEA, 0xE2, 0x25, 0xD3, 0x77, 0x3C, 0x91, 0x34,
                0x8C, 0xC2, 0x6D, 0x49, 0x31, 0x31, 0xE3, 0x09, 0x63, 0x2D, 0x5E, 0x9E, 0xF3, 0x1D, 0x12, 0xDE, 0x93, 0xA3, 0x63,
                0x30, 0x61, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x1D, 0x13, 0x01, 0x01, 0xFF, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xFF,
                0x30, 0x0E, 0x06, 0x03, 0x55, 0x1D, 0x0F, 0x01, 0x01, 0xFF, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x1D, 0x06,
                0x03, 0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0xEF, 0x18, 0xE0, 0xEC, 0xD4, 0x66, 0x04, 0x34, 0xDF, 0x0D, 0xBC,
                0x91, 0x1E, 0xD4, 0x52, 0x16, 0x99, 0x66, 0x83, 0x9F, 0x30, 0x1F, 0x06, 0x03, 0x55, 0x1D, 0x23, 0x04, 0x18, 0x30,
                0x16, 0x80, 0x14, 0xEF, 0x18, 0xE0, 0xEC, 0xD4, 0x66, 0x04, 0x34, 0xDF, 0x0D, 0xBC, 0x91, 0x1E, 0xD4, 0x52, 0x16,
                0x99, 0x66, 0x83, 0x9F, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00,
                0x30, 0x45, 0x02, 0x21, 0x00, 0xA7, 0xFA, 0x5F, 0xA7, 0x66, 0xFF, 0xF7, 0xB5, 0xFD, 0x15, 0x8C, 0xD3, 0x4F, 0x62,
                0x5D, 0x48, 0x91, 0x20, 0x0E, 0x05, 0x85, 0x0E, 0xF1, 0x25, 0xF6, 0xDE, 0xE7, 0x56, 0x54, 0xF9, 0x97, 0x36, 0x02,
                0x20, 0x23, 0xB5, 0x1B, 0x6E, 0x42, 0x53, 0x28, 0x28, 0xDB, 0x09, 0x92, 0x03, 0x37, 0x05, 0x0D, 0x70, 0xF0, 0xB6,
                0xB7, 0x2C, 0xCD, 0x3C, 0xB7, 0x81, 0xE6, 0x71, 0xAC, 0x22, 0x5D, 0xF5, 0x47, 0x2B },
              413,
              { 0xEF, 0x18, 0xE0, 0xEC, 0xD4, 0x66, 0x04, 0x34, 0xDF, 0x0D,
                0xBC, 0x91, 0x1E, 0xD4, 0x52, 0x16, 0x99, 0x66, 0x83, 0x9F } }
        };

    size_t paaLookupTableIdx;
    for (paaLookupTableIdx = 0; paaLookupTableIdx < sizeof(sPAALookupTable) / sizeof(*sPAALookupTable); ++paaLookupTableIdx)
    {
        if (skid.data_equal(ByteSpan(sPAALookupTable[paaLookupTableIdx].mSKID)))
        {
            break;
        }
    }

    VerifyOrReturnError(paaLookupTableIdx < sizeof(sPAALookupTable) / sizeof(*sPAALookupTable), CHIP_ERROR_INVALID_ARGUMENT);

    return CopySpanToMutableSpan(ByteSpan{ sPAALookupTable[paaLookupTableIdx].mPAACertificate }, out_dac_buffer);
}

class ExampleDACVerifier : public DeviceAttestationVerifier
{
public:
    AttestationVerificationResult
    VerifyAttestationInformation(const ByteSpan & attestation_info_buffer, const ByteSpan & attestation_challenge_buffer,
                                 const ByteSpan & attestation_signature_buffer, const ByteSpan & pai_cert_der_buffer,
                                 const ByteSpan & dac_cert_der_buffer, const ByteSpan & attestation_nonce) override;
};

AttestationVerificationResult ExampleDACVerifier::VerifyAttestationInformation(const ByteSpan & attestation_info_buffer,
                                                                               const ByteSpan & attestation_challenge_buffer,
                                                                               const ByteSpan & attestation_signature_buffer,
                                                                               const ByteSpan & pai_cert_der_buffer,
                                                                               const ByteSpan & dac_cert_der_buffer,
                                                                               const ByteSpan & attestation_nonce)
{
    // match DAC and PAI VIDs
    if (!pai_cert_der_buffer.empty())
    {
        VendorId paiVid;
        VendorId dacVid;

        VerifyOrReturnError(ExtractVIDFromX509Cert(pai_cert_der_buffer, paiVid) == CHIP_NO_ERROR,
                            AttestationVerificationResult::kPaiFormatInvalid);
        VerifyOrReturnError(ExtractVIDFromX509Cert(dac_cert_der_buffer, dacVid) == CHIP_NO_ERROR,
                            AttestationVerificationResult::kDacFormatInvalid);

        VerifyOrReturnError(paiVid == dacVid, AttestationVerificationResult::kDacVendorIdMismatch);
    }

    uint8_t akidBuf[Credentials::kKeyIdentifierLength];
    MutableByteSpan akid(akidBuf);
    ExtractAKIDFromX509Cert(pai_cert_der_buffer.empty() ? dac_cert_der_buffer : pai_cert_der_buffer, akid);

    constexpr size_t paaCertAllocatedLen = kMaxDERCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> paaCert;

    VerifyOrReturnError(paaCert.Alloc(paaCertAllocatedLen), AttestationVerificationResult::kNoMemory);

    MutableByteSpan paa(paaCert.Get(), paaCertAllocatedLen);
    VerifyOrReturnError(GetProductAttestationAuthorityCert(akid, paa) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kPaaNotFound);

    VerifyOrReturnError(ValidateCertificateChain(paa.data(), paa.size(), pai_cert_der_buffer.data(), pai_cert_der_buffer.size(),
                                                 dac_cert_der_buffer.data(), dac_cert_der_buffer.size()) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kPaaUntrusted);

    P256PublicKey remoteManufacturerPubkey;
    VerifyOrReturnError(ExtractPubkeyFromX509Cert(dac_cert_der_buffer, remoteManufacturerPubkey) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kDacFormatInvalid);

    //    ReturnErrorOnFailure(DeconstructAttestationElements(attestationElements, certDeclaration, attestationNonce, timestamp,
    //                                                        firmwareInfo));

    // Step f
    // Verify that Nonce matches with what we sent
    // VerifyOrReturnError(attestation_nonce.data_equal(attestationNonce), AttestationVerificationResult::kNonceMismatch);

    // Step d
    P256ECDSASignature deviceSignature;
    // SetLength will fail if signature doesn't fit
    VerifyOrReturnError(deviceSignature.SetLength(attestation_signature_buffer.size()) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kInvalidSignature);
    memcpy(deviceSignature, attestation_signature_buffer.data(), attestation_signature_buffer.size());
    VerifyOrReturnError(ValidateAttestationData(remoteManufacturerPubkey, attestation_info_buffer, attestation_challenge_buffer,
                                                deviceSignature) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kDacSignatureInvalid);

    return AttestationVerificationResult::kSuccess;
}

} // namespace

DeviceAttestationVerifier * GetExampleDACVerifier()
{
    static ExampleDACVerifier example_dac_verifier;

    return &example_dac_verifier;
}

} // namespace Examples
} // namespace Credentials
} // namespace chip
