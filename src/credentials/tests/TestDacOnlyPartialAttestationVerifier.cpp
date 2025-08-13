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

#include <app/tests/suites/credentials/TestHarnessDACProvider.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/attestation_verifier/DacOnlyPartialAttestationVerifier.h>
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
using chip::Credentials::Examples::TestHarnessDACProvider;

namespace {

// Callback function to capture attestation verification results
static void OnAttestationInformationVerificationCallback(void * context, const DeviceAttestationVerifier::AttestationInfo & info,
                                                         AttestationVerificationResult result)
{
    AttestationVerificationResult * pResult = reinterpret_cast<AttestationVerificationResult *>(context);
    *pResult                                = result;
}

} // namespace

struct TestDacOnlyPartialAttestationVerifier : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { attestationResult = AttestationVerificationResult::kSuccess; }

    // Create attestation verification result variable
    // This will be used to capture the result of the verification callback
    PartialDACVerifier verifier;
    AttestationVerificationResult attestationResult = AttestationVerificationResult::kSuccess;
    Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> attestationCallback{
        OnAttestationInformationVerificationCallback, &attestationResult
    };
};

// Test verifier behavior with empty parameters
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithInvalidParameters)
{
    // Create AttestationInfo with empty buffers
    DeviceAttestationVerifier::AttestationInfo invalidInfo(ByteSpan(),                    // empty attestationElements
                                                           ByteSpan(),                    // empty attestationChallenge
                                                           ByteSpan(),                    // empty attestationSignature
                                                           ByteSpan(),                    // empty paiDer
                                                           ByteSpan(),                    // empty dacDer
                                                           ByteSpan(),                    // empty attestationNonce
                                                           static_cast<VendorId>(0xFFF1), // vendorId
                                                           0x8000                         // productId
    );

    // Call the verifier with invalid info
    verifier.VerifyAttestationInformation(invalidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kInvalidArgument);
}

// Test verifier behavior with oversized attestationElements buffer - verifies handling of large data
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithLargeAttestationElementsBuffer)
{
    // Create a buffer with more than 900 elements
    constexpr size_t kLargeBufferSize = 1024; // More than 900 elements
    uint8_t largeBuffer[kLargeBufferSize];

    // Fill the buffer with some test data
    for (size_t i = 0; i < kLargeBufferSize; i++)
    {
        largeBuffer[i] = static_cast<uint8_t>(i % 256); // Fill with repeating pattern 0-255
    }

    // Create ByteSpan from the large buffer
    ByteSpan largeAttestationElements(largeBuffer, kLargeBufferSize);

    // The following test data uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    uint8_t challengeData[32] = { 0x01, 0x02, 0x03 }; // Example challenge
    uint8_t signatureData[64] = { 0x04, 0x05, 0x06 }; // Example signature
    uint8_t paiData[256]      = { 0x07, 0x08, 0x09 }; // Example PAI certificate
    uint8_t dacData[256]      = { 0x0A, 0x0B, 0x0C }; // Example DAC certificate
    uint8_t nonceData[32]     = { 0x0D, 0x0E, 0x0F }; // Example nonce

    DeviceAttestationVerifier::AttestationInfo infoWithLargeBuffer(
        largeAttestationElements, // Large attestationElements buffer
        ByteSpan(challengeData, sizeof(challengeData)), ByteSpan(signatureData, sizeof(signatureData)),
        ByteSpan(paiData, sizeof(paiData)), ByteSpan(dacData, sizeof(dacData)), ByteSpan(nonceData, sizeof(nonceData)),
        static_cast<VendorId>(0xFFF1), // vendorId
        0x8000                         // productId
    );

    // Call the verifier with large buffer
    verifier.VerifyAttestationInformation(infoWithLargeBuffer, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kInvalidArgument);
}

// Test verifier behavior with valid attestation elements
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithValidAttestationElements)
{
    // Use real test vectors from CHIPCert_unit_test_vectors.h for PAI and DAC certificates
    auto paiAsset = GetIAA1CertAsset();
    auto dacAsset = GetNodeA1CertAsset();

    // The following test data uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    uint8_t attestationElements[32] = { 0x01, 0x02, 0x03 };
    uint8_t challengeData[32]       = { 0x04, 0x05, 0x06 };
    uint8_t signatureData[64]       = { 0x07, 0x08, 0x09 };
    uint8_t nonceData[32]           = { 0x0A, 0x0B, 0x0C };

    DeviceAttestationVerifier::AttestationInfo validInfo(
        ByteSpan(attestationElements, sizeof(attestationElements)), ByteSpan(challengeData, sizeof(challengeData)),
        ByteSpan(signatureData, sizeof(signatureData)), paiAsset.mCert, dacAsset.mCert, ByteSpan(nonceData, sizeof(nonceData)),
        static_cast<VendorId>(0xFFF1), 0x8000);

    // Call the verifier with valid information
    verifier.VerifyAttestationInformation(validInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacFormatInvalid);
}

// Test verifier behavior with VID/PID mismatch
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithVIDPIDMismatch)
{
    // Use real test vectors from CHIPCert_unit_test_vectors.h for PAI and DAC certificates
    auto paiAsset = GetIAA1CertAsset();
    auto dacAsset = GetNodeA1CertAsset();

    // The following test data uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    uint8_t attestationElements[32] = { 0x01, 0x02, 0x03 };
    uint8_t challengeData[32]       = { 0x04, 0x05, 0x06 };
    uint8_t signatureData[64]       = { 0x07, 0x08, 0x09 };
    uint8_t nonceData[32]           = { 0x0A, 0x0B, 0x0C };

    DeviceAttestationVerifier::AttestationInfo mismatchInfo(
        ByteSpan(attestationElements, sizeof(attestationElements)), ByteSpan(challengeData, sizeof(challengeData)),
        ByteSpan(signatureData, sizeof(signatureData)), paiAsset.mCert, dacAsset.mCert, ByteSpan(nonceData, sizeof(nonceData)),
        static_cast<VendorId>(0x1234), // Mismatched VID
        0x9999                         // Mismatched PID
    );

    // Call the verifier with mismatched information
    verifier.VerifyAttestationInformation(mismatchInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacFormatInvalid);
}

// Test with valid DAC but invalid PAI
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithValidDACButInvalidPAI)
{
    // The following test data uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data (PAI intentionally invalid).
    uint8_t attestationElements[32] = { 0x01, 0x02, 0x03 };
    uint8_t challengeData[32]       = { 0x04, 0x05, 0x06 };
    uint8_t signatureData[64]       = { 0x07, 0x08, 0x09 };
    uint8_t nonceData[32]           = { 0x0A, 0x0B, 0x0C };
    uint8_t invalidPaiData[64]      = { 0xFF, 0xEE, 0xDD, 0xCC };

    DeviceAttestationVerifier::AttestationInfo dacValidPaiInvalidInfo(
        ByteSpan(attestationElements, sizeof(attestationElements)), ByteSpan(challengeData, sizeof(challengeData)),
        ByteSpan(signatureData, sizeof(signatureData)), ByteSpan(invalidPaiData, sizeof(invalidPaiData)), // Invalid PAI certificate
        TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert,                                                     // Valid DAC certificate
        ByteSpan(nonceData, sizeof(nonceData)), static_cast<VendorId>(0xFFF1), 0x8000);

    // Call the verifier with the invalid information
    verifier.VerifyAttestationInformation(dacValidPaiInvalidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kPaiFormatInvalid);
}

// Test with mismatched Vendor IDs
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithMismatchedVendorIDs)
{
    // The following attestation information uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    uint8_t attestationElements[32] = { 0x01, 0x02, 0x03 };
    uint8_t challengeData[32]       = { 0x04, 0x05, 0x06 };
    uint8_t signatureData[64]       = { 0x07, 0x08, 0x09 };
    uint8_t nonceData[32]           = { 0x0A, 0x0B, 0x0C };

    DeviceAttestationVerifier::AttestationInfo mismatchedVidInfo(
        ByteSpan(attestationElements, sizeof(attestationElements)), ByteSpan(challengeData, sizeof(challengeData)),
        ByteSpan(signatureData, sizeof(signatureData)),
        TestCerts::sTestCert_PAI_FFF2_8001_Cert,      // PAI with VID=FFF2
        TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert, // DAC with VID=FFF1
        ByteSpan(nonceData, sizeof(nonceData)), static_cast<VendorId>(0xFFF1), 0x8000);

    // Call the verifier with the mismatched VID information
    verifier.VerifyAttestationInformation(mismatchedVidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacVendorIdMismatch);
}

// Test with valid PAI and DAC
TEST_F(TestDacOnlyPartialAttestationVerifier, TestPassingVIDPIDChecks)
{
    // The following attestation information uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    uint8_t attestationElements[32] = { 0x01, 0x02, 0x03 };
    uint8_t challengeData[32]       = { 0x04, 0x05, 0x06 };
    uint8_t signatureData[64]       = { 0x07, 0x08, 0x09 };
    uint8_t nonceData[32]           = { 0x0A, 0x0B, 0x0C };

    DeviceAttestationVerifier::AttestationInfo passingVidPidInfo(
        ByteSpan(attestationElements, sizeof(attestationElements)), ByteSpan(challengeData, sizeof(challengeData)),
        ByteSpan(signatureData, sizeof(signatureData)),
        TestCerts::sTestCert_PAI_FFF2_NoPID_Cert,     // PAI with VID=FFF2, NO Product ID
        TestCerts::sTestCert_DAC_FFF2_8001_0008_Cert, // DAC with VID=FFF2, PID=8001
        ByteSpan(nonceData, sizeof(nonceData)), static_cast<VendorId>(0xFFF2), 0x8001);

    // Call the verifier with the valid PAI and DAC information
    verifier.VerifyAttestationInformation(passingVidPidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kAttestationSignatureInvalid);
}

// Test with matching PAI and DAC Product IDs
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithMatchingPAIAndDACProductIDs)
{
    // The following attestation information uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    uint8_t attestationElements[32] = { 0x01, 0x02, 0x03 };
    uint8_t challengeData[32]       = { 0x04, 0x05, 0x06 };
    uint8_t signatureData[64]       = { 0x07, 0x08, 0x09 };
    uint8_t nonceData[32]           = { 0x0A, 0x0B, 0x0C };

    DeviceAttestationVerifier::AttestationInfo matchingPidInfo(
        ByteSpan(attestationElements, sizeof(attestationElements)), ByteSpan(challengeData, sizeof(challengeData)),
        ByteSpan(signatureData, sizeof(signatureData)),
        TestCerts::sTestCert_PAI_FFF2_8001_Cert,      // PAI with VID=FFF2, PID=8001
        TestCerts::sTestCert_DAC_FFF2_8001_0008_Cert, // DAC with VID=FFF2, PID=8001 (same as PAI)
        ByteSpan(nonceData, sizeof(nonceData)), static_cast<VendorId>(0xFFF2), 0x8001);

    // Call the verifier with the matching PAI and DAC information
    verifier.VerifyAttestationInformation(matchingPidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kAttestationSignatureInvalid);
}

// Test with mismatched PAI and DAC Product IDs
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithMismatchedPAIAndDACProductIDs)
{
    // The following attestation information uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    uint8_t attestationElements[32] = { 0x01, 0x02, 0x03 };
    uint8_t challengeData[32]       = { 0x04, 0x05, 0x06 };
    uint8_t signatureData[64]       = { 0x07, 0x08, 0x09 };
    uint8_t nonceData[32]           = { 0x0A, 0x0B, 0x0C };

    DeviceAttestationVerifier::AttestationInfo mismatchedPidInfo(
        ByteSpan(attestationElements, sizeof(attestationElements)), ByteSpan(challengeData, sizeof(challengeData)),
        ByteSpan(signatureData, sizeof(signatureData)),
        TestCerts::sTestCert_PAI_FFF2_8004_FB_Cert,   // PAI with VID=FFF2, PID=8004
        TestCerts::sTestCert_DAC_FFF2_8001_0008_Cert, // DAC with VID=FFF2, PID=8001
        ByteSpan(nonceData, sizeof(nonceData)), static_cast<VendorId>(0xFFF2), 0x8001);

    // Call the verifier with the mismatched PAI and DAC information
    verifier.VerifyAttestationInformation(mismatchedPidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacProductIdMismatch);
}

// Test with invalid attestation signature format
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithInvalidAttestationSignatureFormat)
{
    // The following attestation information uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    uint8_t attestationElements[32] = { 0x01, 0x02, 0x03 };
    uint8_t challengeData[32]       = { 0x04, 0x05, 0x06 };
    uint8_t nonceData[32]           = { 0x0A, 0x0B, 0x0C };

    // Create oversized signature data that exceeds maximum allowed signature size
    constexpr size_t kOversizedSignatureSize = chip::Crypto::kP256_ECDSA_Signature_Length_Raw + 1;
    uint8_t oversizedSignatureData[kOversizedSignatureSize];
    memset(oversizedSignatureData, 0xAA, sizeof(oversizedSignatureData));

    DeviceAttestationVerifier::AttestationInfo oversizedSignatureInfo(
        ByteSpan(attestationElements, sizeof(attestationElements)), ByteSpan(challengeData, sizeof(challengeData)),
        ByteSpan(oversizedSignatureData, sizeof(oversizedSignatureData)), // Oversized signature
        TestCerts::sTestCert_PAI_FFF2_8001_Cert,                          // Valid PAI certificate
        TestCerts::sTestCert_DAC_FFF2_8001_0008_Cert,                     // Valid DAC certificate
        ByteSpan(nonceData, sizeof(nonceData)), static_cast<VendorId>(0xFFF2), 0x8001);

    // Call the verifier with the oversized signature information
    verifier.VerifyAttestationInformation(oversizedSignatureInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kAttestationSignatureInvalidFormat);
}
