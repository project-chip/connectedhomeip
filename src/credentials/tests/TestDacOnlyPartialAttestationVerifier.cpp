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

#include <credentials/attestation_verifier/DacOnlyPartialAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/tests/CHIPAttCert_test_vectors.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::TestCerts;

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
};

// Test verifier behavior with empty parameters
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithInvalidParameters)
{
    // Create attestation verification result variable
    // This will be used to capture the result of the verification callback
    AttestationVerificationResult attestationResult = AttestationVerificationResult::kSuccess;
    Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> attestationCallback(
        OnAttestationInformationVerificationCallback, &attestationResult);

    PartialDACVerifier verifier;

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

    // Expect it to fail with invalid argument error
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

    // Create attestation verification result variable
    // This will be used to capture the result of the verification callback
    AttestationVerificationResult attestationResult = AttestationVerificationResult::kSuccess;
    Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> attestationCallback(
        OnAttestationInformationVerificationCallback, &attestationResult);

    PartialDACVerifier verifier;

    // Create some minimal valid test data for other required fields
    uint8_t challengeData[32] = { 0x01, 0x02, 0x03 }; // Example challenge
    uint8_t signatureData[64] = { 0x04, 0x05, 0x06 }; // Example signature
    uint8_t paiData[256]      = { 0x07, 0x08, 0x09 }; // Example PAI certificate
    uint8_t dacData[256]      = { 0x0A, 0x0B, 0x0C }; // Example DAC certificate
    uint8_t nonceData[32]     = { 0x0D, 0x0E, 0x0F }; // Example nonce

    // Create AttestationInfo with large buffer
    DeviceAttestationVerifier::AttestationInfo infoWithLargeBuffer(
        largeAttestationElements, // Large attestationElements buffer
        ByteSpan(challengeData, sizeof(challengeData)), ByteSpan(signatureData, sizeof(signatureData)),
        ByteSpan(paiData, sizeof(paiData)), ByteSpan(dacData, sizeof(dacData)), ByteSpan(nonceData, sizeof(nonceData)),
        static_cast<VendorId>(0xFFF1), // vendorId
        0x8000                         // productId
    );

    // Call the verifier with large buffer
    verifier.VerifyAttestationInformation(infoWithLargeBuffer, &attestationCallback);

    // The result will depend on the verifier's implementation, but we're testing that it can handle large buffers without crashing
    EXPECT_TRUE(attestationResult == AttestationVerificationResult::kInvalidArgument);
}
