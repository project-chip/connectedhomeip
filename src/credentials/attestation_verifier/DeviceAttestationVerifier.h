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
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <stdlib.h>

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
    kPaaArgumentInvalid  = 106,

    kPaiExpired           = 200,
    kPaiSignatureInvalid  = 201,
    kPaiRevoked           = 202,
    kPaiFormatInvalid     = 203,
    kPaiArgumentInvalid   = 204,
    kPaiVendorIdMismatch  = 205,
    kPaiAuthorityNotFound = 206,
    kPaiMissing           = 207,

    kDacExpired           = 300,
    kDacSignatureInvalid  = 301,
    kDacRevoked           = 302,
    kDacFormatInvalid     = 303,
    kDacArgumentInvalid   = 304,
    kDacVendorIdMismatch  = 305,
    kDacProductIdMismatch = 306,
    kDacAuthorityNotFound = 307,

    kFirmwareInformationMismatch = 400,
    kFirmwareInformationMissing  = 401,

    kAttestationSignatureInvalid       = 500,
    kAttestationElementsMalformed      = 501,
    kAttestationNonceMismatch          = 502,
    kAttestationSignatureInvalidFormat = 503,

    kCertificationDeclarationNoKeyId            = 600,
    kCertificationDeclarationNoCertificateFound = 601,
    kCertificationDeclarationInvalidSignature   = 602,
    kCertificationDeclarationInvalidFormat      = 603,
    kCertificationDeclarationInvalidVendorId    = 604,
    kCertificationDeclarationInvalidProductId   = 605,
    kCertificationDeclarationInvalidPAA         = 606,

    kNoMemory = 700,

    kInvalidArgument = 800,

    kInternalError = 900,

    kNotImplemented = 0xFFFFU,

    // TODO: Add more attestation verification errors
};

enum CertificateType : uint8_t
{
    kUnknown = 0,
    kDAC     = 1,
    kPAI     = 2,
};

struct DeviceInfoForAttestation
{
    // Vendor ID reported by device in Basic Information cluster
    uint16_t vendorId = VendorId::NotSpecified;
    // Product ID reported by device in Basic Information cluster
    uint16_t productId = 0;
    // Vendor ID from DAC
    uint16_t dacVendorId = VendorId::NotSpecified;
    // Product ID from DAC
    uint16_t dacProductId = 0;
    // Vendor ID from PAI cert
    uint16_t paiVendorId = VendorId::NotSpecified;
    // Product ID from PAI cert (0 if absent)
    uint16_t paiProductId = 0;
    // Vendor ID from PAA cert
    uint16_t paaVendorId = VendorId::NotSpecified;
    // Subject Key Identifier (SKID) from PAA cert
    uint8_t paaSKID[Crypto::kSubjectKeyIdentifierLength] = { 0 };
};

/**
 * @brief Helper utility to model a basic trust store usable for device attestation verifiers.
 *
 * API is synchronous. Real commissioner implementations may entirely
 * hide Product Attestation Authority cert lookup behind the DeviceAttestationVerifier and
 * never use this interface at all. It is provided as a utility to help build DeviceAttestationVerifier
 * implementations suitable for testing or examples.
 */
class AttestationTrustStore
{
public:
    AttestationTrustStore()          = default;
    virtual ~AttestationTrustStore() = default;

    // Not copyable
    AttestationTrustStore(const AttestationTrustStore &) = delete;
    AttestationTrustStore & operator=(const AttestationTrustStore &) = delete;

    /**
     * @brief Look-up a PAA cert by SKID
     *
     * The implementations of this interface must have access to a set of PAAs to trust.
     *
     * Interface is synchronous, and therefore this should not be used unless to expose a PAA
     * store that is both fully local and quick to access.
     *
     * @param[in] skid Buffer containing the subject key identifier (SKID) of the PAA to look-up
     * @param[in,out] outPaaDerBuffer Buffer to receive the contents of the PAA root cert, if found.
     *                                  Size will be updated to match actual size.
     *
     * @returns CHIP_NO_ERROR on success, CHIP_INVALID_ARGUMENT if `skid` or `outPaaDerBuffer` arguments
     *          are not usable, CHIP_BUFFER_TOO_SMALL if certificate doesn't fit in `outPaaDerBuffer`
     *          span, CHIP_ERROR_CA_CERT_NOT_FOUND if no PAA found that matches `skid.
     *
     */
    virtual CHIP_ERROR GetProductAttestationAuthorityCert(const ByteSpan & skid, MutableByteSpan & outPaaDerBuffer) const = 0;
};

/**
 * @brief Helper utility to model obtaining verifying keys by Key ID
 *
 * API is synchronous. Real commissioner implementations may entirely
 * hide key lookup behind the DeviceAttestationVerifier and never use this interface at all.
 * It is provided as a utility to help build DeviceAttestationVerifier
 * implementations suitable for testing or examples.
 */
class WellKnownKeysTrustStore
{
public:
    WellKnownKeysTrustStore()          = default;
    virtual ~WellKnownKeysTrustStore() = default;

    // Not copyable
    WellKnownKeysTrustStore(const WellKnownKeysTrustStore &) = delete;
    WellKnownKeysTrustStore & operator=(const WellKnownKeysTrustStore &) = delete;

    /**
     * @brief Add a trusted key directly
     *
     * @param[in] kid - Key ID to use. Usually 20 bytes long, max 32 bytes.
     * @param[in] pubKey - Verifying public key to attach to the key ID.
     *
     * @return CHIP_NO_ERROR on success, CHIP_INVALID_ARGUMENT if `kid` or `pubKey` arguments
     *          are not usable. CHIP_ERROR_NO_MEMORY if the trust store is full.
     */
    virtual CHIP_ERROR AddTrustedKey(const ByteSpan & kid, const Crypto::P256PublicKey & pubKey) = 0;

    /**
     * @brief Add a trusted key via a public certificate.
     *
     * The subject public key of the certificate will be used.
     * The subject key ID extensions of the certificate will be the `kid`.
     *
     * Verification of trust chaining is at the discretion of the implementation.
     *
     * @param[in] derCertBytes - Certificate containing the X.509 DER certificate with the key.
     *
     * @return CHIP_NO_ERROR on success, CHIP_INVALID_ARGUMENT if derCertBytes is improperly
     *         formatted or not trusted. CHIP_ERROR_NO_MEMORY if the trust store is full.
     */
    virtual CHIP_ERROR AddTrustedKey(const ByteSpan & derCertBytes) = 0;

    /**
     * @brief Look-up a verifying key by Key ID
     *
     * Interface is synchronous.
     *
     * @param[in] kid Buffer containing the key identifier (KID) of the verifying key to look-up. Usually
     *                a SHA-1-sized buffer (20 bytes).
     * @param[out] outPubKey Reference to where the verifying key found will be stored on CHIP_NO_ERROR
     *
     * @returns CHIP_NO_ERROR on success, CHIP_INVALID_ARGUMENT if `kid` or `pubKey` arguments
     *          are not usable, CHIP_ERROR_KEY_NOT_FOUND if no key is found that matches `kid`.
     */
    virtual CHIP_ERROR LookupVerifyingKey(const ByteSpan & kid, Crypto::P256PublicKey & outPubKey) const = 0;

    /**
     * @brief Returns true if `kid` identifies a known test key.
     *
     * @param kid - Key ID to use. Usually 20 bytes long, max 32 bytes.
     * @return true if it's a test/development-only signing key identifier, false otherwise
     */
    virtual bool IsCdTestKey(const ByteSpan & kid) const = 0;
};

/**
 * @brief Basic AttestationTrustStore that holds all data within caller-owned memory.
 *
 * This is useful to wrap a fixed constant array of certificates into a trust store
 * implementation.
 */

class ArrayAttestationTrustStore : public AttestationTrustStore
{
public:
    ArrayAttestationTrustStore(const ByteSpan * derCerts, size_t numCerts) : mDerCerts(derCerts), mNumCerts(numCerts) {}

    CHIP_ERROR GetProductAttestationAuthorityCert(const ByteSpan & skid, MutableByteSpan & outPaaDerBuffer) const override
    {
        VerifyOrReturnError(!skid.empty() && (skid.data() != nullptr), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(skid.size() == Crypto::kSubjectKeyIdentifierLength, CHIP_ERROR_INVALID_ARGUMENT);

        size_t paaIdx;
        ByteSpan candidate;

        for (paaIdx = 0; paaIdx < mNumCerts; ++paaIdx)
        {
            uint8_t skidBuf[Crypto::kSubjectKeyIdentifierLength] = { 0 };
            candidate                                            = mDerCerts[paaIdx];
            MutableByteSpan candidateSkidSpan{ skidBuf };
            VerifyOrReturnError(CHIP_NO_ERROR == Crypto::ExtractSKIDFromX509Cert(candidate, candidateSkidSpan),
                                CHIP_ERROR_INTERNAL);

            if (skid.data_equal(candidateSkidSpan))
            {
                // Found a match
                return CopySpanToMutableSpan(candidate, outPaaDerBuffer);
            }
        }

        return CHIP_ERROR_CA_CERT_NOT_FOUND;
    }

protected:
    const ByteSpan * mDerCerts;
    const size_t mNumCerts;
};

class DeviceAttestationVerifier
{
public:
    DeviceAttestationVerifier()          = default;
    virtual ~DeviceAttestationVerifier() = default;

    // Not copyable
    DeviceAttestationVerifier(const DeviceAttestationVerifier &) = delete;
    DeviceAttestationVerifier & operator=(const DeviceAttestationVerifier &) = delete;

    struct AttestationInfo
    {
        AttestationInfo(const ByteSpan & attestationElements, const ByteSpan & attestationChallenge,
                        const ByteSpan & attestationSignature, const ByteSpan & paiDer, const ByteSpan & dacDer,
                        const ByteSpan & attestationNonce, VendorId remoteVendorId, uint16_t remoteProductId) :
            attestationElementsBuffer(attestationElements),
            attestationChallengeBuffer(attestationChallenge), attestationSignatureBuffer(attestationSignature),
            paiDerBuffer(paiDer), dacDerBuffer(dacDer), attestationNonceBuffer(attestationNonce), vendorId(remoteVendorId),
            productId(remoteProductId)
        {}
        const ByteSpan
            attestationElementsBuffer; // Buffer containing attestation elements portion of Attestation Response (raw TLV)
        const ByteSpan attestationChallengeBuffer; // Buffer containing the attestation challenge from the secure session
        const ByteSpan attestationSignatureBuffer; // Buffer the signature portion of Attestation Response
        const ByteSpan paiDerBuffer;               // Buffer containing the PAI certificate from device in DER format.
        const ByteSpan dacDerBuffer;               // Buffer containing the DAC certificate from device in DER format.
        const ByteSpan attestationNonceBuffer;     // Buffer containing attestation nonce.
        VendorId vendorId;
        uint16_t productId;
    };

    // Copies the bytes passed to it, and holds the PAI, DAC, and CD for additional verification step
    class AttestationDeviceInfo
    {
    public:
        AttestationDeviceInfo(const AttestationInfo & attestationInfo);
        AttestationDeviceInfo(const ByteSpan & attestationElementsBuffer, const ByteSpan paiDerBuffer, const ByteSpan dacDerBuffer);

        ~AttestationDeviceInfo() = default;

        // Returns buffer containing the PAI certificate from device in DER format.
        const ByteSpan paiDerBuffer() const { return ByteSpan(mPaiDerBuffer.Get(), mPaiDerBuffer.AllocatedSize()); }

        // Returns buffer containing the DAC certificate from device in DER format.
        const ByteSpan dacDerBuffer() const { return ByteSpan(mDacDerBuffer.Get(), mDacDerBuffer.AllocatedSize()); }

        // Returns optional buffer containing the certificate declaration from device.
        const Optional<ByteSpan> cdBuffer() const
        {
            if (mCdBuffer.Get())
            {
                return MakeOptional(ByteSpan(mDacDerBuffer.Get(), mDacDerBuffer.AllocatedSize()));
            }
            else
            {
                return Optional<ByteSpan>();
            }
        }

    private:
        Platform::ScopedMemoryBufferWithSize<uint8_t> mPaiDerBuffer;
        Platform::ScopedMemoryBufferWithSize<uint8_t> mDacDerBuffer;
        Platform::ScopedMemoryBufferWithSize<uint8_t> mCdBuffer;
    };

    typedef void (*OnAttestationInformationVerification)(void * context, const AttestationInfo & info,
                                                         AttestationVerificationResult result);

    /**
     * @brief Verify an attestation information payload against a DAC/PAI chain.
     *
     * @param[in] info All of the information required to verify the attestation.
     * @param[in] onCompletion Callback handler to provide Attestation Information Verification result to the caller of
     *                         VerifyAttestationInformation()
     */
    virtual void VerifyAttestationInformation(const AttestationInfo & info,
                                              Callback::Callback<OnAttestationInformationVerification> * onCompletion) = 0;

    /**
     * @brief Verify a CMS Signed Data signature against the CSA certificate of Subject Key Identifier that matches
     *        the subjectKeyIdentifier field of cmsEnvelopeBuffer.
     *
     * @param[in]  cmsEnvelopeBuffer A ByteSpan with a CMS signed message.
     * @param[out] certDeclBuffer    A ByteSpan to hold the CD content extracted from the CMS signed message.
     *
     * @returns AttestationVerificationResult::kSuccess on success or another specific
     *          value from AttestationVerificationResult enum on failure.
     */
    virtual AttestationVerificationResult ValidateCertificationDeclarationSignature(const ByteSpan & cmsEnvelopeBuffer,
                                                                                    ByteSpan & certDeclBuffer) = 0;

    /**
     * @brief Verify a CMS Signed Data Payload against the Basic Information Cluster and DAC/PAI's Vendor and Product IDs
     *
     * @param[in] certDeclBuffer   A ByteSpan with the Certification Declaration content.
     * @param[in] firmwareInfo     A ByteSpan with the Firmware Information content.
     * @param[in] deviceInfo       The device information
     *
     * @returns AttestationVerificationResult::kSuccess on success or another specific
     *          value from AttestationVerificationResult enum on failure.
     */
    virtual AttestationVerificationResult ValidateCertificateDeclarationPayload(const ByteSpan & certDeclBuffer,
                                                                                const ByteSpan & firmwareInfo,
                                                                                const DeviceInfoForAttestation & deviceInfo) = 0;

    // TODO: Validate Firmware Information

    /**
     * @brief Verify an operational certificate signing request payload against the DAC's public key.
     *
     * @param[in]  nocsrElementsBuffer Buffer containing CSR elements as per specifications section 11.22.5.6. NOCSR Elements.
     * @param[in]  attestationChallengeBuffer Buffer containing the attestation challenge from the secure session
     * @param[in]  attestationSignatureBuffer Buffer containing the signature portion of CSR Response
     * @param[in]  dacPublicKey Public Key from the DAC's certificate received from device.
     * @param[in]  csrNonce Buffer containing CSR nonce.
     */
    virtual CHIP_ERROR VerifyNodeOperationalCSRInformation(const ByteSpan & nocsrElementsBuffer,
                                                           const ByteSpan & attestationChallengeBuffer,
                                                           const ByteSpan & attestationSignatureBuffer,
                                                           const Crypto::P256PublicKey & dacPublicKey,
                                                           const ByteSpan & csrNonce) = 0;

    /**
     * @brief Get the trust store used for the attestation verifier.
     *
     * Returns nullptr if not supported. Be careful not to hold-on to the trust store
     * for too long. It is only expected to have same lifetime as the DeviceAttestationVerifier.
     *
     * @return a pointer to the trust store or nullptr if none is directly accessible.
     */
    virtual WellKnownKeysTrustStore * GetCertificationDeclarationTrustStore() { return nullptr; }

    void EnableCdTestKeySupport(bool enabled) { mEnableCdTestKeySupport = enabled; }
    bool IsCdTestKeySupported() const { return mEnableCdTestKeySupport; }

protected:
    CHIP_ERROR ValidateAttestationSignature(const Crypto::P256PublicKey & pubkey, const ByteSpan & attestationElements,
                                            const ByteSpan & attestationChallenge, const Crypto::P256ECDSASignature & signature);

    // Default to support the "development" test key for legacy purposes (since the DefaultDACVerifier)
    // always supported development keys.
    bool mEnableCdTestKeySupport = true;
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
