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
#pragma once

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace Credentials {

enum class AttestationVerificationResult : uint16_t
{
    kSuccess = 0,

    kPaaUntrusted        = 100,
    kPaaNotFound         = 101,
    kPaaExpired          = 102,
    kPaaSignatureInvalid = 103,
    kPaaRevoked          = 104,
    kPaaFormatInvalid    = 105,

    kPaiExpired           = 200,
    kPaiSignatureInvalid  = 201,
    kPaiRevoked           = 202,
    kPaiFormatInvalid     = 203,
    kPaiVendorIdMismatch  = 204,
    kPaiAuthorityNotFound = 205,

    kDacExpired           = 300,
    kDacSignatureInvalid  = 301,
    kDacRevoked           = 302,
    kDacFormatInvalid     = 303,
    kDacVendorIdMismatch  = 304,
    kDacProductIdMismatch = 305,
    kDacAuthorityNotFound = 306,

    kFirmwareInformationMismatch = 400,
    kFirmwareInformationMissing  = 401,

    kCertificationDeclarationMissing = 500,

    kNonceMismatch = 600,

    kInvalidSignatureFormat = 700,

    kAttestationSignatureInvalid = 800,

    kNoMemory = 900,

    kNotImplemented = 0xFFFFU,

    // TODO: Add more attestation verification errors
};

class DeviceAttestationVerifier
{
public:
    DeviceAttestationVerifier()          = default;
    virtual ~DeviceAttestationVerifier() = default;

    // Not copyable
    DeviceAttestationVerifier(const DeviceAttestationVerifier &) = delete;
    DeviceAttestationVerifier & operator=(const DeviceAttestationVerifier &) = delete;

    /**
     * @brief Verify an attestation information payload against a DAC/PAI chain.
     *
     * @param[in] attestationInfoBuffer Buffer containing attestation information portion of Attestation Response (raw TLV)
     * @param[in] attestationChallengeBuffer Buffer containing the attestation challenge from the secure session
     * @param[in] attestationSignatureBuffer Buffer the signature portion of Attestation Response
     * @param[in] paiCertDerBuffer Buffer containing the PAI certificate from device in DER format.
     *                                If length zero, there was no PAI certificate.
     * @param[in] dacCertDerBuffer Buffer containing the DAC certificate from device in DER format.
     * @param[in] attestationNonce Buffer containing attestation nonce.
     *
     * @returns AttestationVerificationResult::kSuccess on success or another specific
     *          value from AttestationVerificationResult enum on failure.
     */
    virtual AttestationVerificationResult
    VerifyAttestationInformation(const ByteSpan & attestationInfoBuffer, const ByteSpan & attestationChallengeBuffer,
                                 const ByteSpan & attestationSignatureBuffer, const ByteSpan & paiCertDerBuffer,
                                 const ByteSpan & dacCertDerBuffer, const ByteSpan & attestationNonce) = 0;

    // TODO: Validate Certification Declaration
    // TODO: Validate Firmware Information

protected:
    CHIP_ERROR ValidateAttestationSignature(const chip::Crypto::P256PublicKey & pubkey, const ByteSpan & attestationElements,
                                            const ByteSpan & attestationChallenge,
                                            const chip::Crypto::P256ECDSASignature & signature);
};

/**
 * Instance getter for the global DeviceAttestationVerifier.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global device attestation verifier. Assume never null.
 */
DeviceAttestationVerifier * GetDeviceAttestationVerifier();

/**
 * Instance setter for the global DeviceAttestationVerifier.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `verifier` is nullptr, no change is done.
 *
 * @param[in] verifier the DeviceAttestationVerifier to start returning with the getter
 */
void SetDeviceAttestationVerifier(DeviceAttestationVerifier * verifier);

} // namespace Credentials
} // namespace chip
