/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <pw_unit_test/framework.h>

#include <controller/CommissioneeDeviceProxy.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/tests/CHIPAttCert_test_vectors.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::TestCerts;

struct TestDeviceAttestationVerifier : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestDeviceAttestationVerifier, UnimplementedDACVerifierStub)
{
    DeviceAttestationVerifier * verifier = GetDeviceAttestationVerifier();
    ASSERT_NE(verifier, nullptr);

    uint8_t testData[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    ByteSpan testSpan(testData);

    // Create AttestationInfo using the constructor with all required arguments
    DeviceAttestationVerifier::AttestationInfo attestationInfo(testSpan, // attestationElements
                                                               testSpan, // attestationChallenge
                                                               testSpan, // attestationSignature
                                                               testSpan, // paiDerBuffer
                                                               testSpan, // dacDerBuffer
                                                               testSpan, // certificationDeclarationBuffer
                                                               VendorId(0x1234),
                                                               0x5678 // productId
    );

    DeviceInfoForAttestation deviceInfo;
    deviceInfo.vendorId  = VendorId(0x1234);
    deviceInfo.productId = 0x5678;

    Crypto::P256Keypair keypair;
    ASSERT_EQ(keypair.Initialize(Crypto::ECPKeyTarget::ECDSA), CHIP_NO_ERROR);
    Crypto::P256PublicKey pubkey = keypair.Pubkey();

    // Test VerifyAttestationInformation
    bool callbackCalled = false;
    chip::Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> callback(
        [](void * context, const DeviceAttestationVerifier::AttestationInfo &, AttestationVerificationResult) {
            bool * called = static_cast<bool *>(context);
            *called       = true;
        },
        &callbackCalled);
    verifier->VerifyAttestationInformation(attestationInfo, &callback);
    EXPECT_FALSE(callbackCalled);

    // Test ValidateCertificationDeclarationSignature
    ByteSpan certDeclBuffer              = testSpan;
    AttestationVerificationResult result = verifier->ValidateCertificationDeclarationSignature(testSpan, certDeclBuffer);
    EXPECT_EQ(result, AttestationVerificationResult::kNotImplemented);

    // Test ValidateCertificateDeclarationPayload
    result = verifier->ValidateCertificateDeclarationPayload(testSpan, testSpan, deviceInfo);
    EXPECT_EQ(result, AttestationVerificationResult::kNotImplemented);

    // Test VerifyNodeOperationalCSRInformation
    CHIP_ERROR error = verifier->VerifyNodeOperationalCSRInformation(testSpan, testSpan, testSpan, pubkey, testSpan);
    EXPECT_EQ(error, CHIP_ERROR_NOT_IMPLEMENTED);
}

TEST_F(TestDeviceAttestationVerifier, AttestationDeviceInfoCopiesFields)
{
    // Prepare minimal data for PAI, DAC and other required fields
    uint8_t paiData[3]   = { 0x01, 0x02, 0x03 };
    uint8_t dacData[5]   = { 0x10, 0x11, 0x12, 0x13, 0x14 };
    uint8_t nonceData[4] = { 0x21, 0x22, 0x23, 0x24 };

    ByteSpan paiSpan(paiData);
    ByteSpan dacSpan(dacData);
    ByteSpan nonceSpan(nonceData);

    uint8_t cdData[6] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    uint8_t tlvBuf[64];
    chip::TLV::TLVWriter writer;
    writer.Init(tlvBuf, sizeof(tlvBuf));
    chip::TLV::TLVType outerType;

    CHIP_ERROR err = writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // CD
    writer.Put(chip::TLV::ContextTag(1), ByteSpan(cdData));
    // Nonce
    writer.Put(chip::TLV::ContextTag(2), ByteSpan(nonceData));
    // Timestamp
    writer.Put(chip::TLV::ContextTag(3), static_cast<uint32_t>(0));
    // FirmwareInfo
    writer.Put(chip::TLV::ContextTag(4), ByteSpan());
    // VendorReserved
    writer.Put(chip::TLV::ContextTag(5), ByteSpan());

    err = writer.EndContainer(outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    ByteSpan attestationElementsSpan(tlvBuf);

    // Construct AttestationInfo
    DeviceAttestationVerifier::AttestationInfo attestationInfo(attestationElementsSpan, // attestationElements
                                                               nonceSpan,               // attestationChallenge
                                                               nonceSpan,               // attestationSignature
                                                               paiSpan,                 // paiDerBuffer
                                                               dacSpan,                 // dacDerBuffer
                                                               nonceSpan,               // certificationDeclarationBuffer
                                                               VendorId(0x1234),
                                                               0x5678 // productId
    );

    DeviceAttestationVerifier::AttestationDeviceInfo deviceInfo(attestationInfo);

    EXPECT_EQ(deviceInfo.BasicInformationVendorId(), VendorId(0x1234));
    EXPECT_EQ(deviceInfo.BasicInformationProductId(), 0x5678);

    ByteSpan copiedPai = deviceInfo.paiDerBuffer();
    ASSERT_EQ(copiedPai.size(), paiSpan.size());
    EXPECT_EQ(0, memcmp(copiedPai.data(), paiSpan.data(), paiSpan.size()));

    ByteSpan copiedDac = deviceInfo.dacDerBuffer();
    ASSERT_EQ(copiedDac.size(), dacSpan.size());
    EXPECT_EQ(0, memcmp(copiedDac.data(), dacSpan.data(), dacSpan.size()));

    ASSERT_TRUE(deviceInfo.cdBuffer().HasValue());
    ByteSpan copiedCd = deviceInfo.cdBuffer().Value();
    ASSERT_EQ(copiedCd.size(), sizeof(cdData));
    EXPECT_EQ(0, memcmp(copiedCd.data(), cdData, sizeof(cdData)));
}

TEST_F(TestDeviceAttestationVerifier, SetDeviceAttestationVerifierNullptrArgument)
{
    DeviceAttestationVerifier * original = GetDeviceAttestationVerifier();
    SetDeviceAttestationVerifier(nullptr);
    EXPECT_EQ(GetDeviceAttestationVerifier(), original);
}

TEST_F(TestDeviceAttestationVerifier, AttestationResultDescription)
{
    using chip::Credentials::AttestationVerificationResult;
    using chip::Credentials::GetAttestationResultDescription;

    struct EnumDescPair
    {
        AttestationVerificationResult value;
        const char * expected;
    };

    EnumDescPair cases[] = {
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
        { AttestationVerificationResult::kDacArgumentInvalid, "DAC is invalid in some way according to X.509 backend" },
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
        { AttestationVerificationResult::kAttestationNonceMismatch,
          "Attestation nonce does not match the one from Attestation Request" },
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
          "Certification declaration required a fixed allowed PAA which does not match the final PAA found" },
        { AttestationVerificationResult::kNoMemory, "Failed to allocate memory to process attestation verification" },
        { AttestationVerificationResult::kInvalidArgument,
          "Some unexpected invalid argument was provided internally to the device attestation procedure (likely malformed input "
          "data from candidate device)" },
        { AttestationVerificationResult::kInternalError,
          "An internal error arose in the device attestation procedure (likely malformed input data from candidate device)" },
        { AttestationVerificationResult::kNotImplemented,
          "Reached a critical-but-unimplemented part of the device attestation procedure!" }
    };

    for (const auto & c : cases)
    {
        EXPECT_STREQ(GetAttestationResultDescription(c.value), c.expected);
    }

    auto unknownValue =
        static_cast<AttestationVerificationResult>(static_cast<int>(AttestationVerificationResult::kNotImplemented) + 1);
    EXPECT_STREQ(GetAttestationResultDescription(unknownValue), "<AttestationVerificationResult does not have a description!>");
}

TEST_F(TestDeviceAttestationVerifier, SetRevocationDelegate)
{
    static uint8_t dummyCert[1] = { 0x00 };
    ByteSpan certSpan(dummyCert);
    ArrayAttestationTrustStore trustStore(&certSpan, 1);

    DefaultDACVerifier verifier(&trustStore);

    EXPECT_EQ(verifier.SetRevocationDelegate(nullptr), CHIP_NO_ERROR);
}
