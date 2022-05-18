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

static const ByteSpan kTestPaaRoots[] = {
    TestCerts::sTestCert_PAA_FFF1_Cert,
    TestCerts::sTestCert_PAA_NoVID_Cert,
};

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

/**
 * @brief Look-up of well-known keys used for CD signing by CSA.
 *
 * Current version uses only test key/cert provided in spec.
 */
CHIP_ERROR GetCertificationDeclarationCertificate(const ByteSpan & skid, MutableByteSpan & outCertificate)
{
    struct CertChainLookupTable
    {
        const uint8_t mCertificate[kMax_x509_Certificate_Length];
        const uint8_t mSKID[Crypto::kSubjectKeyIdentifierLength];
    };

    static CertChainLookupTable
        sCertChainLookupTable[] = {
            { { 0x30, 0x82, 0x01, 0xb3, 0x30, 0x82, 0x01, 0x5a, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x45, 0xda, 0xf3, 0x9d,
                0xe4, 0x7a, 0xa0, 0x8f, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x2b, 0x31,
                0x29, 0x30, 0x27, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x20, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65,
                0x73, 0x74, 0x20, 0x43, 0x44, 0x20, 0x53, 0x69, 0x67, 0x6e, 0x69, 0x6e, 0x67, 0x20, 0x41, 0x75, 0x74, 0x68, 0x6f,
                0x72, 0x69, 0x74, 0x79, 0x30, 0x20, 0x17, 0x0d, 0x32, 0x31, 0x30, 0x36, 0x32, 0x38, 0x31, 0x34, 0x32, 0x33, 0x34,
                0x33, 0x5a, 0x18, 0x0f, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a,
                0x30, 0x2b, 0x31, 0x29, 0x30, 0x27, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x20, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72,
                0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x44, 0x20, 0x53, 0x69, 0x67, 0x6e, 0x69, 0x6e, 0x67, 0x20, 0x41, 0x75,
                0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
                0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x3c, 0x39, 0x89, 0x22,
                0x45, 0x2b, 0x55, 0xca, 0xf3, 0x89, 0xc2, 0x5b, 0xd1, 0xbc, 0xa4, 0x65, 0x69, 0x52, 0xcc, 0xb9, 0x0e, 0x88, 0x69,
                0x24, 0x9a, 0xd8, 0x47, 0x46, 0x53, 0x01, 0x4c, 0xbf, 0x95, 0xd6, 0x87, 0x96, 0x5e, 0x03, 0x6b, 0x52, 0x1c, 0x51,
                0x03, 0x7e, 0x6b, 0x8c, 0xed, 0xef, 0xca, 0x1e, 0xb4, 0x40, 0x46, 0x69, 0x4f, 0xa0, 0x88, 0x82, 0xee, 0xd6, 0x51,
                0x9d, 0xec, 0xba, 0xa3, 0x66, 0x30, 0x64, 0x30, 0x12, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x08,
                0x30, 0x06, 0x01, 0x01, 0xff, 0x02, 0x01, 0x01, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04,
                0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x62, 0xfa, 0x82,
                0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3, 0x71, 0x60, 0x30, 0x1f,
                0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9,
                0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3, 0x71, 0x60, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48,
                0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x47, 0x00, 0x30, 0x44, 0x02, 0x20, 0x2c, 0x54, 0x5c, 0xe4, 0xe4, 0x57, 0xd8,
                0xa6, 0xf0, 0xd9, 0xd9, 0xbb, 0xeb, 0xd6, 0xec, 0xe1, 0xdd, 0xfe, 0x7f, 0x8c, 0x6d, 0x9a, 0x6c, 0xf3, 0x75, 0x32,
                0x1f, 0xc6, 0xfa, 0xc7, 0x13, 0x84, 0x02, 0x20, 0x54, 0x07, 0x78, 0xe8, 0x74, 0x39, 0x72, 0x52, 0x7e, 0xed, 0xeb,
                0xaf, 0x58, 0x68, 0x62, 0x20, 0xb5, 0x40, 0x78, 0xf2, 0xcd, 0x4e, 0x62, 0xa7, 0x6a, 0xe7, 0xcb, 0xb9, 0x2f, 0xf5,
                0x4c, 0x8b },
              { 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e,
                0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3, 0x71, 0x60 } }
        };

    size_t certChainLookupTableIdx;
    for (certChainLookupTableIdx = 0; certChainLookupTableIdx < ArraySize(sCertChainLookupTable); ++certChainLookupTableIdx)
    {
        if (skid.data_equal(ByteSpan(sCertChainLookupTable[certChainLookupTableIdx].mSKID)))
        {
            break;
        }
    }

    VerifyOrReturnError(certChainLookupTableIdx < ArraySize(sCertChainLookupTable), CHIP_ERROR_INVALID_ARGUMENT);

    return CopySpanToMutableSpan(ByteSpan{ sCertChainLookupTable[certChainLookupTableIdx].mCertificate }, outCertificate);
}

class DefaultDACVerifier : public DeviceAttestationVerifier
{
public:
    DefaultDACVerifier(const AttestationTrustStore * paaRootStore) : mAttestationTrustStore(paaRootStore) {}

    void VerifyAttestationInformation(const DeviceAttestationVerifier::AttestationInfo & info,
                                      Callback::Callback<OnAttestationInformationVerification> * onCompletion) override;

    AttestationVerificationResult ValidateCertificationDeclarationSignature(const ByteSpan & cmsEnvelopeBuffer,
                                                                            ByteSpan & certDeclBuffer) override;

    AttestationVerificationResult ValidateCertificateDeclarationPayload(const ByteSpan & certDeclBuffer,
                                                                        const ByteSpan & firmwareInfo,
                                                                        const DeviceInfoForAttestation & deviceInfo) override;

    CHIP_ERROR VerifyNodeOperationalCSRInformation(const ByteSpan & nocsrElementsBuffer,
                                                   const ByteSpan & attestationChallengeBuffer,
                                                   const ByteSpan & attestationSignatureBuffer, const P256PublicKey & dacPublicKey,
                                                   const ByteSpan & csrNonce) override;

protected:
    DefaultDACVerifier() {}

    const AttestationTrustStore * mAttestationTrustStore;
};

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

        VerifyOrExit(ExtractAKIDFromX509Cert(info.paiDerBuffer, akid) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kPaiFormatInvalid);

        VerifyOrExit(paaCert.Alloc(paaCertAllocatedLen), attestationError = AttestationVerificationResult::kNoMemory);

        paaDerBuffer = MutableByteSpan(paaCert.Get(), paaCertAllocatedLen);
        VerifyOrExit(mAttestationTrustStore->GetProductAttestationAuthorityCert(akid, paaDerBuffer) == CHIP_NO_ERROR,
                     attestationError = AttestationVerificationResult::kPaaNotFound);

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

    VerifyOrExit(IsCertificateValidAtIssuance(info.dacDerBuffer, info.paiDerBuffer) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kPaiExpired);

    VerifyOrExit(IsCertificateValidAtIssuance(info.dacDerBuffer, paaDerBuffer) == CHIP_NO_ERROR,
                 attestationError = AttestationVerificationResult::kPaaExpired);

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
    onCompletion->mCall(onCompletion->mContext, attestationError);
}

AttestationVerificationResult DefaultDACVerifier::ValidateCertificationDeclarationSignature(const ByteSpan & cmsEnvelopeBuffer,
                                                                                            ByteSpan & certDeclBuffer)
{
    uint8_t certificate[Credentials::kMaxDERCertLength];
    MutableByteSpan certificateSpan(certificate);
    ByteSpan skid;

    VerifyOrReturnError(CMS_ExtractKeyId(cmsEnvelopeBuffer, skid) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kCertificationDeclarationNoKeyId);

    VerifyOrReturnError(GetCertificationDeclarationCertificate(skid, certificateSpan) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kCertificationDeclarationNoCertificateFound);

    VerifyOrReturnError(CMS_Verify(cmsEnvelopeBuffer, certificateSpan, certDeclBuffer) == CHIP_NO_ERROR,
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
        // TODO: check if version_number field in Certification Declaration matches the one in Firmware Information.
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

    VerifyOrReturnError(csrNonce.size() == Controller::kCSRNonceLength, CHIP_ERROR_INVALID_ARGUMENT);

    ByteSpan csrSpan;
    ByteSpan csrNonceSpan;
    ByteSpan vendorReserved1Span;
    ByteSpan vendorReserved2Span;
    ByteSpan vendorReserved3Span;
    ReturnErrorOnFailure(DeconstructNOCSRElements(nocsrElementsBuffer, csrSpan, csrNonceSpan, vendorReserved1Span,
                                                  vendorReserved2Span, vendorReserved3Span));

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

} // namespace

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
