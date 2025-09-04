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
CHIP_ERROR HazardousOperationLoadKeypairFromRaw(ByteSpan private_key, ByteSpan public_key, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serialized_keypair;
    ReturnErrorOnFailure(serialized_keypair.SetLength(private_key.size() + public_key.size()));
    memcpy(serialized_keypair.Bytes(), public_key.data(), public_key.size());
    memcpy(serialized_keypair.Bytes() + public_key.size(), private_key.data(), private_key.size());
    return keypair.Deserialize(serialized_keypair);
}

static CHIP_ERROR CreateSignedAttestationData(uint8_t * tlvBuffer, size_t & tlvLen, chip::Crypto::P256ECDSASignature & signature,
                                              const ByteSpan & cdData, const ByteSpan & nonceData, const ByteSpan & privateKey,
                                              const ByteSpan & publicKey, const uint8_t * challengeData, size_t challengeDataLen);

struct TestDacOnlyPartialAttestationVerifier : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    static constexpr VendorId kTestVendorId  = static_cast<VendorId>(0xFFF1);
    static constexpr uint16_t kTestProductId = 0x8000;

    // The following test data uses arbitrary values solely for test structure;
    // the actual contents do not represent real or meaningful data.
    static constexpr uint8_t kTestAttestationElements[32] = { 0x01, 0x02, 0x03 };
    static constexpr uint8_t kTestChallengeData[32]       = { 0x04, 0x05, 0x06 };
    static constexpr uint8_t kTestSignatureData[64]       = { 0x07, 0x08, 0x09 };
    static constexpr uint8_t kTestNonceData[32]           = { 0x0A, 0x0B, 0x0C };
    static constexpr uint8_t kTestCDData[32]              = { 0x11, 0x22, 0x33 };

    void SetUp() override { attestationResult = AttestationVerificationResult::kSuccess; }

    // Create attestation verification result variable
    // This will be used to capture the result of the verification callback
    PartialDACVerifier verifier;
    AttestationVerificationResult attestationResult = AttestationVerificationResult::kSuccess;
    Callback::Callback<DeviceAttestationVerifier::OnAttestationInformationVerification> attestationCallback{
        OnAttestationInformationVerificationCallback, &attestationResult
    };

    // Helper method to run a DAC validity test with specified keys and certificate.
    void RunDACValidityTest(const ByteSpan & privateKey, const ByteSpan & publicKey, const ByteSpan & dacCert,
                            const ByteSpan & cdData, const ByteSpan & nonceData, const uint8_t * challengeData,
                            size_t challengeDataLen)
    {
        uint8_t tlvBuf[128];
        size_t tlvLen;
        chip::Crypto::P256ECDSASignature signature;

        // Create signed attestation data
        CHIP_ERROR err = CreateSignedAttestationData(tlvBuf, tlvLen, signature, cdData, nonceData, privateKey, publicKey,
                                                     challengeData, challengeDataLen);
        ASSERT_EQ(err, CHIP_NO_ERROR);

        // Set up attestation info with the generated TLV and signature
        DeviceAttestationVerifier::AttestationInfo info(ByteSpan(tlvBuf, tlvLen), ByteSpan(challengeData, challengeDataLen),
                                                        ByteSpan(signature.ConstBytes(), signature.Length()),
                                                        TestCerts::sTestCert_PAI_FFF2_8004_FB_Cert, dacCert, nonceData,
                                                        static_cast<VendorId>(0xFFF2), 8004);

        // Call the verifier and check for expired DAC result
        verifier.VerifyAttestationInformation(info, &attestationCallback);
        EXPECT_EQ(attestationResult, AttestationVerificationResult::kDacExpired);
    }
};

// Helper method to construct TLV attestation elements structure
static CHIP_ERROR ConstructAttestationElementsTLV(uint8_t * tlvBuffer, size_t bufferSize, const ByteSpan & cdData,
                                                  const ByteSpan & nonceData, size_t & tlvLen)
{
    chip::TLV::TLVWriter writer;
    writer.Init(tlvBuffer, bufferSize);
    chip::TLV::TLVType outerType;

    ReturnErrorOnFailure(writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerType));
    writer.Put(chip::TLV::ContextTag(1), cdData);
    writer.Put(chip::TLV::ContextTag(2), nonceData);
    writer.Put(chip::TLV::ContextTag(3), static_cast<uint32_t>(0));
    writer.Put(chip::TLV::ContextTag(4), ByteSpan());
    writer.Put(chip::TLV::ContextTag(5), ByteSpan());
    ReturnErrorOnFailure(writer.EndContainer(outerType));

    tlvLen = writer.GetLengthWritten();
    return CHIP_NO_ERROR;
}

// Helper method to create signed attestation data using standard test parameters.
static CHIP_ERROR CreateSignedAttestationData(uint8_t * tlvBuffer, size_t & tlvLen, chip::Crypto::P256ECDSASignature & signature,
                                              const ByteSpan & cdData, const ByteSpan & nonceData, const ByteSpan & privateKey,
                                              const ByteSpan & publicKey, const uint8_t * challengeData, size_t challengeDataLen)
{
    // Construct TLV attestation elements
    ReturnErrorOnFailure(ConstructAttestationElementsTLV(tlvBuffer, sizeof(uint8_t) * 128, cdData, nonceData, tlvLen));

    // Concatenate TLV and challenge data for signing
    constexpr size_t kMaxToSignSize = 128 + 64; // 128 for TLV, 64 for challengeData (adjust if needed)
    if (tlvLen + challengeDataLen > kMaxToSignSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    uint8_t toSign[kMaxToSignSize];
    memcpy(toSign, tlvBuffer, tlvLen);
    memcpy(toSign + tlvLen, challengeData, challengeDataLen);

    // Load keypair and sign the data
    chip::Crypto::P256Keypair keypair;
    ReturnErrorOnFailure(HazardousOperationLoadKeypairFromRaw(privateKey, publicKey, keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(toSign, tlvLen + challengeDataLen, signature));

    return CHIP_NO_ERROR;
}

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
// This check is only valid if the platform supports current time retrieval
// This is necessary to determine if the certificate is not yet valid
#if defined(CURRENT_TIME_NOT_IMPLEMENTED) || !(CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME)
    GTEST_SKIP() << "Skipping test: platform does not support current time.";
#else
    RunDACValidityTest(TestCerts::sTestCert_DAC_FFF2_8004_0020_ValInPast_PrivateKey,
                       TestCerts::sTestCert_DAC_FFF2_8004_0020_ValInPast_PublicKey,
                       TestCerts::sTestCert_DAC_FFF2_8004_0020_ValInPast_Cert, ByteSpan(kTestCDData), ByteSpan(kTestNonceData),
                       kTestChallengeData, sizeof(kTestChallengeData));
#endif
}

// Test case verifies that certificates with future validity periods are properly rejected
TEST_F(TestDacOnlyPartialAttestationVerifier, TestWithValidInFutureDACCertificate)
{
// This check is only valid if the platform supports current time retrieval
// This is necessary to determine if the certificate is not yet valid
#if defined(CURRENT_TIME_NOT_IMPLEMENTED) || !(CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME)
    GTEST_SKIP() << "Skipping test: platform does not support current time.";
#else
    RunDACValidityTest(TestCerts::sTestCert_DAC_FFF2_8004_0021_ValInFuture_PrivateKey,
                       TestCerts::sTestCert_DAC_FFF2_8004_0021_ValInFuture_PublicKey,
                       TestCerts::sTestCert_DAC_FFF2_8004_0021_ValInFuture_Cert, ByteSpan(kTestCDData), ByteSpan(kTestNonceData),
                       kTestChallengeData, sizeof(kTestChallengeData));
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
    CHIP_ERROR err = HazardousOperationLoadKeypairFromRaw(ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PrivateKey),
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
    uint8_t nonceInElements[32] = { 0x55, 0x66, 0x77, 0x88 };
    uint8_t expectedNonce[32]   = { 0x99, 0x88, 0x77 };

    uint8_t tlvBuf[128];
    size_t tlvLen;
    chip::Crypto::P256ECDSASignature signature;

    // Create signed attestation data with mismatched nonce in elements
    CHIP_ERROR err = CreateSignedAttestationData(tlvBuf, tlvLen, signature, ByteSpan(kTestCDData), ByteSpan(nonceInElements),
                                                 ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PrivateKey),
                                                 ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PublicKey), kTestChallengeData,
                                                 sizeof(kTestChallengeData));
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
    uint8_t tlvBuf[128];
    size_t tlvLen;
    chip::Crypto::P256ECDSASignature signature;

    CHIP_ERROR err = CreateSignedAttestationData(tlvBuf, tlvLen, signature, ByteSpan(kTestCDData), ByteSpan(kTestNonceData),
                                                 ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PrivateKey),
                                                 ByteSpan(TestCerts::sTestCert_DAC_FFF1_8000_0004_PublicKey), kTestChallengeData,
                                                 sizeof(kTestChallengeData));
    ASSERT_EQ(err, CHIP_NO_ERROR);

    DeviceAttestationVerifier::AttestationInfo validInfo(
        ByteSpan(tlvBuf, tlvLen), ByteSpan(kTestChallengeData), ByteSpan(signature.ConstBytes(), signature.Length()),
        TestCerts::sTestCert_PAI_FFF1_8000_Cert, TestCerts::sTestCert_DAC_FFF1_8000_0004_Cert, ByteSpan(kTestNonceData),
        kTestVendorId, kTestProductId);

    verifier.VerifyAttestationInformation(validInfo, &attestationCallback);
    EXPECT_EQ(attestationResult, AttestationVerificationResult::kPaaFormatInvalid);
}
