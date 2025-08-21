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

/**
 * Helper function to load a P256 keypair from raw private and public key byte spans.
 * Combines the public and private key data into a serialized keypair format,
 * then deserializes it into the provided keypair object.
 *
 * @param private_key ByteSpan containing the raw private key bytes.
 * @param public_key ByteSpan containing the raw public key bytes.
 * @param keypair Reference to a P256Keypair object to populate.
 * @return CHIP_ERROR indicating success or failure of the operation.
 */
CHIP_ERROR LoadKeypairFromRaw(ByteSpan private_key, ByteSpan public_key, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serialized_keypair;
    ReturnErrorOnFailure(serialized_keypair.SetLength(private_key.size() + public_key.size()));
    memcpy(serialized_keypair.Bytes(), public_key.data(), public_key.size());
    memcpy(serialized_keypair.Bytes() + public_key.size(), private_key.data(), private_key.size());
    return keypair.Deserialize(serialized_keypair);
}

struct TestDacOnlyPartialAttestationVerifier : public ::testing::Test
{
    static constexpr VendorId kTestVendorId  = static_cast<VendorId>(0xFFF1);
    static constexpr uint16_t kTestProductId = 0x8000;

    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    // The following test data uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    static constexpr uint8_t kTestAttestationElements[32] = { 0x01, 0x02, 0x03 };
    static constexpr uint8_t kTestChallengeData[32]       = { 0x04, 0x05, 0x06 };
    static constexpr uint8_t kTestSignatureData[64]       = { 0x07, 0x08, 0x09 };
    static constexpr uint8_t kTestNonceData[32]           = { 0x0A, 0x0B, 0x0C };

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
    DeviceAttestationVerifier::AttestationInfo invalidInfo(ByteSpan(), // attestationElements
                                                           ByteSpan(), // attestationChallenge
                                                           ByteSpan(), // attestationSignature
                                                           ByteSpan(), // paiDer
                                                           ByteSpan(), // dacDer
                                                           ByteSpan(), // attestationNonce
                                                           kTestVendorId, kTestProductId);

    // Call the verifier with invalid info
    verifier.VerifyAttestationInformation(invalidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kInvalidArgument);
}

// Test verifier behavior with oversized attestationElements buffer - verifies handling of large data
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithLargeAttestationElementsBuffer)
{
    // Create a buffer larger than kMaxResponseLength (900 bytes)
    constexpr size_t kLargeBufferSize = 1024;
    uint8_t largeBuffer[kLargeBufferSize];

    // Fill the buffer with some test data
    for (size_t i = 0; i < kLargeBufferSize; i++)
    {
        largeBuffer[i] = static_cast<uint8_t>(i % 256); // Fill with repeating pattern 0-255
    }

    // Create ByteSpan from the large buffer
    ByteSpan largeAttestationElements(largeBuffer);

    // The following test data uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    uint8_t paiData[256] = { 0x01, 0x02, 0x03 };
    uint8_t dacData[256] = { 0x0D, 0x0E, 0x0F };

    DeviceAttestationVerifier::AttestationInfo infoWithLargeBuffer(
        largeAttestationElements, ByteSpan(kTestChallengeData), ByteSpan(kTestSignatureData), ByteSpan(paiData), ByteSpan(dacData),
        ByteSpan(kTestNonceData), kTestVendorId, kTestProductId);

    // Call the verifier with large buffer
    verifier.VerifyAttestationInformation(infoWithLargeBuffer, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kInvalidArgument);
}

// Test with invalid DAC certificate format
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithInvalidDACFormat)
{
    // The actual contents do not represent real or meaningful data.
    uint8_t invalidDacData[64] = { 0xFF, 0xEE, 0xDD, 0xCC };

    DeviceAttestationVerifier::AttestationInfo info(
        ByteSpan(kTestAttestationElements), ByteSpan(kTestChallengeData), ByteSpan(kTestSignatureData),
        TestCerts::sTestCert_PAI_FFF1_8000_Cert, ByteSpan(invalidDacData), ByteSpan(kTestNonceData), kTestVendorId, kTestProductId);

    // Call the verifier with the invalid DAC information
    verifier.VerifyAttestationInformation(info, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacFormatInvalid);
}

// Test with DAC certificate where VID/PID can be extracted but public key extraction fails
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithDACMissingPublicKey)
{
    // Use a DAC certificate with valid VID/PID but a missing or corrupted public key.
    // For this test, we take the first 16 bytes of a valid DAC certificate to simulate a broken public key field.
    uint8_t brokenDacData[16];
    memcpy(brokenDacData, TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert.data(), sizeof(brokenDacData));

    DeviceAttestationVerifier::AttestationInfo info(
        ByteSpan(kTestAttestationElements), ByteSpan(kTestChallengeData), ByteSpan(kTestSignatureData),
        TestCerts::sTestCert_PAI_FFF1_8000_Cert, ByteSpan(brokenDacData), ByteSpan(kTestNonceData), kTestVendorId, kTestProductId);

    verifier.VerifyAttestationInformation(info, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacFormatInvalid);
}

// Test with valid DAC but invalid PAI
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithValidDACButInvalidPAI)
{
    // The actual contents do not represent real or meaningful data.
    uint8_t invalidPaiData[64] = { 0xFF, 0xEE, 0xDD, 0xCC };

    DeviceAttestationVerifier::AttestationInfo dacValidPaiInvalidInfo(
        ByteSpan(kTestAttestationElements), ByteSpan(kTestChallengeData), ByteSpan(kTestSignatureData),
        ByteSpan(invalidPaiData),                     // Invalid PAI certificate
        TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert, // Valid DAC certificate
        ByteSpan(kTestNonceData), kTestVendorId, kTestProductId);

    // Call the verifier with the invalid information
    verifier.VerifyAttestationInformation(dacValidPaiInvalidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kPaiFormatInvalid);
}

// Test with mismatched Vendor IDs
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithMismatchedVendorIDs)
{
    DeviceAttestationVerifier::AttestationInfo mismatchedVidInfo(ByteSpan(kTestAttestationElements), ByteSpan(kTestChallengeData),
                                                                 ByteSpan(kTestSignatureData),
                                                                 TestCerts::sTestCert_PAI_FFF2_8001_Cert,      // PAI with VID=FFF2
                                                                 TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert, // DAC with VID=FFF1
                                                                 ByteSpan(kTestNonceData), kTestVendorId, kTestProductId);

    // Call the verifier with the mismatched VID information
    verifier.VerifyAttestationInformation(mismatchedVidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacVendorIdMismatch);
}

// Test with mismatched PAI and DAC Product IDs
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithMismatchedPAIAndDACProductIDs)
{
    DeviceAttestationVerifier::AttestationInfo mismatchedPidInfo(
        ByteSpan(kTestAttestationElements), ByteSpan(kTestChallengeData), ByteSpan(kTestSignatureData),
        TestCerts::sTestCert_PAI_FFF2_8004_FB_Cert,   // PAI with VID=FFF2, PID=8004
        TestCerts::sTestCert_DAC_FFF2_8001_0008_Cert, // DAC with VID=FFF2, PID=8001
        ByteSpan(kTestNonceData), static_cast<VendorId>(0xFFF2), 0x8001);

    // Call the verifier with the mismatched PAI and DAC information
    verifier.VerifyAttestationInformation(mismatchedPidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacProductIdMismatch);
}

// Test with invalid attestation signature format
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithInvalidAttestationSignatureFormat)
{
    // Create oversized signature data that exceeds maximum allowed signature size
    constexpr size_t kOversizedSignatureSize = chip::Crypto::kP256_ECDSA_Signature_Length_Raw + 1;
    uint8_t oversizedSignatureData[kOversizedSignatureSize];
    memset(oversizedSignatureData, 0xAA, sizeof(oversizedSignatureData));

    DeviceAttestationVerifier::AttestationInfo oversizedSignatureInfo(
        ByteSpan(kTestAttestationElements), ByteSpan(kTestChallengeData),
        ByteSpan(oversizedSignatureData),             // Oversized signature
        TestCerts::sTestCert_PAI_FFF2_8001_Cert,      // Valid PAI certificate
        TestCerts::sTestCert_DAC_FFF2_8001_0008_Cert, // Valid DAC certificate
        ByteSpan(kTestNonceData), kTestVendorId, kTestProductId);

    // Call the verifier with the oversized signature information
    verifier.VerifyAttestationInformation(oversizedSignatureInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kAttestationSignatureInvalidFormat);
}

// Test with matching PAI and DAC Product IDs
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithMatchingPAIAndDACProductIDs)
{
    DeviceAttestationVerifier::AttestationInfo matchingPidInfo(
        ByteSpan(kTestAttestationElements), ByteSpan(kTestChallengeData), ByteSpan(kTestSignatureData),
        TestCerts::sTestCert_PAI_FFF2_8001_Cert,      // PAI with VID=FFF2, PID=8001
        TestCerts::sTestCert_DAC_FFF2_8001_0008_Cert, // DAC with VID=FFF2, PID=8001 (same as PAI)
        ByteSpan(kTestNonceData), kTestVendorId, kTestProductId);

    // Call the verifier with the matching PAI and DAC information
    verifier.VerifyAttestationInformation(matchingPidInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kAttestationSignatureInvalid);
}

// Test case validates that the verifier correctly rejects DAC certificates that have expired
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithExpiredDACCertificate)
{
    // The actual contents do not represent real or meaningful data.
    uint8_t cdData[32] = { 0x11, 0x22, 0x33, 0x44 };
    uint8_t tlvBuf[128];

    // Construct TLV structure representing attestation elements that would be signed by device
    chip::TLV::TLVWriter writer;
    writer.Init(tlvBuf, sizeof(tlvBuf));
    chip::TLV::TLVType outerType;

    // Add attestation elements to the TLV structure
    CHIP_ERROR err = writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    writer.Put(chip::TLV::ContextTag(1), ByteSpan(cdData));
    writer.Put(chip::TLV::ContextTag(2), ByteSpan(kTestNonceData));
    writer.Put(chip::TLV::ContextTag(3), static_cast<uint32_t>(0));
    writer.Put(chip::TLV::ContextTag(4), ByteSpan());
    writer.Put(chip::TLV::ContextTag(5), ByteSpan());
    err = writer.EndContainer(outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    size_t tlvLen = writer.GetLengthWritten();

    // Create the message that the device would have signed: attestation elements + challenge
    uint8_t toSign[sizeof(tlvBuf) + sizeof(kTestChallengeData)];
    memcpy(toSign, tlvBuf, tlvLen);
    memcpy(toSign + tlvLen, kTestChallengeData, sizeof(kTestChallengeData));

    // Use a test certificate that is specifically expired
    chip::Crypto::P256Keypair keypair;
    err = LoadKeypairFromRaw(ByteSpan(TestCerts::sTestCert_DAC_FFF2_8004_0020_ValInPast_PrivateKey),
                             ByteSpan(TestCerts::sTestCert_DAC_FFF2_8004_0020_ValInPast_PublicKey), keypair);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Generate signature using the expired certificate's private key
    chip::Crypto::P256ECDSASignature signature;
    err = keypair.ECDSA_sign_msg(toSign, tlvLen + sizeof(kTestChallengeData), signature);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Create attestation info using the expired DAC certificate
    DeviceAttestationVerifier::AttestationInfo expiredDACInfo(
        ByteSpan(tlvBuf, tlvLen), ByteSpan(kTestChallengeData), ByteSpan(signature.ConstBytes(), signature.Length()),
        TestCerts::sTestCert_PAI_FFF2_8004_FB_Cert, TestCerts::sTestCert_DAC_FFF2_8004_0020_ValInPast_Cert,
        ByteSpan(kTestNonceData), static_cast<VendorId>(0xFFF2), 8004);

    verifier.VerifyAttestationInformation(expiredDACInfo, &attestationCallback);

    // Expect the verifier to detect the expired certificate and reject attestation
#if !defined(CURRENT_TIME_NOT_IMPLEMENTED)
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacExpired);
#endif
}

// Test case verifies that certificates with future validity periods are properly rejected
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithValidInFutureDACCertificate)
{
    // The actual contents do not represent real or meaningful data.
    uint8_t cdData[32] = { 0x11, 0x22, 0x33, 0x44 };
    uint8_t tlvBuf[128];

    // Construct TLV structure representing attestation elements that would be signed by device
    chip::TLV::TLVWriter writer;
    writer.Init(tlvBuf, sizeof(tlvBuf));
    chip::TLV::TLVType outerType;

    // Add attestation elements to the TLV structure
    CHIP_ERROR err = writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    writer.Put(chip::TLV::ContextTag(1), ByteSpan(cdData));
    writer.Put(chip::TLV::ContextTag(2), ByteSpan(kTestNonceData));
    writer.Put(chip::TLV::ContextTag(3), static_cast<uint32_t>(0));
    writer.Put(chip::TLV::ContextTag(4), ByteSpan());
    writer.Put(chip::TLV::ContextTag(5), ByteSpan());
    err = writer.EndContainer(outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    size_t tlvLen = writer.GetLengthWritten();

    // Concatenate TLV structure and challenge data for signing
    uint8_t toSign[sizeof(tlvBuf) + sizeof(kTestChallengeData)];
    memcpy(toSign, tlvBuf, tlvLen);
    memcpy(toSign + tlvLen, kTestChallengeData, sizeof(kTestChallengeData));

    // Load a certificate that has a validity period starting in the future
    chip::Crypto::P256Keypair keypair;
    err = LoadKeypairFromRaw(ByteSpan(TestCerts::sTestCert_DAC_FFF2_8004_0021_ValInFuture_PrivateKey),
                             ByteSpan(TestCerts::sTestCert_DAC_FFF2_8004_0021_ValInFuture_PublicKey), keypair);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Sign the concatenated data
    chip::Crypto::P256ECDSASignature signature;
    err = keypair.ECDSA_sign_msg(toSign, tlvLen + sizeof(kTestChallengeData), signature);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Provide the future-valid certificate to the verifier
    DeviceAttestationVerifier::AttestationInfo futureValDACInfo(
        ByteSpan(tlvBuf, tlvLen), ByteSpan(kTestChallengeData), ByteSpan(signature.ConstBytes(), signature.Length()),
        TestCerts::sTestCert_PAI_FFF2_8004_FB_Cert, TestCerts::sTestCert_DAC_FFF2_8004_0021_ValInFuture_Cert,
        ByteSpan(kTestNonceData), static_cast<VendorId>(0xFFF2), 8004);

    verifier.VerifyAttestationInformation(futureValDACInfo, &attestationCallback);
    // Verify that the future-valid certificate is rejected as expired (not yet valid)
#if !defined(CURRENT_TIME_NOT_IMPLEMENTED)
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacExpired);
#endif
}

// Test case verifies that wrong formats for attestation elements are properly rejected
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithMalformedAttestationElements)
{
    // Create a malformed attestationElements buffer
    uint8_t malformedAttestationElements[32] = { 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55,
                                                 0x44, 0x33, 0x22, 0x11, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60,
                                                 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0x0F };

    // Concatenate attestationElements + attestationChallenge for signing
    uint8_t toSign[sizeof(malformedAttestationElements) + sizeof(kTestChallengeData)];
    memcpy(toSign, malformedAttestationElements, sizeof(malformedAttestationElements));
    memcpy(toSign + sizeof(malformedAttestationElements), kTestChallengeData, sizeof(kTestChallengeData));

    // Load a valid keypair for signing
    chip::Crypto::P256Keypair keypair;
    CHIP_ERROR err = LoadKeypairFromRaw(ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PrivateKey),
                                        ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PublicKey), keypair);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Sign the concatenated data
    chip::Crypto::P256ECDSASignature signature;
    err = keypair.ECDSA_sign_msg(toSign, sizeof(toSign), signature);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Prepare AttestationInfo with valid certificates and keys, but malformed attestationElements
    DeviceAttestationVerifier::AttestationInfo malformedInfo(
        ByteSpan(malformedAttestationElements), ByteSpan(kTestChallengeData), ByteSpan(signature.ConstBytes(), signature.Length()),
        TestCerts::sTestCert_PAI_FFF1_8000_Cert, TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert, ByteSpan(kTestNonceData),
        kTestVendorId, kTestProductId);

    verifier.VerifyAttestationInformation(malformedInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kAttestationElementsMalformed);
}

// Test case verifies that nonce in attestation elements does not match the expected nonce
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithMismatchedNonce)
{
    // The following attestation information uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    uint8_t cdData[32]          = { 0x11, 0x22, 0x33, 0x44 };
    uint8_t nonceInElements[32] = { 0x55, 0x66, 0x77, 0x88 };
    uint8_t expectedNonce[32]   = { 0x99, 0x88, 0x77 };

    uint8_t tlvBuf[128];

    // Construct TLV structure representing attestation elements that would be signed by device
    chip::TLV::TLVWriter writer;
    writer.Init(tlvBuf, sizeof(tlvBuf));
    chip::TLV::TLVType outerType;

    // Add attestation elements to the TLV structure
    CHIP_ERROR err = writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    writer.Put(chip::TLV::ContextTag(1), ByteSpan(cdData));
    writer.Put(chip::TLV::ContextTag(2), ByteSpan(nonceInElements));
    writer.Put(chip::TLV::ContextTag(3), static_cast<uint32_t>(0));
    writer.Put(chip::TLV::ContextTag(4), ByteSpan());
    writer.Put(chip::TLV::ContextTag(5), ByteSpan());
    err = writer.EndContainer(outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    size_t tlvLen = writer.GetLengthWritten();

    // Concatenate TLV and challenge data
    uint8_t toSign[sizeof(tlvBuf) + sizeof(kTestChallengeData)];
    memcpy(toSign, tlvBuf, tlvLen);
    memcpy(toSign + tlvLen, kTestChallengeData, sizeof(kTestChallengeData));

    // Load the keypair for signing
    chip::Crypto::P256Keypair keypair;
    err = LoadKeypairFromRaw(ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PrivateKey),
                             ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PublicKey), keypair);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Sign the concatenated data
    chip::Crypto::P256ECDSASignature signature;
    err = keypair.ECDSA_sign_msg(toSign, tlvLen + sizeof(kTestChallengeData), signature);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Prepare AttestationInfo with valid certificates and keys, but mismatched nonce
    DeviceAttestationVerifier::AttestationInfo mismatchedNonceInfo(
        ByteSpan(tlvBuf, tlvLen), ByteSpan(kTestChallengeData), ByteSpan(signature.ConstBytes(), signature.Length()),
        TestCerts::sTestCert_PAI_FFF1_8000_Cert, TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert, ByteSpan(expectedNonce),
        kTestVendorId, kTestProductId);

    verifier.VerifyAttestationInformation(mismatchedNonceInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kAttestationNonceMismatch);
}

// Test case verifies that the verifier handles cases where PAA certificate lookup fails
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithInvalidPAAFormat)
{
    // The actual contents do not represent real or meaningful data.
    uint8_t cdData[32] = { 0x11, 0x22, 0x33, 0x44 };
    uint8_t tlvBuf[128];

    // Construct TLV structure representing attestation elements that would be signed by device
    chip::TLV::TLVWriter writer;
    writer.Init(tlvBuf, sizeof(tlvBuf));
    chip::TLV::TLVType outerType;

    // Add attestation elements to the TLV structure
    CHIP_ERROR err = writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    writer.Put(chip::TLV::ContextTag(1), ByteSpan(cdData));
    writer.Put(chip::TLV::ContextTag(2), ByteSpan(kTestNonceData));
    writer.Put(chip::TLV::ContextTag(3), static_cast<uint32_t>(0));
    writer.Put(chip::TLV::ContextTag(4), ByteSpan());
    writer.Put(chip::TLV::ContextTag(5), ByteSpan());
    err = writer.EndContainer(outerType);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    size_t tlvLen = writer.GetLengthWritten();

    // Concatenate TLV and challenge data
    uint8_t toSign[sizeof(tlvBuf) + sizeof(kTestChallengeData)];
    memcpy(toSign, tlvBuf, tlvLen);
    memcpy(toSign + tlvLen, kTestChallengeData, sizeof(kTestChallengeData));

    // Load the keypair for signing
    chip::Crypto::P256Keypair keypair;
    err = LoadKeypairFromRaw(ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PrivateKey),
                             ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PublicKey), keypair);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Sign the concatenated data
    chip::Crypto::P256ECDSASignature signature;
    err = keypair.ECDSA_sign_msg(toSign, tlvLen + sizeof(kTestChallengeData), signature);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Use valid certificates but the verifier configuration will cause PAA format validation to fail
    DeviceAttestationVerifier::AttestationInfo validInfo(
        ByteSpan(tlvBuf, tlvLen), ByteSpan(kTestChallengeData), ByteSpan(signature.ConstBytes(), signature.Length()),
        TestCerts::sTestCert_PAI_FFF1_8000_Cert, TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert, ByteSpan(kTestNonceData),
        kTestVendorId, kTestProductId);

    verifier.VerifyAttestationInformation(validInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kPaaFormatInvalid);
}
