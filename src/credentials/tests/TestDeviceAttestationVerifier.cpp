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
#include <credentials/attestation_verifier/TestDACRevocationDelegateImpl.h>
#include <credentials/tests/CHIPAttCert_test_vectors.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::TestCerts;

struct TestDeviceAttestationVerifier : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// Test that AttestationDeviceInfo correctly copies fields from AttestationInfo
TEST_F(TestDeviceAttestationVerifier, AttestationDeviceInfoCopiesFields)
{
    // Use real DER test vectors for PAI/DAC and a valid 32-byte nonce
    const ByteSpan paiSpan = TestCerts::sTestCert_PAI_FFF1_8000_Cert;
    const ByteSpan dacSpan = TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert;
    uint8_t nonceData[32]  = {
        0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d, 0x2c, 0xf3, 0x1d, 0xb2, 0xb4,
        0xe1, 0x8b, 0x41, 0x1b, 0x1d, 0x3a, 0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2,
    };
    ByteSpan nonceSpan(nonceData);

    // Prepare attestation elements TLV with a minimal CertificateDeclaration
    uint8_t cdData[32] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    uint8_t tlvBuf[128];
    chip::TLV::TLVWriter writer;
    writer.Init(tlvBuf, sizeof(tlvBuf));
    chip::TLV::TLVType outerType;

    // Add attestation elements to the TLV structure
    CHIP_ERROR err = writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    EXPECT_SUCCESS(writer.Put(chip::TLV::ContextTag(1), ByteSpan(cdData)));
    EXPECT_SUCCESS(writer.Put(chip::TLV::ContextTag(2), ByteSpan(nonceData)));
    EXPECT_SUCCESS(writer.Put(chip::TLV::ContextTag(3), static_cast<uint32_t>(0))); // Timestamp
    EXPECT_SUCCESS(writer.Put(chip::TLV::ContextTag(4), ByteSpan()));               // FirmwareInfo
    EXPECT_SUCCESS(writer.Put(chip::TLV::ContextTag(5), ByteSpan()));               // VendorReserved
    err = writer.EndContainer(outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    ByteSpan attestationElementsSpan(tlvBuf, writer.GetLengthWritten());

    // Construct AttestationInfo
    DeviceAttestationVerifier::AttestationInfo attestationInfo(attestationElementsSpan,
                                                               nonceSpan, // attestationChallenge
                                                               nonceSpan, // attestationSignature
                                                               paiSpan, dacSpan,
                                                               nonceSpan, // certificationDeclarationBuffer
                                                               VendorId(0x1234),
                                                               0x5678 // productId
    );

    // Create AttestationDeviceInfo
    DeviceAttestationVerifier::AttestationDeviceInfo deviceInfo(attestationInfo);

    // Verify that vendorId and productId were copied correctly
    EXPECT_EQ(deviceInfo.BasicInformationVendorId(), VendorId(0x1234));
    EXPECT_EQ(deviceInfo.BasicInformationProductId(), 0x5678); // productId

    // Verify that the PAI, DAC, and CD buffers were copied correctly
    ByteSpan copiedPai = deviceInfo.paiDerBuffer();
    EXPECT_TRUE(copiedPai.data_equal(paiSpan));

    // Verify that the DAC buffer was copied correctly
    ByteSpan copiedDac = deviceInfo.dacDerBuffer();
    EXPECT_TRUE(copiedDac.data_equal(dacSpan));

    // Verify that the CD buffer was copied correctly
    ASSERT_TRUE(deviceInfo.cdBuffer().HasValue());
    ByteSpan copiedCd = deviceInfo.cdBuffer().Value();
    EXPECT_TRUE(copiedCd.data_equal(ByteSpan(cdData)));
}

// Test that setting the DeviceAttestationVerifier to nullptr does not change the current verifier
TEST_F(TestDeviceAttestationVerifier, SetDeviceAttestationVerifierNullptrArgument)
{
    DeviceAttestationVerifier * original = GetDeviceAttestationVerifier();
    SetDeviceAttestationVerifier(nullptr);
    EXPECT_EQ(GetDeviceAttestationVerifier(), original);
}

// Test that setting a revocation delegate works as expected
TEST_F(TestDeviceAttestationVerifier, SetRevocationDelegate)
{
    // Create a minimal AttestationTrustStore with a dummy certificate
    static uint8_t dummyCert[1]       = { 0x00 };
    static const ByteSpan certSpans[] = { ByteSpan(dummyCert) };
    ArrayAttestationTrustStore trustStore(certSpans, 1);
    DefaultDACVerifier verifier(&trustStore);

    // Check the initial state of the revocation delegate
    EXPECT_EQ(verifier.SetRevocationDelegate(nullptr), CHIP_NO_ERROR);
}

// Test CheckForRevokedDACChain with successful revocation check
TEST_F(TestDeviceAttestationVerifier, CheckForRevokedDACChainSuccess)
{
    // Create a minimal AttestationTrustStore with a dummy certificate
    static uint8_t dummyCert[1]       = { 0x00 };
    static const ByteSpan certSpans[] = { ByteSpan(dummyCert) };
    ArrayAttestationTrustStore trustStore(certSpans, 1);
    DefaultDACVerifier verifier(&trustStore);

    // Create a test revocation delegate that returns success (no revocation data set)
    TestDACRevocationDelegateImpl revocationDelegate;
    EXPECT_SUCCESS(verifier.SetRevocationDelegate(&revocationDelegate));

    // Prepare dummy attestation data
    uint8_t testData[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    ByteSpan testSpan(testData);

    // Create AttestationInfo using the constructor with all required arguments
    DeviceAttestationVerifier::AttestationInfo attestationInfo(testSpan, testSpan, testSpan, testSpan, testSpan, testSpan,
                                                               VendorId(0x1234), 0x5678);

    // Test that the callback is called with success result
    AttestationVerificationResult result = AttestationVerificationResult::kInternalError;
    chip::Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> callback(
        [](void * context, const DeviceAttestationVerifier::AttestationInfo &, AttestationVerificationResult verificationResult) {
            AttestationVerificationResult * resultPtr = static_cast<AttestationVerificationResult *>(context);
            *resultPtr                                = verificationResult;
        },
        &result);

    verifier.CheckForRevokedDACChain(attestationInfo, &callback);
    EXPECT_EQ(result, AttestationVerificationResult::kSuccess);
}

// Test CheckForRevokedDACChain with empty revocation data (should return success)
TEST_F(TestDeviceAttestationVerifier, CheckForRevokedDACChainEmptyRevocationData)
{
    // Create a minimal AttestationTrustStore with a dummy certificate
    static uint8_t dummyCert[1]       = { 0x00 };
    static const ByteSpan certSpans[] = { ByteSpan(dummyCert) };
    ArrayAttestationTrustStore trustStore(certSpans, 1);
    DefaultDACVerifier verifier(&trustStore);

    // Create a test revocation delegate with empty revocation data
    TestDACRevocationDelegateImpl revocationDelegate;
    EXPECT_SUCCESS(revocationDelegate.SetDeviceAttestationRevocationData("[]")); // Empty revocation list
    EXPECT_SUCCESS(verifier.SetRevocationDelegate(&revocationDelegate));

    // Prepare dummy attestation data
    uint8_t testData[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    ByteSpan testSpan(testData);

    // Create AttestationInfo using the constructor with all required arguments
    DeviceAttestationVerifier::AttestationInfo attestationInfo(testSpan, testSpan, testSpan, testSpan, testSpan, testSpan,
                                                               VendorId(0x1234), 0x5678);

    // Test that the callback is called with success result (empty revocation list)
    AttestationVerificationResult result = AttestationVerificationResult::kInternalError;
    chip::Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> callback(
        [](void * context, const DeviceAttestationVerifier::AttestationInfo &, AttestationVerificationResult verificationResult) {
            AttestationVerificationResult * resultPtr = static_cast<AttestationVerificationResult *>(context);
            *resultPtr                                = verificationResult;
        },
        &result);

    verifier.CheckForRevokedDACChain(attestationInfo, &callback);
    EXPECT_EQ(result, AttestationVerificationResult::kSuccess);

    // Clean up
    revocationDelegate.ClearDeviceAttestationRevocationData();
}

// Test CheckForRevokedDACChain without revocation delegate (should skip checks)
TEST_F(TestDeviceAttestationVerifier, CheckForRevokedDACChainNoDelegate)
{
    // Create a minimal AttestationTrustStore with a dummy certificate
    static uint8_t dummyCert[1]       = { 0x00 };
    static const ByteSpan certSpans[] = { ByteSpan(dummyCert) };
    ArrayAttestationTrustStore trustStore(certSpans, 1);
    DefaultDACVerifier verifier(&trustStore);

    // Don't set a revocation delegate (should be nullptr by default)

    // Prepare dummy attestation data
    uint8_t testData[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    ByteSpan testSpan(testData);

    // Create AttestationInfo using the constructor with all required arguments
    DeviceAttestationVerifier::AttestationInfo attestationInfo(testSpan, testSpan, testSpan, testSpan, testSpan, testSpan,
                                                               VendorId(0x1234), 0x5678);

    // Test that the callback is called with success result (skipped)
    AttestationVerificationResult result = AttestationVerificationResult::kInternalError;
    chip::Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> callback(
        [](void * context, const DeviceAttestationVerifier::AttestationInfo &, AttestationVerificationResult verificationResult) {
            AttestationVerificationResult * resultPtr = static_cast<AttestationVerificationResult *>(context);
            *resultPtr                                = verificationResult;
        },
        &result);

    verifier.CheckForRevokedDACChain(attestationInfo, &callback);
    EXPECT_EQ(result, AttestationVerificationResult::kSuccess);
}
