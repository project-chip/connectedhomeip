/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>

#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/TestDACRevocationDelegateImpl.h>
#include <credentials/attestation_verifier/TestPAAStore.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <credentials/examples/ExampleDACs.h>
#include <credentials/examples/ExamplePAI.h>

#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>

#include "CHIPAttCert_test_vectors.h"

#include <fstream>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;

namespace {

// Example Credentials impl uses development certs.
static const ByteSpan kExpectedDacPublicKey = DevelopmentCerts::kDacPublicKey;
static const ByteSpan kExpectedPaiPublicKey = DevelopmentCerts::kPaiPublicKey;

} // namespace

struct TestDeviceAttestationCredentials : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestDeviceAttestationCredentials, TestDACProvidersExample_Providers)
{
    uint8_t der_cert_buf[kMaxDERCertLength];
    MutableByteSpan der_cert_span(der_cert_buf);

    // Make sure default provider exists and is not implemented on at least one method
    DeviceAttestationCredentialsProvider * default_provider = GetDeviceAttestationCredentialsProvider();
    ASSERT_NE(default_provider, nullptr);

    CHIP_ERROR err = default_provider->GetDeviceAttestationCert(der_cert_span);
    EXPECT_EQ(err, CHIP_ERROR_NOT_IMPLEMENTED);

    // Replace default provider with example provider
    DeviceAttestationCredentialsProvider * example_dac_provider = Examples::GetExampleDACProvider();
    ASSERT_NE(example_dac_provider, nullptr);
    EXPECT_NE(default_provider, example_dac_provider);

    SetDeviceAttestationCredentialsProvider(example_dac_provider);
    default_provider = GetDeviceAttestationCredentialsProvider();
    EXPECT_EQ(default_provider, example_dac_provider);

    // Make sure DAC is what we expect, by validating public key
    memset(der_cert_span.data(), 0, der_cert_span.size());
    err = example_dac_provider->GetDeviceAttestationCert(der_cert_span);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    P256PublicKey dac_public_key;
    err = ExtractPubkeyFromX509Cert(der_cert_span, dac_public_key);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(dac_public_key.Length(), kExpectedDacPublicKey.size());
    EXPECT_EQ(0, memcmp(dac_public_key.ConstBytes(), kExpectedDacPublicKey.data(), kExpectedDacPublicKey.size()));

    // Make sure PAI is what we expect, by validating public key
    der_cert_span = MutableByteSpan{ der_cert_span };
    memset(der_cert_span.data(), 0, der_cert_span.size());
    err = example_dac_provider->GetProductAttestationIntermediateCert(der_cert_span);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    P256PublicKey pai_public_key;
    err = ExtractPubkeyFromX509Cert(der_cert_span, pai_public_key);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(pai_public_key.Length(), kExpectedPaiPublicKey.size());
    EXPECT_EQ(0, memcmp(pai_public_key.ConstBytes(), kExpectedPaiPublicKey.data(), kExpectedPaiPublicKey.size()));

    // Check for CD presence
    uint8_t other_data_buf[kMaxCMSSignedCDMessage];
    MutableByteSpan other_data_span(other_data_buf);
    memset(other_data_span.data(), 0, other_data_span.size());

    err = example_dac_provider->GetCertificationDeclaration(other_data_span);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_GT(other_data_span.size(), 0u);
    EXPECT_NE(other_data_span.data()[0], 0);

    // Check for firmware information presence
    other_data_span = MutableByteSpan{ other_data_buf };
    memset(other_data_span.data(), 0, other_data_span.size());

    err = example_dac_provider->GetFirmwareInformation(other_data_span);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(other_data_span.size(), 0u);
}

TEST_F(TestDeviceAttestationCredentials, TestDACProvidersExample_Signature)
{
    constexpr uint8_t kExampleMessage[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12,
                                            0x13, 0x14, 0x15, 0x16, 0x17, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
                                            0x26, 0x27, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };

    DeviceAttestationCredentialsProvider * example_dac_provider = Examples::GetExampleDACProvider();
    ASSERT_NE(example_dac_provider, nullptr);

    // Sign using the example attestation private key
    P256ECDSASignature da_signature;
    MutableByteSpan out_sig_span(da_signature.Bytes(), da_signature.Capacity());
    CHIP_ERROR err = example_dac_provider->SignWithDeviceAttestationKey(ByteSpan{ kExampleMessage }, out_sig_span);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(out_sig_span.size(), kP256_ECDSA_Signature_Length_Raw);
    da_signature.SetLength(out_sig_span.size());

    // Get DAC from the provider
    uint8_t dac_cert_buf[kMaxDERCertLength];
    MutableByteSpan dac_cert_span(dac_cert_buf);

    memset(dac_cert_span.data(), 0, dac_cert_span.size());
    err = example_dac_provider->GetDeviceAttestationCert(dac_cert_span);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Extract public key from DAC, prior to signature verification
    P256PublicKey dac_public_key;
    err = ExtractPubkeyFromX509Cert(dac_cert_span, dac_public_key);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(dac_public_key.Length(), kExpectedDacPublicKey.size());
    EXPECT_EQ(memcmp(dac_public_key.ConstBytes(), kExpectedDacPublicKey.data(), kExpectedDacPublicKey.size()), 0);

    // Verify round trip signature
    err = dac_public_key.ECDSA_validate_msg_signature(&kExampleMessage[0], sizeof(kExampleMessage), da_signature);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

static void OnAttestationInformationVerificationCallback(void * context, const DeviceAttestationVerifier::AttestationInfo & info,
                                                         AttestationVerificationResult result)
{
    AttestationVerificationResult * pResult = reinterpret_cast<AttestationVerificationResult *>(context);
    *pResult                                = result;
}

TEST_F(TestDeviceAttestationCredentials, TestDACVerifierExample_AttestationInfoVerification)
{
    uint8_t attestationElementsTestVector[] = {
        0x15, 0x30, 0x01, 0xeb, 0x30, 0x81, 0xe8, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x81,
        0xda, 0x30, 0x81, 0xd7, 0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04,
        0x02, 0x01, 0x30, 0x45, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x38, 0x04, 0x36, 0x15,
        0x24, 0x00, 0x01, 0x25, 0x01, 0xf1, 0xff, 0x36, 0x02, 0x05, 0x00, 0x80, 0x18, 0x25, 0x03, 0x34, 0x12, 0x2c, 0x04, 0x13,
        0x5a, 0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x31, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x31, 0x2d, 0x32, 0x34, 0x24,
        0x05, 0x00, 0x24, 0x06, 0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18, 0x31, 0x7c, 0x30, 0x7a, 0x02, 0x01, 0x03,
        0x80, 0x14, 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3,
        0x71, 0x60, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a,
        0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x04, 0x46, 0x30, 0x44, 0x02, 0x20, 0x43, 0xa6, 0x3f, 0x2b, 0x94, 0x3d, 0xf3,
        0x3c, 0x38, 0xb3, 0xe0, 0x2f, 0xca, 0xa7, 0x5f, 0xe3, 0x53, 0x2a, 0xeb, 0xbf, 0x5e, 0x63, 0xf5, 0xbb, 0xdb, 0xc0, 0xb1,
        0xf0, 0x1d, 0x3c, 0x4f, 0x60, 0x02, 0x20, 0x4c, 0x1a, 0xbf, 0x5f, 0x18, 0x07, 0xb8, 0x18, 0x94, 0xb1, 0x57, 0x6c, 0x47,
        0xe4, 0x72, 0x4e, 0x4d, 0x96, 0x6c, 0x61, 0x2e, 0xd3, 0xfa, 0x25, 0xc1, 0x18, 0xc3, 0xf2, 0xb3, 0xf9, 0x03, 0x69, 0x30,
        0x02, 0x20, 0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d, 0x2c, 0xf3, 0x1d, 0xb2, 0xb4, 0xe1, 0x8b,
        0x41, 0x1b, 0x1d, 0x3a, 0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2, 0x26, 0x03, 0xfd, 0xc6, 0x5b, 0x28,
        0xd0, 0xf1, 0xff, 0x3e, 0x00, 0x01, 0x00, 0x17, 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x5f, 0x76, 0x65, 0x6e, 0x64, 0x6f,
        0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x31, 0xd0, 0xf1, 0xff, 0x3e, 0x00, 0x03, 0x00, 0x18, 0x76,
        0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x33, 0x5f, 0x65, 0x78, 0x61, 0x6d,
        0x70, 0x6c, 0x65, 0x18
    };
    uint8_t attestationChallengeTestVector[] = { 0x7a, 0x49, 0x53, 0x05, 0xd0, 0x77, 0x79, 0xa4,
                                                 0x94, 0xdd, 0x39, 0xa0, 0x85, 0x1b, 0x66, 0x0d };
    uint8_t attestationSignatureTestVector[] = { 0x79, 0x82, 0x53, 0x5d, 0x24, 0xcf, 0xe1, 0x4a, 0x71, 0xab, 0x04, 0x24, 0xcf,
                                                 0x0b, 0xac, 0xf1, 0xe3, 0x45, 0x48, 0x7e, 0xd5, 0x0f, 0x1a, 0xc0, 0xbc, 0x25,
                                                 0x9e, 0xcc, 0xfb, 0x39, 0x08, 0x1e, 0x61, 0xa9, 0x26, 0x7e, 0x74, 0xf8, 0x55,
                                                 0xda, 0x53, 0x63, 0x83, 0x74, 0xa0, 0x16, 0x71, 0xcf, 0x3d, 0x7d, 0xb8, 0xcc,
                                                 0x17, 0x0b, 0x38, 0x03, 0x45, 0xe6, 0x0b, 0xc8, 0x6f, 0xdf, 0x45, 0x9e };
    uint8_t attestationNonceTestVector[]     = { 0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d,
                                                 0x2c, 0xf3, 0x1d, 0xb2, 0xb4, 0xe1, 0x8b, 0x41, 0x1b, 0x1d, 0x3a,
                                                 0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2 };

    // Make sure default verifier exists and is not implemented on at least one method
    DeviceAttestationVerifier * default_verifier = GetDeviceAttestationVerifier();
    ASSERT_NE(default_verifier, nullptr);

    AttestationVerificationResult attestationResult = AttestationVerificationResult::kSuccess;
    ByteSpan emptyByteSpan;
    attestationResult = default_verifier->ValidateCertificationDeclarationSignature(ByteSpan(), emptyByteSpan);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kNotImplemented);

    DeviceAttestationVerifier * example_dac_verifier = GetDefaultDACVerifier(GetTestAttestationTrustStore());
    ASSERT_NE(example_dac_verifier, nullptr);
    EXPECT_NE(default_verifier, example_dac_verifier);

    SetDeviceAttestationVerifier(example_dac_verifier);
    default_verifier = GetDeviceAttestationVerifier();
    EXPECT_EQ(default_verifier, example_dac_verifier);

    attestationResult = AttestationVerificationResult::kNotImplemented;
    Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> attestationInformationVerificationCallback(
        OnAttestationInformationVerificationCallback, &attestationResult);

    Credentials::DeviceAttestationVerifier::AttestationInfo info(
        ByteSpan(attestationElementsTestVector), ByteSpan(attestationChallengeTestVector), ByteSpan(attestationSignatureTestVector),
        TestCerts::sTestCert_PAI_FFF1_8000_Cert, TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert, ByteSpan(attestationNonceTestVector),
        static_cast<VendorId>(0xFFF1), 0x8000);
    default_verifier->VerifyAttestationInformation(info, &attestationInformationVerificationCallback);

    EXPECT_EQ(attestationResult, AttestationVerificationResult::kSuccess);
}

TEST_F(TestDeviceAttestationCredentials, TestDACVerifierExample_CertDeclarationVerification)
{
    // -> format_version = 1
    // -> vendor_id = 0xFFF1
    // -> product_id_array = [ 0x8000 ]
    // -> device_type_id = 0x1234
    // -> certificate_id = "ZIG20141ZB330001-24"
    // -> security_level = 0
    // -> security_information = 0
    // -> version_number = 0x2694
    // -> certification_type = 0
    // -> dac_origin_vendor_id is not present
    // -> dac_origin_product_id is not present
    static constexpr CertificationElements sTestCMS_CertElements = { 1,    0xFFF1, { 0x8000 }, 1, 0x1234, "ZIG20141ZB330001-24",
                                                                     0,    0,      0x2694,     0, 0,      0,
                                                                     false };
    static constexpr uint8_t sTestCMS_CDContent[] = { 0x15, 0x24, 0x00, 0x01, 0x25, 0x01, 0xf1, 0xff, 0x36, 0x02, 0x05,
                                                      0x00, 0x80, 0x18, 0x25, 0x03, 0x34, 0x12, 0x2c, 0x04, 0x13, 0x5a,
                                                      0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x31, 0x5a, 0x42, 0x33, 0x33,
                                                      0x30, 0x30, 0x30, 0x31, 0x2d, 0x32, 0x34, 0x24, 0x05, 0x00, 0x24,
                                                      0x06, 0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18 };

    static constexpr uint8_t sTest_CD[] = {
        0x30, 0x81, 0xe8, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x81, 0xda, 0x30, 0x81, 0xd7,
        0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x45,
        0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x38, 0x04, 0x36, 0x15, 0x24, 0x00, 0x01, 0x25,
        0x01, 0xf1, 0xff, 0x36, 0x02, 0x05, 0x00, 0x80, 0x18, 0x25, 0x03, 0x34, 0x12, 0x2c, 0x04, 0x13, 0x5a, 0x49, 0x47, 0x32,
        0x30, 0x31, 0x34, 0x31, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x31, 0x2d, 0x32, 0x34, 0x24, 0x05, 0x00, 0x24, 0x06,
        0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18, 0x31, 0x7c, 0x30, 0x7a, 0x02, 0x01, 0x03, 0x80, 0x14, 0x62, 0xfa,
        0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04, 0xf3, 0x71, 0x60, 0x30, 0x0b,
        0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
        0x04, 0x03, 0x02, 0x04, 0x46, 0x30, 0x44, 0x02, 0x20, 0x43, 0xa6, 0x3f, 0x2b, 0x94, 0x3d, 0xf3, 0x3c, 0x38, 0xb3, 0xe0,
        0x2f, 0xca, 0xa7, 0x5f, 0xe3, 0x53, 0x2a, 0xeb, 0xbf, 0x5e, 0x63, 0xf5, 0xbb, 0xdb, 0xc0, 0xb1, 0xf0, 0x1d, 0x3c, 0x4f,
        0x60, 0x02, 0x20, 0x4c, 0x1a, 0xbf, 0x5f, 0x18, 0x07, 0xb8, 0x18, 0x94, 0xb1, 0x57, 0x6c, 0x47, 0xe4, 0x72, 0x4e, 0x4d,
        0x96, 0x6c, 0x61, 0x2e, 0xd3, 0xfa, 0x25, 0xc1, 0x18, 0xc3, 0xf2, 0xb3, 0xf9, 0x03, 0x69
    };

    // Replace default verifier with example verifier
    DeviceAttestationVerifier * example_dac_verifier = GetDefaultDACVerifier(GetTestAttestationTrustStore());
    ASSERT_NE(example_dac_verifier, nullptr);

    SetDeviceAttestationVerifier(example_dac_verifier);
    DeviceAttestationVerifier * default_verifier = GetDeviceAttestationVerifier();
    EXPECT_EQ(default_verifier, example_dac_verifier);

    // Check for CD presence
    uint8_t cd_data_buf[kMaxCMSSignedCDMessage] = { 0 };
    MutableByteSpan cd_data_span(cd_data_buf);

    ByteSpan cd_payload;
    AttestationVerificationResult attestation_result =
        default_verifier->ValidateCertificationDeclarationSignature(ByteSpan(sTest_CD), cd_payload);
    EXPECT_EQ(attestation_result, AttestationVerificationResult::kSuccess);

    EXPECT_TRUE(cd_payload.data_equal(ByteSpan(sTestCMS_CDContent)));

    DeviceInfoForAttestation deviceInfo{
        .vendorId     = sTestCMS_CertElements.VendorId,
        .productId    = sTestCMS_CertElements.ProductIds[0],
        .dacVendorId  = sTestCMS_CertElements.VendorId,
        .dacProductId = sTestCMS_CertElements.ProductIds[0],
        .paiVendorId  = sTestCMS_CertElements.VendorId,
        .paiProductId = sTestCMS_CertElements.ProductIds[0],
        .paaVendorId  = sTestCMS_CertElements.VendorId,
    };
    attestation_result = default_verifier->ValidateCertificateDeclarationPayload(cd_payload, ByteSpan(), deviceInfo);
    EXPECT_EQ(attestation_result, AttestationVerificationResult::kSuccess);
}

TEST_F(TestDeviceAttestationCredentials, TestDACVerifierExample_NocsrInformationVerification)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    constexpr uint8_t nocsrElementsTestVector[] = {
        0x15, 0x30, 0x01, 0xdd, 0x30, 0x81, 0xda, 0x30, 0x81, 0x81, 0x02, 0x01, 0x00, 0x30, 0x0e, 0x31, 0x0c, 0x30, 0x0a, 0x06,
        0x03, 0x55, 0x04, 0x0a, 0x0c, 0x03, 0x43, 0x53, 0x41, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d,
        0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x5c, 0xa2, 0x79, 0xe3,
        0x66, 0x82, 0xc2, 0xd4, 0x6c, 0xe7, 0xd4, 0xcf, 0x89, 0x67, 0x84, 0x67, 0x08, 0xb5, 0xb9, 0xf8, 0x5b, 0x9c, 0xda, 0xfd,
        0x8c, 0xa8, 0x85, 0x26, 0x12, 0xcb, 0x0f, 0x0c, 0x7a, 0x71, 0x31, 0x4e, 0xc8, 0xdc, 0x9c, 0x96, 0x34, 0xdd, 0xee, 0xfe,
        0xe9, 0xf6, 0x3f, 0x0e, 0x8b, 0xd7, 0xda, 0xcf, 0xc3, 0xb6, 0xa4, 0x53, 0x2a, 0xad, 0xd8, 0x9a, 0x96, 0x51, 0xcd, 0x6e,
        0xa0, 0x11, 0x30, 0x0f, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x0e, 0x31, 0x02, 0x30, 0x00, 0x30,
        0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x20, 0x0e, 0x67,
        0x5e, 0xe1, 0xb3, 0xbb, 0xfe, 0x15, 0x2a, 0x17, 0x4a, 0xf5, 0x35, 0xe2, 0x2d, 0x55, 0xce, 0x10, 0xc1, 0x50, 0xca, 0xc0,
        0x1b, 0x31, 0x18, 0xde, 0x05, 0xe8, 0xfd, 0x9f, 0x10, 0x48, 0x02, 0x21, 0x00, 0xd8, 0x8c, 0x57, 0xcc, 0x6e, 0x74, 0xf0,
        0xe5, 0x48, 0x8a, 0x26, 0x16, 0x7a, 0x07, 0xfd, 0x6d, 0xbe, 0xf1, 0xaa, 0xad, 0x72, 0x1c, 0x58, 0x0b, 0x6e, 0xae, 0x21,
        0xbe, 0x5e, 0x6d, 0x0c, 0x72, 0x30, 0x02, 0x20, 0x81, 0x4a, 0x4d, 0x4c, 0x1c, 0x4a, 0x8e, 0xbb, 0xea, 0xdb, 0x0a, 0xe2,
        0x82, 0xf9, 0x91, 0xeb, 0x13, 0xac, 0x5f, 0x9f, 0xce, 0x94, 0x30, 0x93, 0x19, 0xaa, 0x94, 0x09, 0x6c, 0x8c, 0xd4, 0xb8,
        0x30, 0x03, 0x17, 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x5f, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73,
        0x65, 0x72, 0x76, 0x65, 0x64, 0x31, 0x30, 0x05, 0x18, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65,
        0x72, 0x76, 0x65, 0x64, 0x33, 0x5f, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x18,
    };
    constexpr uint8_t attestationChallengeTestVector[]      = { 0x7a, 0x49, 0x53, 0x05, 0xd0, 0x77, 0x79, 0xa4,
                                                                0x94, 0xdd, 0x39, 0xa0, 0x85, 0x1b, 0x66, 0x0d };
    constexpr uint8_t csrNonceTestVector[]                  = { 0x81, 0x4a, 0x4d, 0x4c, 0x1c, 0x4a, 0x8e, 0xbb, 0xea, 0xdb, 0x0a,
                                                                0xe2, 0x82, 0xf9, 0x91, 0xeb, 0x13, 0xac, 0x5f, 0x9f, 0xce, 0x94,
                                                                0x30, 0x93, 0x19, 0xaa, 0x94, 0x09, 0x6c, 0x8c, 0xd4, 0xb8 };
    constexpr uint8_t attestationSignatureTestVector[]      = { 0x87, 0x8e, 0x46, 0xcf, 0xfa, 0x83, 0xc8, 0x32, 0x96, 0xeb, 0x27,
                                                                0x2e, 0xbc, 0x37, 0x1c, 0x1f, 0xef, 0xee, 0x6d, 0x69, 0x54, 0xf3,
                                                                0x78, 0x9f, 0xd3, 0xd2, 0x27, 0xe1, 0x64, 0x13, 0xd3, 0xd4, 0x75,
                                                                0xa6, 0x2f, 0xd0, 0x12, 0xb9, 0x19, 0xd9, 0x95, 0x8b, 0xc7, 0x3d,
                                                                0x7c, 0x63, 0xb3, 0xcc, 0x1e, 0xf2, 0xb6, 0x2c, 0x18, 0xe0, 0xcc,
                                                                0x10, 0x2e, 0xd1, 0xba, 0x4d, 0xac, 0x85, 0xfe, 0xea };
    constexpr uint8_t wrongAttestationSignatureTestVector[] = {
        /* added 1 to first index */
        0x88, 0x8e, 0x46, 0xcf, 0xfa, 0x83, 0xc8, 0x32, 0x96, 0xeb, 0x27, 0x2e, 0xbc, 0x37, 0x1c, 0x1f,
        0xef, 0xee, 0x6d, 0x69, 0x54, 0xf3, 0x78, 0x9f, 0xd3, 0xd2, 0x27, 0xe1, 0x64, 0x13, 0xd3, 0xd4,
        0x75, 0xa6, 0x2f, 0xd0, 0x12, 0xb9, 0x19, 0xd9, 0x95, 0x8b, 0xc7, 0x3d, 0x7c, 0x63, 0xb3, 0xcc,
        0x1e, 0xf2, 0xb6, 0x2c, 0x18, 0xe0, 0xcc, 0x10, 0x2e, 0xd1, 0xba, 0x4d, 0xac, 0x85, 0xfe, 0xea
    };
    constexpr uint8_t attestationPublicKey[] = { 0x04, 0xce, 0x5c, 0xf8, 0xef, 0xb0, 0x5d, 0x4e, 0xee, 0x79, 0x0d, 0x0a, 0x71,
                                                 0xd5, 0xc0, 0x11, 0xbb, 0x74, 0x72, 0x40, 0xdb, 0xa2, 0x14, 0x58, 0x84, 0x5d,
                                                 0x33, 0xe3, 0x4b, 0x0a, 0xf6, 0x65, 0x16, 0x33, 0x06, 0x3a, 0x80, 0x4b, 0x2f,
                                                 0xf8, 0x5d, 0xca, 0xb2, 0x01, 0x9a, 0x0a, 0xb6, 0xf5, 0x59, 0x57, 0x75, 0xfe,
                                                 0x8d, 0x85, 0xfb, 0xd7, 0xa0, 0x7c, 0x8e, 0x83, 0x7d, 0xa4, 0xd5, 0xa8, 0xb9 };

    DeviceAttestationVerifier * exampleDacVerifier = GetDefaultDACVerifier(GetTestAttestationTrustStore());
    ASSERT_NE(exampleDacVerifier, nullptr);

    P256PublicKey dacPubkey;
    EXPECT_EQ(sizeof(attestationPublicKey), dacPubkey.Length());
    memcpy(dacPubkey.Bytes(), attestationPublicKey, dacPubkey.Length());

    err = exampleDacVerifier->VerifyNodeOperationalCSRInformation(
        ByteSpan(nocsrElementsTestVector), ByteSpan(attestationChallengeTestVector), ByteSpan(attestationSignatureTestVector),
        dacPubkey, ByteSpan(csrNonceTestVector));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // now test with invalid signature
    err = exampleDacVerifier->VerifyNodeOperationalCSRInformation(
        ByteSpan(nocsrElementsTestVector), ByteSpan(attestationChallengeTestVector), ByteSpan(wrongAttestationSignatureTestVector),
        dacPubkey, ByteSpan(csrNonceTestVector));
    EXPECT_EQ(err, CHIP_ERROR_INVALID_SIGNATURE);
}

TEST_F(TestDeviceAttestationCredentials, TestAttestationTrustStore)
{
    // SKID to trigger CHIP_ERROR_INVALID_ARGUMENT
    ByteSpan kPaaFFF1BadSkidSpan1{ TestCerts::sTestCert_PAA_FFF1_Cert.data(), TestCerts::sTestCert_PAA_FFF1_Cert.size() - 1 };

    // SKID to trigger CHIP_ERROR_INVALID_ARGUMENT
    ByteSpan kPaaFFF1BadSkidSpan2;

    // SKID to trigger CHIP_ERROR_CA_CERT_NOT_FOUND
    uint8_t kPaaGoodSkidNotPresent[] = { 0x6A, 0xFD, 0x22, 0x77, 0x1F, 0x51, 0x71, 0x1F, 0xEC, 0xBF,
                                         0x16, 0x41, 0x97, 0x67, 0x10, 0xDC, 0xDC, 0x31, 0xA1, 0x71 };
    ByteSpan kPaaGoodSkidNotPresentSpan{ kPaaGoodSkidNotPresent };

    struct TestCase
    {
        ByteSpan skidSpan;
        ByteSpan expectedCertSpan;
        CHIP_ERROR expectedResult;
    };

    const TestCase kTestCases[] = {
        { TestCerts::sTestCert_PAA_FFF1_SKID, TestCerts::sTestCert_PAA_FFF1_Cert, CHIP_NO_ERROR },
        { TestCerts::sTestCert_PAA_NoVID_SKID, TestCerts::sTestCert_PAA_NoVID_Cert, CHIP_NO_ERROR },
        { TestCerts::sTestCert_PAA_NoVID_SKID, TestCerts::sTestCert_PAA_NoVID_Cert, CHIP_ERROR_BUFFER_TOO_SMALL },
        { kPaaFFF1BadSkidSpan1, TestCerts::sTestCert_PAA_FFF1_Cert, CHIP_ERROR_INVALID_ARGUMENT },
        { kPaaFFF1BadSkidSpan2, TestCerts::sTestCert_PAA_FFF1_Cert, CHIP_ERROR_INVALID_ARGUMENT },
        { kPaaGoodSkidNotPresentSpan, TestCerts::sTestCert_PAA_FFF1_Cert, CHIP_ERROR_CA_CERT_NOT_FOUND },
    };

    const AttestationTrustStore * testAttestationTrustStore = GetTestAttestationTrustStore();
    ASSERT_NE(testAttestationTrustStore, nullptr);

    for (const auto & testCase : kTestCases)
    {
        uint8_t buf[kMaxDERCertLength];
        MutableByteSpan paaCertSpan{ buf };
        if (testCase.expectedResult == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            // Make the output much too small if checking for size handling
            paaCertSpan = paaCertSpan.SubSpan(0, 16);
        }

        // Try to obtain cert
        CHIP_ERROR result = testAttestationTrustStore->GetProductAttestationAuthorityCert(testCase.skidSpan, paaCertSpan);
        EXPECT_EQ(result, testCase.expectedResult);

        // In success cases, make sure the cert matches expectation.
        if (testCase.expectedResult == CHIP_NO_ERROR)
        {
            EXPECT_TRUE(paaCertSpan.data_equal(testCase.expectedCertSpan));
        }
    }
}

static void WriteTestRevokedData(const char * jsonData, const char * fileName)
{
    // TODO: Add option to load test data from the test without using file. #34588

    // write data to /tmp/sample_revoked_set.json using fstream APIs
    std::ofstream file;
    file.open(fileName, std::ofstream::out | std::ofstream::trunc);
    file << jsonData;
    file.close();
}

TEST_F(TestDeviceAttestationCredentials, TestDACRevocationDelegateImpl)
{
    uint8_t attestationElementsTestVector[]  = { 0 };
    uint8_t attestationChallengeTestVector[] = { 0 };
    uint8_t attestationSignatureTestVector[] = { 0 };
    uint8_t attestationNonceTestVector[]     = { 0 };

    // Details for TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert
    //    Issuer: MEYxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBSTEUMBIGCisGAQQBgqJ8AgEMBEZGRjExFDASBgorBgEEAYKifAICDAQ4MDAw
    //    AKID: AF42B7094DEBD515EC6ECF33B81115225F325288
    //    Serial Number: 0C694F7F866067B2
    //
    // Details for TestCerts::sTestCert_PAI_FFF1_8000_Cert
    //    Issuer: MDAxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBQTEUMBIGCisGAQQBgqJ8AgEMBEZGRjE=
    //    AKID: 6AFD22771F511FECBF1641976710DCDC31A1717E
    //    Serial Number: 3E6CE6509AD840CD1
    Credentials::DeviceAttestationVerifier::AttestationInfo info(
        ByteSpan(attestationElementsTestVector), ByteSpan(attestationChallengeTestVector), ByteSpan(attestationSignatureTestVector),
        TestCerts::sTestCert_PAI_FFF1_8000_Cert, TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert, ByteSpan(attestationNonceTestVector),
        static_cast<VendorId>(0xFFF1), 0x8000);

    AttestationVerificationResult attestationResult = AttestationVerificationResult::kNotImplemented;

    Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> attestationInformationVerificationCallback(
        OnAttestationInformationVerificationCallback, &attestationResult);

    TestDACRevocationDelegateImpl revocationDelegateImpl;

    // Test without revocation set
    revocationDelegateImpl.CheckForRevokedDACChain(info, &attestationInformationVerificationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kSuccess);

    const char * tmpJsonFile = "/tmp/sample_revoked_set.json";
    revocationDelegateImpl.SetDeviceAttestationRevocationSetPath(tmpJsonFile);

    // Test empty json
    WriteTestRevokedData("", tmpJsonFile);
    revocationDelegateImpl.CheckForRevokedDACChain(info, &attestationInformationVerificationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kSuccess);

    // Test DAC is revoked
    const char * jsonData = R"(
    [{
        "type": "revocation_set",
        "issuer_subject_key_id": "AF42B7094DEBD515EC6ECF33B81115225F325288",
        "issuer_name": "MEYxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBSTEUMBIGCisGAQQBgqJ8AgEMBEZGRjExFDASBgorBgEEAYKifAICDAQ4MDAw",
        "revoked_serial_numbers": ["0C694F7F866067B2"]
    }]
    )";
    WriteTestRevokedData(jsonData, tmpJsonFile);
    revocationDelegateImpl.CheckForRevokedDACChain(info, &attestationInformationVerificationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacRevoked);

    // Test PAI is revoked
    jsonData = R"(
    [{
        "type": "revocation_set",
        "issuer_subject_key_id": "6AFD22771F511FECBF1641976710DCDC31A1717E",
        "issuer_name": "MDAxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBQTEUMBIGCisGAQQBgqJ8AgEMBEZGRjE=",
        "revoked_serial_numbers": ["3E6CE6509AD840CD"]
    }]
    )";
    WriteTestRevokedData(jsonData, tmpJsonFile);
    revocationDelegateImpl.CheckForRevokedDACChain(info, &attestationInformationVerificationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kPaiRevoked);

    // Test DAC and PAI both revoked
    jsonData = R"(
    [{
        "type": "revocation_set",
        "issuer_subject_key_id": "AF42B7094DEBD515EC6ECF33B81115225F325288",
        "issuer_name": "MEYxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBSTEUMBIGCisGAQQBgqJ8AgEMBEZGRjExFDASBgorBgEEAYKifAICDAQ4MDAw",
        "revoked_serial_numbers": ["0C694F7F866067B2"]
    },
    {
        "type": "revocation_set",
        "issuer_subject_key_id": "6AFD22771F511FECBF1641976710DCDC31A1717E",
        "issuer_name": "MDAxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBQTEUMBIGCisGAQQBgqJ8AgEMBEZGRjE=",
        "revoked_serial_numbers": ["3E6CE6509AD840CD"]
    }]
    )";
    WriteTestRevokedData(jsonData, tmpJsonFile);
    revocationDelegateImpl.CheckForRevokedDACChain(info, &attestationInformationVerificationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kPaiAndDacRevoked);

    // Test with another test DAC and PAI
    Credentials::DeviceAttestationVerifier::AttestationInfo FFF2_8001_info(
        ByteSpan(attestationElementsTestVector), ByteSpan(attestationChallengeTestVector), ByteSpan(attestationSignatureTestVector),
        TestCerts::sTestCert_PAI_FFF2_8001_Cert, TestCerts::sTestCert_DAC_FFF2_8001_0008_Cert, ByteSpan(attestationNonceTestVector),
        static_cast<VendorId>(0xFFF2), 0x8001);
    revocationDelegateImpl.CheckForRevokedDACChain(FFF2_8001_info, &attestationInformationVerificationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kSuccess);

    // Test issuer does not match
    jsonData = R"(
    [{
        "type": "revocation_set",
        "issuer_subject_key_id": "BF42B7094DEBD515EC6ECF33B81115225F325289",
        "issuer_name": "MEYxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBSTEUMBIGCisGAQQBgqJ8AgEMBEZGRjExFDASBgorBgEEAYKifAICDAQ4MDAw",
        "revoked_serial_numbers": ["0C694F7F866067B2"]
    }]
    )";
    WriteTestRevokedData(jsonData, tmpJsonFile);
    revocationDelegateImpl.CheckForRevokedDACChain(info, &attestationInformationVerificationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kSuccess);

    // Test subject key ID does not match
    jsonData = R"(
    [{
        "type": "revocation_set",
        "issuer_subject_key_id": "BF42B7094DEBD515EC6ECF33B81115225F325289",
        "issuer_name": "MEYxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBSTEUMBIGCisGAQQBgqJ8AgEMBEZGRjExFDASBgorBgEEAYKifAICDAQ4MDAw",
        "revoked_serial_numbers": ["0C694F7F866067B2"]
    }]
    )";
    WriteTestRevokedData(jsonData, tmpJsonFile);
    revocationDelegateImpl.CheckForRevokedDACChain(info, &attestationInformationVerificationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kSuccess);

    // Test serial number does not match
    jsonData = R"(
    [{
        "type": "revocation_set",
        "issuer_subject_key_id": "AF42B7094DEBD515EC6ECF33B81115225F325288",
        "issuer_name": "MEYxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBSTEUMBIGCisGAQQBgqJ8AgEMBEZGRjExFDASBgorBgEEAYKifAICDAQ4MDAw",
        "revoked_serial_numbers": ["3E6CE6509AD840CD1", "BC694F7F866067B1"]
    }]
    )";
    WriteTestRevokedData(jsonData, tmpJsonFile);
    revocationDelegateImpl.CheckForRevokedDACChain(info, &attestationInformationVerificationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kSuccess);

    // Test starting serial number bytes match but not all
    jsonData = R"(
    [{
        "type": "revocation_set",
        "issuer_subject_key_id": "AF42B7094DEBD515EC6ECF33B81115225F325288",
        "issuer_name": "MEYxGDAWBgNVBAMMD01hdHRlciBUZXN0IFBBSTEUMBIGCisGAQQBgqJ8AgEMBEZGRjExFDASBgorBgEEAYKifAICDAQ4MDAw",
        "revoked_serial_numbers": ["0C694F7F866067B21234"]
    }]
    )";
    WriteTestRevokedData(jsonData, tmpJsonFile);
    revocationDelegateImpl.CheckForRevokedDACChain(info, &attestationInformationVerificationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kSuccess);
}
