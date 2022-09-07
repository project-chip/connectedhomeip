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
#include "DefaultDeviceAttestationVerifier.h"

#include <controller/OperationalCredentialsDelegate.h>
#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationVendorReserved.h>
#include <crypto/CHIPCryptoPAL.h>
#include <string.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

namespace chip {
namespace TestCerts {
extern const ByteSpan sTestCert_PAA_FFF1_Cert;
extern const ByteSpan sTestCert_PAA_NoVID_Cert;
} // namespace TestCerts
} // namespace chip

using namespace chip::Crypto;

namespace chip {
namespace Credentials {

namespace {

// As per specifications section 11.22.5.1. Constant RESP_MAX
constexpr size_t kMaxResponseLength = 900;

static const ByteSpan kTestPaaRoots[] = {
    TestCerts::sTestCert_PAA_FFF1_Cert,
    TestCerts::sTestCert_PAA_NoVID_Cert,
};

// Test CD Signing Key from `credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem`
// used to verify any in-SDK development CDs. The associated keypair to do actual signing is in
// `credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem`.
//
// -----BEGIN CERTIFICATE-----
// MIIBszCCAVqgAwIBAgIIRdrzneR6oI8wCgYIKoZIzj0EAwIwKzEpMCcGA1UEAwwg
// TWF0dGVyIFRlc3QgQ0QgU2lnbmluZyBBdXRob3JpdHkwIBcNMjEwNjI4MTQyMzQz
// WhgPOTk5OTEyMzEyMzU5NTlaMCsxKTAnBgNVBAMMIE1hdHRlciBUZXN0IENEIFNp
// Z25pbmcgQXV0aG9yaXR5MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEPDmJIkUr
// VcrzicJb0bykZWlSzLkOiGkkmthHRlMBTL+V1oeWXgNrUhxRA35rjO3vyh60QEZp
// T6CIgu7WUZ3suqNmMGQwEgYDVR0TAQH/BAgwBgEB/wIBATAOBgNVHQ8BAf8EBAMC
// AQYwHQYDVR0OBBYEFGL6gjNZrPqplj4c+hQK3fUE83FgMB8GA1UdIwQYMBaAFGL6
// gjNZrPqplj4c+hQK3fUE83FgMAoGCCqGSM49BAMCA0cAMEQCICxUXOTkV9im8NnZ
// u+vW7OHd/n+MbZps83UyH8b6xxOEAiBUB3jodDlyUn7t669YaGIgtUB48s1OYqdq
// 58u5L/VMiw==
// -----END CERTIFICATE-----
//
constexpr uint8_t gTestCdPubkeyBytes[65] = { 0x04, 0x3c, 0x39, 0x89, 0x22, 0x45, 0x2b, 0x55, 0xca, 0xf3, 0x89, 0xc2, 0x5b,
                                             0xd1, 0xbc, 0xa4, 0x65, 0x69, 0x52, 0xcc, 0xb9, 0x0e, 0x88, 0x69, 0x24, 0x9a,
                                             0xd8, 0x47, 0x46, 0x53, 0x01, 0x4c, 0xbf, 0x95, 0xd6, 0x87, 0x96, 0x5e, 0x03,
                                             0x6b, 0x52, 0x1c, 0x51, 0x03, 0x7e, 0x6b, 0x8c, 0xed, 0xef, 0xca, 0x1e, 0xb4,
                                             0x40, 0x46, 0x69, 0x4f, 0xa0, 0x88, 0x82, 0xee, 0xd6, 0x51, 0x9d, 0xec, 0xba };

constexpr uint8_t gTestCdPubkeyKid[20] = { 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e,
                                           0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3, 0x71, 0x60 };

// Official CD "Signing Key 001"
//
// -----BEGIN CERTIFICATE-----
// MIICCDCCAa2gAwIBAgIHY3NhY2RzMTAKBggqhkjOPQQDAjBSMQwwCgYDVQQKDAND
// U0ExLDAqBgNVBAMMI01hdHRlciBDZXJ0aWZpY2F0aW9uIGFuZCBUZXN0aW5nIENB
// MRQwEgYKKwYBBAGConwCAQwEQzVBMDAgFw0yMjA4MTExOTMxMTVaGA8yMDcyMDcy
// OTE5MzExNVowWDEMMAoGA1UECgwDQ1NBMTIwMAYDVQQDDClDZXJ0aWZpY2F0aW9u
// IERlY2xhcmF0aW9uIFNpZ25pbmcgS2V5IDAwMTEUMBIGCisGAQQBgqJ8AgEMBEM1
// QTAwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARbW8Ou1rqjg/3Pm51ac/rqfmXr
// WSfBxcArHPpLi9trm36yUlE/I/IqWDOdyK24gEYKySHTdte5cMUMO+bm0jbwo2Yw
// ZDASBgNVHRMBAf8ECDAGAQH/AgEAMA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQU
// g/rXgegtAYtPFPChx/aEAYzF0Z8wHwYDVR0jBBgwFoAUl+Rp0MUEFMJvxwH3fpR3
// OQmN9qUwCgYIKoZIzj0EAwIDSQAwRgIhAIbSu8KoWTj5792UxtJ/uSgQXVTLRRsm
// 09ys2m37JxDvAiEA8WMKDbRbwOtkabIyqwDgmiR3KwkyYwaqN4GPsRKfxwQ=
// -----END CERTIFICATE-----
//
constexpr uint8_t gCdSigningKey001PubkeyBytes[65] = {
    0x04, 0x5b, 0x5b, 0xc3, 0xae, 0xd6, 0xba, 0xa3, 0x83, 0xfd, 0xcf, 0x9b, 0x9d, 0x5a, 0x73, 0xfa, 0xea,
    0x7e, 0x65, 0xeb, 0x59, 0x27, 0xc1, 0xc5, 0xc0, 0x2b, 0x1c, 0xfa, 0x4b, 0x8b, 0xdb, 0x6b, 0x9b, 0x7e,
    0xb2, 0x52, 0x51, 0x3f, 0x23, 0xf2, 0x2a, 0x58, 0x33, 0x9d, 0xc8, 0xad, 0xb8, 0x80, 0x46, 0x0a, 0xc9,
    0x21, 0xd3, 0x76, 0xd7, 0xb9, 0x70, 0xc5, 0x0c, 0x3b, 0xe6, 0xe6, 0xd2, 0x36, 0xf0
};

constexpr uint8_t gCdSigningKey001Kid[20] = { 0x83, 0xfa, 0xd7, 0x81, 0xe8, 0x2d, 0x01, 0x8b, 0x4f, 0x14,
                                              0xf0, 0xa1, 0xc7, 0xf6, 0x84, 0x01, 0x8c, 0xc5, 0xd1, 0x9f };

std::array<ByteSpan, 2> gCdKids                 = { ByteSpan{ gTestCdPubkeyKid }, ByteSpan{ gCdSigningKey001Kid } };
std::array<Crypto::P256PublicKey, 2> gCdPubkeys = { Crypto::P256PublicKey{ gTestCdPubkeyBytes },
                                                    Crypto::P256PublicKey{ gCdSigningKey001PubkeyBytes } };

const ArrayAttestationTrustStore kTestAttestationTrustStore{ &kTestPaaRoots[0], ArraySize(kTestPaaRoots) };

AttestationVerificationResult MapError(CertificateChainValidationResult certificateChainValidationResult)
{
    switch (certificateChainValidationResult)
    {
    case CertificateChainValidationResult::kRootFormatInvalid:
        return AttestationVerificationResult::kPaaFormatInvalid;

    case CertificateChainValidationResult::kRootArgumentInvalid:
        return AttestationVerificationResult::kPaaArgumentInvalid;

    case CertificateChainValidationResult::kICAFormatInvalid:
        return AttestationVerificationResult::kPaiFormatInvalid;

    case CertificateChainValidationResult::kICAArgumentInvalid:
        return AttestationVerificationResult::kPaiArgumentInvalid;

    case CertificateChainValidationResult::kLeafFormatInvalid:
        return AttestationVerificationResult::kDacFormatInvalid;

    case CertificateChainValidationResult::kLeafArgumentInvalid:
        return AttestationVerificationResult::kDacArgumentInvalid;

    case CertificateChainValidationResult::kChainInvalid:
        return AttestationVerificationResult::kDacSignatureInvalid;

    case CertificateChainValidationResult::kNoMemory:
        return AttestationVerificationResult::kNoMemory;

    case CertificateChainValidationResult::kInternalFrameworkError:
        return AttestationVerificationResult::kInternalError;

    default:
        return AttestationVerificationResult::kInternalError;
    }
}
} // namespace

void DefaultDACVerifier::VerifyAttestationInformation(const DeviceAttestationVerifier::AttestationInfo & info,
                                                      Callback::Callback<OnAttestationInformationVerification> * onCompletion)
{
    AttestationVerificationResult attestationError = AttestationVerificationResult::kSuccess;

    Platform::ScopedMemoryBuffer<uint8_t> paaCert;
    MutableByteSpan paaDerBuffer;
    AttestationCertVidPid dacVidPid;
    AttestationCertVidPid paiVidPid;
    AttestationCertVidPid paaVidPid;

    VerifyOrExit(!info.attestationElementsBuffer.empty() && !info.attestationChallengeBuffer.empty() &&
                     !info.attestationSignatureBuffer.empty() && !info.paiDerBuffer.empty() && !info.dacDerBuffer.empty() &&
                     !info.attestationNonceBuffer.empty() && onCompletion != nullptr,
                 attestationError = AttestationVerificationResult::kInvalidArgument);

    VerifyOrExit(info.attestationElementsBuffer.size() <= kMaxResponseLength,
                 attestationError = AttestationVerificationResult::kInvalidArgument);

    // Ensure PAI is present
    VerifyOrExit(!info.paiDerBuffer.empty(), attestationError = AttestationVerificationResult::kPaiMissing);

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
        uint8_t akidBuf[Crypto::kAuthorityKeyIdentifierLength];
        MutableByteSpan akid(akidBuf);
        constexpr size_t paaCertAllocatedLen = kMaxDERCertLength;
        CHIP_ERROR err                       = CHIP_NO_ERROR;

        VerifyOrExit(ExtractAKIDFromX509Cert(info.paiDerBuffer, akid) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kPaiFormatInvalid);

        VerifyOrExit(paaCert.Alloc(paaCertAllocatedLen), attestationError = AttestationVerificationResult::kNoMemory);

        paaDerBuffer = MutableByteSpan(paaCert.Get(), paaCertAllocatedLen);
        err          = mAttestationTrustStore->GetProductAttestationAuthorityCert(akid, paaDerBuffer);
        VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_NOT_IMPLEMENTED,
                     attestationError = AttestationVerificationResult::kPaaNotFound);

        if (err == CHIP_ERROR_NOT_IMPLEMENTED)
        {
            VerifyOrExit(kTestAttestationTrustStore.GetProductAttestationAuthorityCert(akid, paaDerBuffer) == CHIP_NO_ERROR,
                         attestationError = AttestationVerificationResult::kPaaNotFound);
        }

        VerifyOrExit(ExtractVIDPIDFromX509Cert(paaDerBuffer, paaVidPid) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kPaaFormatInvalid);

        if (paaVidPid.mVendorId.HasValue())
        {
            VerifyOrExit(paaVidPid.mVendorId == paiVidPid.mVendorId,
                         attestationError = AttestationVerificationResult::kPaiVendorIdMismatch);
        }

        VerifyOrExit(!paaVidPid.mProductId.HasValue(), attestationError = AttestationVerificationResult::kPaaFormatInvalid);
    }

#if !defined(CURRENT_TIME_NOT_IMPLEMENTED)
    VerifyOrExit(IsCertificateValidAtCurrentTime(info.dacDerBuffer) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kDacExpired);
#endif

    CertificateChainValidationResult chainValidationResult;
    VerifyOrExit(ValidateCertificateChain(paaDerBuffer.data(), paaDerBuffer.size(), info.paiDerBuffer.data(),
                                          info.paiDerBuffer.size(), info.dacDerBuffer.data(), info.dacDerBuffer.size(),
                                          chainValidationResult) == CHIP_NO_ERROR,
                 attestationError = MapError(chainValidationResult));

    {
        ByteSpan certificationDeclarationSpan;
        ByteSpan attestationNonceSpan;
        uint32_t timestampDeconstructed;
        ByteSpan firmwareInfoSpan;
        DeviceAttestationVendorReservedDeconstructor vendorReserved;
        ByteSpan certificationDeclarationPayload;

        DeviceInfoForAttestation deviceInfo{
            .vendorId     = info.vendorId,
            .productId    = info.productId,
            .dacVendorId  = dacVidPid.mVendorId.Value(),
            .dacProductId = dacVidPid.mProductId.Value(),
            .paiVendorId  = paiVidPid.mVendorId.Value(),
            .paiProductId = paiVidPid.mProductId.ValueOr(0),
            .paaVendorId  = paaVidPid.mVendorId.ValueOr(VendorId::NotSpecified),
        };

        MutableByteSpan paaSKID(deviceInfo.paaSKID);
        VerifyOrExit(ExtractSKIDFromX509Cert(paaDerBuffer, paaSKID) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kPaaFormatInvalid);
        VerifyOrExit(paaSKID.size() == sizeof(deviceInfo.paaSKID),
                     attestationError = AttestationVerificationResult::kPaaFormatInvalid);

        VerifyOrExit(DeconstructAttestationElements(info.attestationElementsBuffer, certificationDeclarationSpan,
                                                    attestationNonceSpan, timestampDeconstructed, firmwareInfoSpan,
                                                    vendorReserved) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kAttestationElementsMalformed);

        // Verify that Nonce matches with what we sent
        VerifyOrExit(attestationNonceSpan.data_equal(info.attestationNonceBuffer),
                     attestationError = AttestationVerificationResult::kAttestationNonceMismatch);

        attestationError = ValidateCertificationDeclarationSignature(certificationDeclarationSpan, certificationDeclarationPayload);
        VerifyOrExit(attestationError == AttestationVerificationResult::kSuccess, attestationError = attestationError);

        attestationError = ValidateCertificateDeclarationPayload(certificationDeclarationPayload, firmwareInfoSpan, deviceInfo);
        VerifyOrExit(attestationError == AttestationVerificationResult::kSuccess, attestationError = attestationError);
    }

exit:
    onCompletion->mCall(onCompletion->mContext, info, attestationError);
}

AttestationVerificationResult DefaultDACVerifier::ValidateCertificationDeclarationSignature(const ByteSpan & cmsEnvelopeBuffer,
                                                                                            ByteSpan & certDeclBuffer)
{
    ByteSpan kid;
    VerifyOrReturnError(CMS_ExtractKeyId(cmsEnvelopeBuffer, kid) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kCertificationDeclarationNoKeyId);

    Crypto::P256PublicKey verifyingKey;
    CHIP_ERROR err = mCdKeysTrustStore.LookupVerifyingKey(kid, verifyingKey);
    VerifyOrReturnError(err == CHIP_NO_ERROR, AttestationVerificationResult::kCertificationDeclarationNoCertificateFound);

    // Disallow test key if support not enabled
    if (mCdKeysTrustStore.IsCdTestKey(kid) && !IsCdTestKeySupported())
    {
        return AttestationVerificationResult::kCertificationDeclarationNoCertificateFound;
    }

    VerifyOrReturnError(CMS_Verify(cmsEnvelopeBuffer, verifyingKey, certDeclBuffer) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kCertificationDeclarationInvalidSignature);

    return AttestationVerificationResult::kSuccess;
}

AttestationVerificationResult DefaultDACVerifier::ValidateCertificateDeclarationPayload(const ByteSpan & certDeclBuffer,
                                                                                        const ByteSpan & firmwareInfo,
                                                                                        const DeviceInfoForAttestation & deviceInfo)
{
    CertificationElementsWithoutPIDs cdContent;
    CertificationElementsDecoder cdElementsDecoder;
    VerifyOrReturnError(DecodeCertificationElements(certDeclBuffer, cdContent) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kCertificationDeclarationInvalidFormat);

    if (!firmwareInfo.empty())
    {
        // TODO: validate contents based on DCL
    }

    // The vendor_id field in the Certification Declaration SHALL match the VendorID attribute found in the Basic Information
    // cluster
    VerifyOrReturnError(cdContent.vendorId == deviceInfo.vendorId,
                        AttestationVerificationResult::kCertificationDeclarationInvalidVendorId);

    //  The product_id_array field in the Certification Declaration SHALL contain the value of the ProductID attribute found in
    //  the Basic Information cluster.
    VerifyOrReturnError(cdElementsDecoder.IsProductIdIn(certDeclBuffer, deviceInfo.productId),
                        AttestationVerificationResult::kCertificationDeclarationInvalidProductId);

    if (cdContent.dacOriginVIDandPIDPresent)
    {
        // The Vendor ID (VID) subject DN in the DAC SHALL match the dac_origin_vendor_id field in the Certification
        // Declaration.
        VerifyOrReturnError(deviceInfo.dacVendorId == cdContent.dacOriginVendorId,
                            AttestationVerificationResult::kCertificationDeclarationInvalidVendorId);
        // The Vendor ID (VID) subject DN in the PAI SHALL match the dac_origin_vendor_id field in the Certification
        // Declaration.
        VerifyOrReturnError(deviceInfo.paiVendorId == cdContent.dacOriginVendorId,
                            AttestationVerificationResult::kCertificationDeclarationInvalidVendorId);
        // The Product ID (PID) subject DN in the DAC SHALL match the dac_origin_product_id field in the Certification
        // Declaration.
        VerifyOrReturnError(deviceInfo.dacProductId == cdContent.dacOriginProductId,
                            AttestationVerificationResult::kCertificationDeclarationInvalidProductId);
        // The Product ID (PID) subject DN in the PAI, if such a Product ID is present, SHALL match the dac_origin_product_id
        // field in the Certification Declaration.
        if (deviceInfo.paiProductId != 0) // if PAI PID is present
        {
            VerifyOrReturnError(deviceInfo.paiProductId == cdContent.dacOriginProductId,
                                AttestationVerificationResult::kCertificationDeclarationInvalidProductId);
        }
    }
    else
    {
        //  The Vendor ID (VID) subject DN in the DAC SHALL match the vendor_id field in the Certification Declaration
        VerifyOrReturnError(deviceInfo.dacVendorId == cdContent.vendorId,
                            AttestationVerificationResult::kCertificationDeclarationInvalidVendorId);
        // The Vendor ID (VID) subject DN in the PAI SHALL match the vendor_id field in the Certification Declaration.
        VerifyOrReturnError(deviceInfo.paiVendorId == cdContent.vendorId,
                            AttestationVerificationResult::kCertificationDeclarationInvalidVendorId);
        // The Product ID (PID) subject DN in the DAC SHALL be present in the product_id_array field in the Certification
        // Declaration.
        VerifyOrReturnError(cdElementsDecoder.IsProductIdIn(certDeclBuffer, deviceInfo.dacProductId),
                            AttestationVerificationResult::kCertificationDeclarationInvalidProductId);
        // The Product ID (PID) subject DN in the PAI, if such a Product ID is present, SHALL match one of the values present in
        // the product_id_array field in the Certification Declaration.
        if (deviceInfo.paiProductId != 0) // if PAI PID is present
        {
            VerifyOrReturnError(cdElementsDecoder.IsProductIdIn(certDeclBuffer, deviceInfo.paiProductId),
                                AttestationVerificationResult::kCertificationDeclarationInvalidProductId);
        }
    }

    if (cdContent.authorizedPAAListPresent)
    {
        // The Subject Key Id of the PAA SHALL match one of the values present in the authorized_paa_list
        // in the Certification Declaration.
        VerifyOrReturnError(cdElementsDecoder.HasAuthorizedPAA(certDeclBuffer, ByteSpan(deviceInfo.paaSKID)),
                            AttestationVerificationResult::kCertificationDeclarationInvalidPAA);
    }

    return AttestationVerificationResult::kSuccess;
}

CHIP_ERROR DefaultDACVerifier::VerifyNodeOperationalCSRInformation(const ByteSpan & nocsrElementsBuffer,
                                                                   const ByteSpan & attestationChallengeBuffer,
                                                                   const ByteSpan & attestationSignatureBuffer,
                                                                   const P256PublicKey & dacPublicKey, const ByteSpan & csrNonce)
{
    VerifyOrReturnError(!nocsrElementsBuffer.empty() && !attestationChallengeBuffer.empty() &&
                            !attestationSignatureBuffer.empty() && !csrNonce.empty(),
                        CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(nocsrElementsBuffer.size() <= kMaxResponseLength, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(csrNonce.size() == Controller::kCSRNonceLength, CHIP_ERROR_INVALID_ARGUMENT);

    ByteSpan csrSpan;
    ByteSpan csrNonceSpan;
    ByteSpan vendorReserved1Span;
    ByteSpan vendorReserved2Span;
    ByteSpan vendorReserved3Span;
    ReturnErrorOnFailure(DeconstructNOCSRElements(nocsrElementsBuffer, csrSpan, csrNonceSpan, vendorReserved1Span,
                                                  vendorReserved2Span, vendorReserved3Span));

    VerifyOrReturnError(csrNonceSpan.size() == Controller::kCSRNonceLength, CHIP_ERROR_INVALID_ARGUMENT);

    // Verify that Nonce matches with what we sent
    VerifyOrReturnError(csrNonceSpan.data_equal(csrNonce), CHIP_ERROR_INVALID_ARGUMENT);

    // Validate overall attestation signature on attestation information
    P256ECDSASignature signature;
    // SetLength will fail if signature doesn't fit
    ReturnErrorOnFailure(signature.SetLength(attestationSignatureBuffer.size()));
    memcpy(signature.Bytes(), attestationSignatureBuffer.data(), attestationSignatureBuffer.size());

    ReturnErrorOnFailure(ValidateAttestationSignature(dacPublicKey, nocsrElementsBuffer, attestationChallengeBuffer, signature));

    return CHIP_NO_ERROR;
}

bool CsaCdKeysTrustStore::IsCdTestKey(const ByteSpan & kid) const
{
    return kid.data_equal(ByteSpan{ gTestCdPubkeyKid });
}

CHIP_ERROR CsaCdKeysTrustStore::AddTrustedKey(const ByteSpan & kid, const Crypto::P256PublicKey & pubKey)
{
    ReturnErrorCodeIf(kid.size() > SingleKeyEntry::kMaxKidSize, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(kid.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(mNumTrustedKeys == kMaxNumTrustedKeys, CHIP_ERROR_NO_MEMORY);

    auto & entry = mTrustedKeys[mNumTrustedKeys];

    entry.kidSize = kid.size();
    memcpy(&entry.kidBuffer[0], kid.data(), kid.size());
    entry.publicKey = pubKey;

    ++mNumTrustedKeys;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CsaCdKeysTrustStore::AddTrustedKey(const ByteSpan & derCertBytes)
{
    // TODO: Verify cert against CD root of trust (i.e. verify signatures). To do so,
    //       we are missing primitives to validate X.509 paths of total length 2.

    uint8_t kidBuf[Crypto::kSubjectKeyIdentifierLength] = { 0 };
    MutableByteSpan kidSpan{ kidBuf };
    P256PublicKey pubKey;

    VerifyOrReturnError(CHIP_NO_ERROR == Crypto::ExtractSKIDFromX509Cert(derCertBytes, kidSpan), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(CHIP_NO_ERROR == Crypto::ExtractPubkeyFromX509Cert(derCertBytes, pubKey), CHIP_ERROR_INVALID_ARGUMENT);
    return AddTrustedKey(kidSpan, pubKey);
}

CHIP_ERROR CsaCdKeysTrustStore::LookupVerifyingKey(const ByteSpan & kid, Crypto::P256PublicKey & outPubKey) const
{
    // First, search for the well known keys
    for (size_t keyIdx = 0; keyIdx < gCdKids.size(); keyIdx++)
    {
        if (kid.data_equal(gCdKids[keyIdx]))
        {
            outPubKey = gCdPubkeys[keyIdx];
            return CHIP_NO_ERROR;
        }
    }

    // Seconds, search externally added keys
    for (size_t keyIdx = 0; keyIdx < mNumTrustedKeys; keyIdx++)
    {
        auto & entry = mTrustedKeys[keyIdx];
        if (kid.data_equal(entry.GetKid()))
        {
            outPubKey = entry.publicKey;
            return CHIP_NO_ERROR;
        }
    }

    // If we get here, the desired key was not found
    return CHIP_ERROR_KEY_NOT_FOUND;
}

const AttestationTrustStore * GetTestAttestationTrustStore()
{
    return &kTestAttestationTrustStore;
}

DeviceAttestationVerifier * GetDefaultDACVerifier(const AttestationTrustStore * paaRootStore)
{
    static DefaultDACVerifier defaultDACVerifier{ paaRootStore };

    return &defaultDACVerifier;
}

} // namespace Credentials
} // namespace chip
